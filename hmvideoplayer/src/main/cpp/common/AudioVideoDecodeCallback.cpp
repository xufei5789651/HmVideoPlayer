#include "AudioVideoDecodeCallback.h"
#include "AVCodecSampleLog.h"
#include "CallBackContext.h"
#include "CodecUserData.h"
#include "SampleInfo.h"
#include "utils/LogUtils.h"

void AudioVideoDecodeCallback::OnCodecError(OH_AVCodec *codec, int32_t errorCode, void *userData) {
    AVCODEC_SAMPLE_LOGE("On codec error, error code: %{public}d", errorCode);
    if (userData == nullptr) {
        return;
    }
    (void)codec;
    CodecUserData *param = reinterpret_cast<CodecUserData *>(userData);
    if (!param) {
        LOGE("AudioVideoDecodeCallback OnCodecError CodecUserData is nullptr");
        return;
    }

    CallBackContext *callback = reinterpret_cast<CallBackContext *>(param->sampleInfo->avcodecErrorCallbackData);
    if (!callback) {
        LOGE("AudioVideoDecodeCallback OnCodecError CallBackContext is nullptr");
        return;
    }

    callback->errorAvcodecCode = errorCode;

    napi_status result = napi_acquire_threadsafe_function(param->sampleInfo->avcodecErrorFn);
    if (result != napi_ok) {
        LOGE("AudioVideoDecodeCallback OnCodecError start napi_acquire_threadsafe_function failed");
        return;
    }

    result = napi_call_threadsafe_function(param->sampleInfo->avcodecErrorFn, callback, napi_tsfn_nonblocking);
    if (result != napi_ok) {
        LOGE("AudioVideoDecodeCallback OnCodecError start napi_acquire_threadsafe_function failed");
        return;
    }
}

void AudioVideoDecodeCallback::OnCodecFormatChange(OH_AVCodec *codec, OH_AVFormat *format, void *userData) {
    AVCODEC_SAMPLE_LOGI("On codec format change");
    if (userData == nullptr) {
        return;
    }
    (void)codec;

    CodecUserData *param = reinterpret_cast<CodecUserData *>(userData);
    if (!param) {
        LOGE("AudioVideoDecodeCallback OnCodecFormatChange CodecUserData is nullptr");
        return;
    }

    CallBackContext *callback = reinterpret_cast<CallBackContext *>(param->sampleInfo->avcodecStreamCallbackData);
    if (!callback) {
        LOGE("AudioVideoDecodeCallback OnCodecError CallBackContext is nullptr");
        return;
    }

    OH_AVFormat_GetIntValue(format, OH_MD_KEY_WIDTH, &callback->videoWidth);
    OH_AVFormat_GetIntValue(format, OH_MD_KEY_HEIGHT, &callback->videoHeight);
    OH_AVFormat_GetIntValue(format, OH_MD_KEY_AUDIO_SAMPLE_FORMAT, &callback->audioSampleFormat);
    OH_AVFormat_GetIntValue(format, OH_MD_KEY_AUD_CHANNEL_COUNT, &callback->audioChannelCount);
    OH_AVFormat_GetIntValue(format, OH_MD_KEY_AUD_SAMPLE_RATE, &callback->audioSampleRate);
    OH_AVFormat_GetDoubleValue(format, OH_MD_KEY_FRAME_RATE, &callback->videoFrameRate);
    
    napi_status result = napi_acquire_threadsafe_function(param->sampleInfo->avcodecStreamChangeFn);
    if (result != napi_ok) {
        LOGE("AudioVideoDecodeCallback OnCodecError start napi_acquire_threadsafe_function failed");
        return;
    }

    result = napi_call_threadsafe_function(param->sampleInfo->avcodecStreamChangeFn, callback, napi_tsfn_nonblocking);
    if (result != napi_ok) {
        LOGE("AudioVideoDecodeCallback OnCodecError start napi_acquire_threadsafe_function failed");
        return;
    }
}

void AudioVideoDecodeCallback::OnNeedInputBuffer(OH_AVCodec *codec, uint32_t index, OH_AVBuffer *buffer,
                                                 void *userData) {
    if (userData == nullptr) {
        return;
    }
    (void)codec;
    CodecUserData *codecUserData = static_cast<CodecUserData *>(userData);
    std::unique_lock<std::mutex> lock(codecUserData->inputMutex);
    codecUserData->inputBufferInfoQueue.emplace(index, buffer);
    codecUserData->inputCond.notify_all();
}

void AudioVideoDecodeCallback::OnNewOutputBuffer(OH_AVCodec *codec, uint32_t index, OH_AVBuffer *buffer,
                                                 void *userData) {
    if (userData == nullptr) {
        return;
    }
    (void)codec;
    CodecUserData *codecUserData = static_cast<CodecUserData *>(userData);
    std::unique_lock<std::mutex> lock(codecUserData->outputMutex);
    codecUserData->outputBufferInfoQueue.emplace(index, buffer);
    codecUserData->outputCond.notify_all();
}
