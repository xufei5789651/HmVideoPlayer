#ifndef VIDEO_CODEC_PLAYER_H
#define VIDEO_CODEC_PLAYER_H

#include <bits/alltypes.h>
#include <mutex>
#include <memory>
#include <atomic>
#include <thread>
#include <unistd.h>
#include <ohaudio/native_audiorenderer.h>
#include <ohaudio/native_audiostreambuilder.h>
#include <fstream>
#include "VideoDecoder.h"
#include "AudioDecoder.h"
#include "multimedia/player_framework/native_avbuffer.h"
#include "Demuxer.h"
#include "SampleInfo.h"
#include "PluginManager.h"

class MediaPlayManager {
public:
    MediaPlayManager(){};
    ~MediaPlayManager();

    static MediaPlayManager &GetInstance() {
        static MediaPlayManager player;
        return player;
    }

    int32_t Init(SampleInfo &sampleInfo);
    int32_t Start();
    // 暂停播放
    int32_t Pause();
    // 恢复播放
    int32_t Resume();
    // 停止播放
    int32_t Stop();
    // 获取时长,单位:微妙
    int64_t GetDuration();
    // 跳转到播放位置，position:毫秒
    int32_t Seek(int64_t position);
    // 设置时间戳回调函数
    void setTimeStampCallback(SampleInfo &sampleInfo);
    
    void setStateChangeCallback(SampleInfo &sampleInfo);
    
    void Release();
    
    void SetSpeed(float speed);

private:
    void VideoDecInputThread();
    void VideoDecOutputThread();
    void AudioDecInputThread();
    void AudioDecOutputThread();
    void ReleaseThread();
    
    int32_t CreateAudioDecoder();
    int32_t CreateVideoDecoder();
    int64_t GetCurrentTime();
    std::unique_ptr<VideoDecoder> videoDecoder = nullptr;
    std::shared_ptr<AudioDecoder> audioDecoder = nullptr;
    std::unique_ptr<Demuxer> demuxer = nullptr;

    std::mutex mutex;
    // 线程是否开始标记
    std::atomic<bool> isStarted{false};
    // 线程是否暂停标记
    std::atomic<bool> isPause{false};
    // 线程是否释放标记
    std::atomic<bool> isReleased{false};
    std::unique_ptr<std::thread> videoDecInputThread = nullptr;
    std::unique_ptr<std::thread> videoDecOutputThread = nullptr;
    std::unique_ptr<std::thread> audioDecInputThread = nullptr;
    std::unique_ptr<std::thread> audioDecOutputThread = nullptr;
    std::condition_variable doneCond;
    SampleInfo sampleInfo;
    CodecUserData *videoDecContext = nullptr;
    CodecUserData *audioDecContext = nullptr;
    OH_AudioStreamBuilder *builder = nullptr;
    OH_AudioRenderer *audioRenderer = nullptr;
    
    int64_t nowTimeStamp = 0;
    int64_t audioTimeStamp = 0;
    int64_t writtenSampleCnt = 0;
    int64_t audioBufferPts = 0;
    static constexpr int64_t MICROSECOND = 1000000;
    
    float speed = 1.0f;
};

#endif // VIDEO_CODEC_PLAYER_H