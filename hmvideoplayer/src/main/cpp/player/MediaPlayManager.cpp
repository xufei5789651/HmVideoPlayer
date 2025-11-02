#include "MediaPlayManager.h"
#include "AVCodecSampleLog.h"
#include "CallBackContext.h"
#include "CodecUserData.h"
#include "StateCode.h"
#include "dfx/error/AVCodecSampleError.h"
#include "utils/ResultCode.h"
#include <cstdint>
#include <queue>
#include "OHAudioRenderCallback.h"

#undef LOG_TAG
#define LOG_TAG "player"

namespace {
constexpr int BALANCE_VALUE = 5;
constexpr int64_t WAIT_TIME_US_THRESHOLD_WARNING = -1 * 40 * 1000; // warning threshold 40ms
constexpr int64_t WAIT_TIME_US_THRESHOLD = 1 * 1000 * 1000;        // max sleep time 1s
constexpr int64_t PER_SINK_TIME_THRESHOLD = 33 * 1000;             // max per sink time 33ms
constexpr int64_t SINK_TIME_US_THRESHOLD = 100000;                 // max sink time 100ms
constexpr int32_t BYTES_PER_SAMPLE_2 = 2;                          // 2 bytes per sample
using namespace std::chrono_literals;
} // namespace

MediaPlayManager::~MediaPlayManager() {
    MediaPlayManager::Release(); 
}

int32_t MediaPlayManager::CreateAudioDecoder() {
    LOGW("audio mime:%{public}s", sampleInfo.audioCodecMime.c_str());
    int32_t ret = audioDecoder->Create(sampleInfo.audioCodecMime);
    if (ret != AVCODEC_SAMPLE_ERR_OK) {
        LOGE("Create audio decoder failed, mime:%{public}s", sampleInfo.audioCodecMime.c_str());
    } else {
        audioDecContext = new CodecUserData;
        ret = audioDecoder->Config(sampleInfo, audioDecContext);
        if (ret != RESULT_CODE_OK) {
            LOGE("Audio Decoder config failed");
            return RESULT_CODE_ERROR;
        }
        OH_AudioStreamBuilder_Create(&builder, AUDIOSTREAM_TYPE_RENDERER);
        // 设置低延时模式
        OH_AudioStreamBuilder_SetLatencyMode(builder, AUDIOSTREAM_LATENCY_MODE_NORMAL);
        // 设置音频采样率
        OH_AudioStreamBuilder_SetSamplingRate(builder, sampleInfo.audioSampleRate);
        // 设置音频声道
        OH_AudioStreamBuilder_SetChannelCount(builder, sampleInfo.audioChannelCount);
        // 设置音频采样格式
        OH_AudioStreamBuilder_SetSampleFormat(builder, AUDIOSTREAM_SAMPLE_S16LE);
        // 设置音频流的编码类型
        OH_AudioStreamBuilder_SetEncodingType(builder, AUDIOSTREAM_ENCODING_TYPE_RAW);
        // 设置音频流的工作场景
        OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_MUSIC);
        LOGW("Init audioSampleRate: %{public}d, ChannelCount: %{public}d", sampleInfo.audioSampleRate,sampleInfo.audioChannelCount);
        OH_AudioRenderer_Callbacks callbacks;
         // 设置音频回调函数
        callbacks.OH_AudioRenderer_OnWriteData = nullptr;
        callbacks.OH_AudioRenderer_OnStreamEvent = nullptr;
        callbacks.OH_AudioRenderer_OnInterruptEvent = OHAudioRenderCallback::OnRenderInterruptEvent;
        callbacks.OH_AudioRenderer_OnError = OHAudioRenderCallback::OnRenderError;
        OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, audioDecContext);
        
        OH_AudioRenderer_OnWriteDataCallback writeDataCb=OHAudioRenderCallback::OnWriteDataCallback;
        OH_AudioStreamBuilder_SetRendererWriteDataCallback(builder, writeDataCb, audioDecContext);
        
        OH_AudioRenderer_OutputDeviceChangeCallback outputDeviceChangeCb=OHAudioRenderCallback::OnOutputDeviceChangeCallback;
        OH_AudioStreamBuilder_SetRendererOutputDeviceChangeCallback(builder, outputDeviceChangeCb, audioDecContext);
        
        // api 20 起始
//        OH_AudioRenderer_OnInterruptCallback interruptCb=OHAudioRenderCallback::OnInterruptCallback;
//        OH_AudioStreamBuilder_SetRendererInterruptCallback(builder, interruptCb, audioDecContext);
        
        // api 20 起始
//        OH_AudioRenderer_OnErrorCallback errorCb=OHAudioRenderCallback::OnErrorCallback;
//        OH_AudioStreamBuilder_SetRendererErrorCallback(builder, errorCb, audioDecContext);
        
        // 构造播放音频流
        OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    }
    return AVCODEC_SAMPLE_ERR_OK;
}

int32_t MediaPlayManager::CreateVideoDecoder() {
    LOGW("video mime:%{public}s", sampleInfo.videoCodecMime.c_str());
    int32_t ret = videoDecoder->Create(sampleInfo.videoCodecMime);
    if (ret != AVCODEC_SAMPLE_ERR_OK) {
        LOGW("Create video decoder failed, mime:%{public}s", sampleInfo.videoCodecMime.c_str());
    } else {
        videoDecContext = new CodecUserData;
        sampleInfo.window = NativeXComponentSample::PluginManager::GetInstance()->pluginWindow_;
        ret = videoDecoder->Config(sampleInfo, videoDecContext);
        if (ret != RESULT_CODE_OK) {
            LOGE("Video Decoder config failed");
            return RESULT_CODE_ERROR;
        }
    }
    return AVCODEC_SAMPLE_ERR_OK;
}

int32_t MediaPlayManager::Init(SampleInfo &info) {
    // 互斥量:mutex
    std::lock_guard<std::mutex> lock(mutex);
     if (isStarted) {
        LOGE("MediaPlayManager Already started.");
        return RESULT_CODE_ERROR;
    }

    if (demuxer != nullptr && videoDecoder != nullptr && audioDecoder != nullptr) {
        LOGE("MediaPlayManager Already started.");
        return RESULT_CODE_ERROR;
    }
    this->sampleInfo = info;
    // 独占智能指针(自动管理内存)
    videoDecoder = std::make_unique<VideoDecoder>();
    audioDecoder = std::make_unique<AudioDecoder>();
    demuxer = std::make_unique<Demuxer>();
    // 解封器初始化:解析音视频配置信息
    int32_t ret = demuxer->Create(sampleInfo);
     if (ret != RESULT_CODE_OK) {
        LOGE("MediaPlayManager Create demuxer failed");
        return RESULT_CODE_ERROR;
    }
    // 初始化音频解码器：为解码作准备
    ret = CreateAudioDecoder();
     if (ret != RESULT_CODE_OK) {
        LOGE("MediaPlayManager Create audio decoder failed");
        return RESULT_CODE_ERROR;
    }
    // 初始化视频解码器：为解码作准备
    ret = CreateVideoDecoder();
    if (ret != RESULT_CODE_OK) {
        LOGE("MediaPlayManager Create video decoder failed");
        return RESULT_CODE_ERROR;
    }
    
    if (this->sampleInfo.stateCallbackData != nullptr && !isReleased) {
        LOGD("MediaPlayManager Init() PREPARED_STATE");
        CallBackContext *stateCallBackContext = reinterpret_cast<CallBackContext *>(this->sampleInfo.stateCallbackData);
        stateCallBackContext->state = PREPARED_STATE;
        
        napi_status result = napi_acquire_threadsafe_function(this->sampleInfo.stateChangeFn);
        if (result != napi_ok) {
            LOGE("MediaPlayManager Init napi_acquire_threadsafe_function failed");
            return RESULT_CODE_ERROR;
        }
        
        result = napi_call_threadsafe_function(this->sampleInfo.stateChangeFn, stateCallBackContext, napi_tsfn_blocking);
        if (result != napi_ok) {
            LOGE("MediaPlayManager Init napi_call_threadsafe_function failed");
            return RESULT_CODE_ERROR;
        }
    }
    
    if (audioDecContext != nullptr) {
        audioDecContext->sampleInfo = &sampleInfo;
    }

    isReleased = false;
    LOGI("MediaPlayManager Init Succeed");
    return RESULT_CODE_OK;
}

int32_t MediaPlayManager::Start() {
    // 互斥量:mutex
    std::lock_guard<std::mutex> lock(mutex);
    int32_t ret;
    
    if (isStarted) {
        LOGE("MediaPlayManager Start() Already started.");
        return RESULT_CODE_ERROR;
    }
    if (demuxer == nullptr) {
        LOGE("MediaPlayManager Start() demuxer nullptr.");
        return RESULT_CODE_ERROR;
    }
    // 视频解码
    if (videoDecContext) {
        ret = videoDecoder->Start();// 视频解码开始
        if (ret != RESULT_CODE_OK) {
            LOGE("MediaPlayManager Start() Video Decoder start failed");
            return RESULT_CODE_ERROR;
        }
        isStarted = true;
        videoDecInputThread = std::make_unique<std::thread>(&MediaPlayManager::VideoDecInputThread, this);
        videoDecOutputThread = std::make_unique<std::thread>(&MediaPlayManager::VideoDecOutputThread, this);

        if (videoDecInputThread == nullptr || videoDecOutputThread == nullptr) {
            LOGE("MediaPlayManager Start() Create thread failed");
            Release();
            return RESULT_CODE_ERROR;
        }
    }
    
    // 音频解码
    if (audioDecContext) {
        ret = audioDecoder->Start(); // 音频解码开始
        if (ret != RESULT_CODE_OK) {
            LOGE("MediaPlayManager Start() Audio Decoder start failed");
            return RESULT_CODE_ERROR;
        }
        isStarted = true;
        audioDecInputThread = std::make_unique<std::thread>(&MediaPlayManager::AudioDecInputThread, this);
        audioDecOutputThread = std::make_unique<std::thread>(&MediaPlayManager::AudioDecOutputThread, this);
        if (audioDecInputThread == nullptr || audioDecOutputThread == nullptr) {
            LOGE("MediaPlayManager Start() Create thread failed");
            Release();
            return RESULT_CODE_ERROR;
        }
        // 清空队列
        while (audioDecContext && !audioDecContext->renderQueue.empty()) {
            audioDecContext->renderQueue.pop();
        }
        if (audioRenderer) {
            // 【OHAudio 重要步骤】开始输出音频数据
            OH_AudioRenderer_Start(audioRenderer);
        }
    }
    
    if (this->sampleInfo.stateCallbackData != nullptr && !isReleased) {
        LOGD("MediaPlayManager Start() PLAYING_STATE");
        CallBackContext *stateCallBackContext = reinterpret_cast<CallBackContext *>(this->sampleInfo.stateCallbackData);
        stateCallBackContext->state = PLAYING_STATE;

        napi_status result = napi_acquire_threadsafe_function(this->sampleInfo.stateChangeFn);
        if (result != napi_ok) {
            LOGE("MediaPlayManager start napi_acquire_threadsafe_function failed");
            return RESULT_CODE_ERROR;
        }

        result = napi_call_threadsafe_function(this->sampleInfo.stateChangeFn, stateCallBackContext, napi_tsfn_nonblocking);
        if (result != napi_ok) {
            LOGE("MediaPlayManager start napi_call_threadsafe_function failed");
            return RESULT_CODE_ERROR;
        }
    }
    
    LOGI("MediaPlayManager Start() Succeed");
    return RESULT_CODE_OK;
}

int32_t MediaPlayManager::Pause() {
    isPause.store(true);
    if (audioRenderer) {
        // 【OHAudio 重要步骤】暂停输出音频数据
        OH_AudioRenderer_Pause(audioRenderer);
    }

    if (this->sampleInfo.stateCallbackData != nullptr && !isReleased) {
        CallBackContext *stateCallBackContext = reinterpret_cast<CallBackContext *>(this->sampleInfo.stateCallbackData);
        stateCallBackContext->state = PAUSED_STATE;

        napi_status result = napi_acquire_threadsafe_function(this->sampleInfo.stateChangeFn);
        if (result != napi_ok) {
            LOGE("MediaPlayManager pause napi_acquire_threadsafe_function failed");
            return RESULT_CODE_ERROR;
        }

        result = napi_call_threadsafe_function(this->sampleInfo.stateChangeFn, stateCallBackContext, napi_tsfn_nonblocking);
        if (result != napi_ok) {
            LOGE("MediaPlayManager pause napi_call_threadsafe_function failed");
            return RESULT_CODE_ERROR;
        }
    }
    return RESULT_CODE_OK;
}

int32_t MediaPlayManager::Resume() {
    isPause.store(false);
    videoDecContext->inputCond.notify_all();
    videoDecContext->outputCond.notify_all();

    audioDecContext->inputCond.notify_all();
    audioDecContext->outputCond.notify_all();

    if (audioRenderer) { 
        // 【OHAudio 重要步骤】开始输出音频数据
        OH_AudioRenderer_Start(audioRenderer);
    }

    if (this->sampleInfo.stateCallbackData != nullptr && !isReleased) {
        CallBackContext *stateCallBackContext = reinterpret_cast<CallBackContext *>(this->sampleInfo.stateCallbackData);
        stateCallBackContext->state = PLAYING_STATE;

        napi_status result = napi_acquire_threadsafe_function(this->sampleInfo.stateChangeFn);
        if (result != napi_ok) {
            LOGE("start napi_acquire_threadsafe_function failed");
            return RESULT_CODE_ERROR;
        }

        result = napi_call_threadsafe_function(this->sampleInfo.stateChangeFn, stateCallBackContext, napi_tsfn_nonblocking);
        if (result != napi_ok) {
            LOGE("start napi_call_threadsafe_function failed");
            return RESULT_CODE_ERROR;
        }
    }
    return AVCODEC_SAMPLE_ERR_OK;
}

int32_t MediaPlayManager::Stop() {
    isPause.store(true);

    if (audioRenderer) {
        OH_AudioRenderer_Stop(audioRenderer);
    }

    if (audioDecoder) {
        audioDecoder->Stop();
    }

    if (videoDecoder) {
        videoDecoder->Stop();
    }

    if (this->sampleInfo.stateCallbackData != nullptr && !isReleased) {
        CallBackContext *stateCallBackContext = reinterpret_cast<CallBackContext *>(this->sampleInfo.stateCallbackData);
        stateCallBackContext->state = STOPPED_STATE;

        napi_status result = napi_acquire_threadsafe_function(this->sampleInfo.stateChangeFn);
        if (result != napi_ok) {
            LOGE("stop napi_acquire_threadsafe_function failed");
            return RESULT_CODE_ERROR;
        }

        result = napi_call_threadsafe_function(this->sampleInfo.stateChangeFn, stateCallBackContext, napi_tsfn_nonblocking);
        if (result != napi_ok) {
            LOGE("stop napi_call_threadsafe_function failed");
            return RESULT_CODE_ERROR;
        }
    }
    return AVCODEC_SAMPLE_ERR_OK;
}

int64_t MediaPlayManager::GetDuration(){
    if (demuxer) {
        return demuxer->GetDuration();
    }
    return RESULT_CODE_ERROR;
}

int32_t MediaPlayManager::Seek(int64_t position){
    if (demuxer) {
        return demuxer->Seek(position);
    }
    return RESULT_CODE_ERROR;
}

void MediaPlayManager::setTimeStampCallback(SampleInfo &sampleInfo) {
    this->sampleInfo.callbackData = sampleInfo.callbackData;
    this->sampleInfo.timestampFn = sampleInfo.timestampFn;
}

void MediaPlayManager::setStateChangeCallback(SampleInfo &sampleInfo) {
    this->sampleInfo.stateCallbackData = sampleInfo.stateCallbackData;
    this->sampleInfo.stateChangeFn = sampleInfo.stateChangeFn;
}

void MediaPlayManager::SetSpeed(float speed) {
    if (this->speed == speed) {
        AVCODEC_SAMPLE_LOGE("Same speed value");
        return;
    }

    OH_AudioRenderer_SetSpeed(audioRenderer, speed);
    this->speed = speed;
    audioDecContext->speed = speed;
}

void MediaPlayManager::ReleaseThread() {
    if (videoDecInputThread && videoDecInputThread->joinable()) {
        videoDecInputThread->detach();
        videoDecInputThread.reset();
    }
    if (videoDecOutputThread && videoDecOutputThread->joinable()) {
        videoDecOutputThread->detach();
        videoDecOutputThread.reset();
    }
    if (audioDecInputThread && audioDecInputThread->joinable()) {
        audioDecInputThread->detach();
        audioDecInputThread.reset();
    }
    if (audioDecOutputThread && audioDecOutputThread->joinable()) {
        audioDecOutputThread->detach();
        audioDecOutputThread.reset();
    }
}

void MediaPlayManager::Release() {
    if (isReleased.load()) {
        return;
    }
    isPause.store(false);
    isReleased.store(true);

    std::lock_guard<std::mutex> lock(mutex);
    isStarted = false;

    // Clear the queue
    while (audioDecContext && !audioDecContext->renderQueue.empty()) {
        audioDecContext->renderQueue.pop();
    }
    if (audioRenderer != nullptr) {
        OH_AudioRenderer_Release(audioRenderer);
        audioRenderer = nullptr;
    }
    ReleaseThread();

    if (demuxer != nullptr) {
        demuxer->Release();
        demuxer.reset();
    }
    if (videoDecoder != nullptr) {
        videoDecoder->Release();
        videoDecoder.reset();
    }
    if (videoDecContext != nullptr) {
        delete videoDecContext;
        videoDecContext = nullptr;
    }
    if (audioDecoder != nullptr) {
        audioDecoder->Release();
        audioDecoder.reset();
    }
    if (audioDecContext != nullptr) {
        delete audioDecContext;
        audioDecContext = nullptr;
    }
    if (builder != nullptr) {
        OH_AudioStreamBuilder_Destroy(builder);
        builder = nullptr;
    }

    if (this->sampleInfo.stateCallbackData != nullptr) {
        CallBackContext *stateCallBackContext = reinterpret_cast<CallBackContext *>(this->sampleInfo.stateCallbackData);
        stateCallBackContext->state = RELEASE_STATE;
        
        napi_status result = napi_acquire_threadsafe_function(this->sampleInfo.stateChangeFn);
        if (result != napi_ok) {
            LOGE("release napi_acquire_threadsafe_function failed");
        }
        
        result = napi_call_threadsafe_function(this->sampleInfo.stateChangeFn, stateCallBackContext, napi_tsfn_nonblocking);
        if (result != napi_ok) {
            LOGE("release napi_call_threadsafe_function failed");
        }
    }
    
//     doneCond.notify_all();
    AVCODEC_SAMPLE_LOGI("Succeed");
}

void MediaPlayManager::VideoDecInputThread() {
    while (true) {
        CHECK_AND_BREAK_LOG(isStarted, "Decoder input thread out");
        std::unique_lock<std::mutex> lock(videoDecContext->inputMutex);
        
//         bool condRet = videoDecContext->inputCond.wait_for(
//             lock, 5s, [this]() { return !isStarted || !videoDecContext->inputBufferInfoQueue.empty(); });

        videoDecContext->inputCond.wait(lock, [this]() {
            return !isPause.load() && (!isStarted || !videoDecContext->inputBufferInfoQueue.empty());
        });
        CHECK_AND_BREAK_LOG(isStarted, "Work done, thread out");
//         CHECK_AND_CONTINUE_LOG(!videoDecContext->inputBufferInfoQueue.empty(),
//                                "Buffer queue is empty, continue, cond ret: %{public}d", condRet);

        CodecBufferInfo bufferInfo = videoDecContext->inputBufferInfoQueue.front();
        videoDecContext->inputBufferInfoQueue.pop();
        videoDecContext->inputFrameCount++;
        lock.unlock();

        demuxer->ReadSample(demuxer->GetVideoTrackId(), reinterpret_cast<OH_AVBuffer *>(bufferInfo.buffer),
                            bufferInfo.attr);

        int32_t ret = videoDecoder->PushInputBuffer(bufferInfo);
        CHECK_AND_BREAK_LOG(ret == AVCODEC_SAMPLE_ERR_OK, "Push data failed, thread out");

        CHECK_AND_BREAK_LOG(!(bufferInfo.attr.flags & AVCODEC_BUFFER_FLAGS_EOS), "Catch EOS, thread out");
    }
}

void MediaPlayManager::VideoDecOutputThread() {
    sampleInfo.frameInterval = MICROSECOND / sampleInfo.frameRate;
    while (true) {
        thread_local auto lastPushTime = std::chrono::system_clock::now();
        CHECK_AND_BREAK_LOG(isStarted, "VD Decoder output thread out");
        std::unique_lock<std::mutex> lock(videoDecContext->outputMutex);
        
//         bool condRet = videoDecContext->outputCond.wait_for(
//             lock, 5s, [this]() { return !isStarted || !videoDecContext->outputBufferInfoQueue.empty(); });


        videoDecContext->outputCond.wait(lock, [this]() {
            return !isPause.load() && (!isStarted || !videoDecContext->outputBufferInfoQueue.empty());
        });


        CHECK_AND_BREAK_LOG(isStarted, "VD Decoder output thread out");
//         CHECK_AND_CONTINUE_LOG(!videoDecContext->outputBufferInfoQueue.empty(),
//                                "VD Buffer queue is empty, continue, cond ret: %{public}d", condRet);

        CodecBufferInfo bufferInfo = videoDecContext->outputBufferInfoQueue.front();
        videoDecContext->outputBufferInfoQueue.pop();
        AVCODEC_SAMPLE_LOGI("VD bufferInfo.bufferIndex: %{public}d", bufferInfo.bufferIndex);
        CHECK_AND_BREAK_LOG(!(bufferInfo.attr.flags & AVCODEC_BUFFER_FLAGS_EOS), "Catch EOS, thread out");
        videoDecContext->outputFrameCount++;
        AVCODEC_SAMPLE_LOGW(
            "VD Out buffer count: %{public}u, size: %{public}d, flag: %{public}u, pts: %{public}" PRId64,
            videoDecContext->outputFrameCount, bufferInfo.attr.size, bufferInfo.attr.flags, bufferInfo.attr.pts);
        lock.unlock();
        // [Start unlock]


        // get audio render position
        int64_t framePosition = 0;
        int64_t timeStamp = 0;
        int32_t ret = OH_AudioRenderer_GetTimestamp(audioRenderer, CLOCK_MONOTONIC, &framePosition, &timeStamp);

        audioTimeStamp = timeStamp;
        // [End unlock]
        // [Start audio_render]
        // audio render getTimeStamp error, render it
        if (ret != AUDIOSTREAM_SUCCESS || (timeStamp == 0) || (framePosition == 0)) {
            // first frame, render without wait
            videoDecoder->FreeOutputBuffer(bufferInfo.bufferIndex, true);
            std::this_thread::sleep_until(lastPushTime + std::chrono::microseconds(sampleInfo.frameInterval));
            lastPushTime = std::chrono::system_clock::now();
            continue;
        }
        // [End audio_render]
        // [Start after_seek]
        // after seek, audio render flush, framePosition = 0, then writtenSampleCnt = 0
        int64_t latency =
            (audioDecContext->frameWrittenForSpeed - framePosition) * 1000 * 1000 / sampleInfo.audioSampleRate / speed;
        AVCODEC_SAMPLE_LOGI("VD latency: %{public}ld writtenSampleCnt: %{public}ld", latency, writtenSampleCnt);

        nowTimeStamp = GetCurrentTime();
        int64_t anchorDiff = (nowTimeStamp - audioTimeStamp) / 1000;
        // us, audio buffer accelerate render time
        int64_t audioPlayedTime = audioDecContext->currentPosAudioBufferPts - latency + anchorDiff;
        // us, video buffer expected render time
        int64_t videoPlayedTime = bufferInfo.attr.pts;

        // audio render timeStamp and now timeStamp diff
        int64_t waitTimeUs = videoPlayedTime - audioPlayedTime;
        // [End after_seek]
        AVCODEC_SAMPLE_LOGI("VD bufferInfo.bufferIndex: %{public}d", bufferInfo.bufferIndex);
        AVCODEC_SAMPLE_LOGI("VD audioPlayedTime: %{public}ld, videoPlayedTime: %{public}ld, nowTimeStamp: %{public}ld, "
                            "audioTimeStamp: %{public}ld, waitTimeUs: %{public}ld, anchorDiff: %{public}ld",
                            audioPlayedTime, videoPlayedTime, nowTimeStamp, audioTimeStamp, waitTimeUs, anchorDiff);

        bool dropFrame = false;
        // [Start video_buffer]
        // video buffer is too late, drop it
        if (waitTimeUs < WAIT_TIME_US_THRESHOLD_WARNING) {
            dropFrame = true;
            AVCODEC_SAMPLE_LOGE("VD buffer is too late");
        } else {
            AVCODEC_SAMPLE_LOGE("VD buffer is too early waitTimeUs: %{public}ld", waitTimeUs);
            // [0, ), render it with waitTimeUs, max 1s
            // [-40,0), render it
            if (waitTimeUs > WAIT_TIME_US_THRESHOLD) {
                waitTimeUs = WAIT_TIME_US_THRESHOLD;
            }
            // per frame render time reduced by 33ms
            if (waitTimeUs > sampleInfo.frameInterval + PER_SINK_TIME_THRESHOLD) {
                waitTimeUs = sampleInfo.frameInterval + PER_SINK_TIME_THRESHOLD;
                AVCODEC_SAMPLE_LOGE("VD buffer is too early and reduced 33ms, waitTimeUs: %{public}ld", waitTimeUs);
            }
        }
        // [End video_buffer]
        // [Start wait_time_us]
        if (waitTimeUs > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(waitTimeUs));
        }
        lastPushTime = std::chrono::system_clock::now();
        ret = videoDecoder->FreeOutputBuffer(bufferInfo.bufferIndex, !dropFrame);
        // [End wait_time_us]
        CHECK_AND_BREAK_LOG(ret == AVCODEC_SAMPLE_ERR_OK, "Decoder output thread out");
    }
    writtenSampleCnt = 0;
    audioBufferPts = 0;
    Release();
}

int64_t MediaPlayManager::GetCurrentTime() {
    int64_t result = -1; // -1 for bad result
    struct timespec time;
    clockid_t clockId = CLOCK_MONOTONIC;
    int ret = clock_gettime(clockId, &time);
    CHECK_AND_RETURN_RET_LOG(ret >= 0, result, "GetCurNanoTime fail, result: %{public}d", ret);
    result = (time.tv_sec * 1000000000) + time.tv_nsec;
    return result;
}

void MediaPlayManager::AudioDecInputThread() {
    while (true) {
        CHECK_AND_BREAK_LOG(isStarted, "Decoder input thread out");
        std::unique_lock<std::mutex> lock(audioDecContext->inputMutex);
        
//         bool condRet = audioDecContext->inputCond.wait_for(
//             lock, 5s, [this]() { return !isStarted || !audioDecContext->inputBufferInfoQueue.empty(); });
        audioDecContext->inputCond.wait(lock, [this]() {
            return !isPause.load() && (!isStarted || !audioDecContext->inputBufferInfoQueue.empty());
        });

        CHECK_AND_BREAK_LOG(isStarted, "Work done, thread out");
//         CHECK_AND_CONTINUE_LOG(!audioDecContext->inputBufferInfoQueue.empty(),
//                                "Buffer queue is empty, continue, cond ret: %{public}d", condRet);

        CodecBufferInfo bufferInfo = audioDecContext->inputBufferInfoQueue.front();
        audioDecContext->inputBufferInfoQueue.pop();
        audioDecContext->inputFrameCount++;
        lock.unlock();

        demuxer->ReadSample(demuxer->GetAudioTrackId(), reinterpret_cast<OH_AVBuffer *>(bufferInfo.buffer),
                            bufferInfo.attr);

        // 当前进度
        if (this->sampleInfo.callbackData != nullptr && !isReleased && bufferInfo.attr.pts != 0) {
            
            CallBackContext *callBackContext = reinterpret_cast<CallBackContext *>(this->sampleInfo.callbackData);
            callBackContext->timestamp = bufferInfo.attr.pts / 1000;
            
            napi_status result = napi_acquire_threadsafe_function(this->sampleInfo.timestampFn);
            if (result != napi_ok) {
                LOGE("MediaPlayManager napi_acquire_threadsafe_function failed");
                break;
            }
            
            result =napi_call_threadsafe_function(this->sampleInfo.timestampFn, callBackContext, napi_tsfn_nonblocking);
            if (result != napi_ok) {
                LOGE("MediaPlayManager napi_call_threadsafe_function failed");
                break;
            }
        }

        int32_t ret = audioDecoder->PushInputBuffer(bufferInfo);
        CHECK_AND_BREAK_LOG(ret == AVCODEC_SAMPLE_ERR_OK, "Push data failed, thread out");

//         CHECK_AND_BREAK_LOG(!(bufferInfo.attr.flags & AVCODEC_BUFFER_FLAGS_EOS), "Catch EOS, thread out");
        if ((bufferInfo.attr.flags & AVCODEC_BUFFER_FLAGS_EOS) 
            && this->sampleInfo.stateCallbackData != nullptr && !isReleased) {
            CallBackContext *stateCallBackContext =reinterpret_cast<CallBackContext *>(this->sampleInfo.stateCallbackData);
            stateCallBackContext->state = COMPLETED_STATE;
            
            napi_status result = napi_acquire_threadsafe_function(this->sampleInfo.stateChangeFn);
            if (result != napi_ok) {
                LOGE("MediaPlayManager napi_acquire_threadsafe_function failed");
                break;
            }
            
            result = napi_call_threadsafe_function(this->sampleInfo.stateChangeFn, stateCallBackContext,napi_tsfn_nonblocking);
            if (result != napi_ok) {
                LOGE("MediaPlayManager napi_call_threadsafe_function failed");
                break;
            }
            LOGE("MediaPlayManager Catch EOS, thread out");
        }
        
    }
}

void MediaPlayManager::AudioDecOutputThread() {
    while (true) {
        CHECK_AND_BREAK_LOG(isStarted, "Decoder output thread out");
        std::unique_lock<std::mutex> lock(audioDecContext->outputMutex);
        
//         bool condRet = audioDecContext->outputCond.wait_for(
//             lock, 5s, [this]() { return !isStarted || !audioDecContext->outputBufferInfoQueue.empty(); });
        audioDecContext->outputCond.wait(lock, [this]() {
            return !isPause.load() && (!isStarted || !audioDecContext->outputBufferInfoQueue.empty());
        });

        CHECK_AND_BREAK_LOG(isStarted, "Decoder output thread out");
//         CHECK_AND_CONTINUE_LOG(!audioDecContext->outputBufferInfoQueue.empty(),
//                                "Buffer queue is empty, continue, cond ret: %{public}d", condRet);

        CodecBufferInfo bufferInfo = audioDecContext->outputBufferInfoQueue.front();
        audioDecContext->outputBufferInfoQueue.pop();
        CHECK_AND_BREAK_LOG(!(bufferInfo.attr.flags & AVCODEC_BUFFER_FLAGS_EOS), "Catch EOS, thread out");
        audioDecContext->outputFrameCount++;
        AVCODEC_SAMPLE_LOGW("Out buffer count: %{public}u, size: %{public}d, flag: %{public}u, pts: %{public}" PRId64,
                            audioDecContext->outputFrameCount, bufferInfo.attr.size, bufferInfo.attr.flags,
                            bufferInfo.attr.pts);
        uint8_t *source = OH_AVBuffer_GetAddr(reinterpret_cast<OH_AVBuffer *>(bufferInfo.buffer));
        // Put the decoded PMC data into the queue
        for (int i = 0; i < bufferInfo.attr.size; i++) {
            audioDecContext->renderQueue.push(*(source + i));
        }
        lock.unlock();

        int32_t ret = audioDecoder->FreeOutputBuffer(bufferInfo.bufferIndex, true);
        CHECK_AND_BREAK_LOG(ret == AVCODEC_SAMPLE_ERR_OK, "Decoder output thread out");

        // SAMPLE_S16LE 2 bytes per frame
        // if set speed, cnt / speed
        writtenSampleCnt += (bufferInfo.attr.size / sampleInfo.audioChannelCount / BYTES_PER_SAMPLE_2);
        AVCODEC_SAMPLE_LOGI("writtenSampleCnt: %ld, bufferInfo.attr.size: %d, sampleInfo.audioChannelCount: %d",
                            writtenSampleCnt, bufferInfo.attr.size, sampleInfo.audioChannelCount);
        audioBufferPts = bufferInfo.attr.pts;
        audioDecContext->endPosAudioBufferPts = audioBufferPts;

        std::unique_lock<std::mutex> lockRender(audioDecContext->renderMutex);
        audioDecContext->renderCond.wait_for(lockRender, 20ms, [this, bufferInfo]() {
            return audioDecContext->renderQueue.size() < BALANCE_VALUE * bufferInfo.attr.size;
        });
    }
    AVCODEC_SAMPLE_LOGI("Out buffer end");
    Release();
}