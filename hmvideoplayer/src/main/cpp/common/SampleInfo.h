#ifndef AVCODEC_SAMPLE_INFO_H
#define AVCODEC_SAMPLE_INFO_H

#include <cstdint>
#include <multimedia/player_framework/native_avcodec_videoencoder.h>
#include <string>
#include <napi/native_api.h>
#include <native_buffer/native_buffer.h>
#include "multimedia/player_framework/native_avcodec_base.h"
#include "multimedia/player_framework/native_avbuffer.h"

struct SampleInfo {
    // 在线视频链接
    std::string url = "";
    // 本地视频
    int32_t inputFd = -1;
    int64_t inputFileOffset = 0;
    int64_t inputFileSize = 0;

    // 视频编解码mime类型
    std::string videoCodecMime = "";
    std::string audioCodecMime = "";
    // 视频宽度
    int32_t videoWidth = 0;
    // 视频高度
    int32_t videoHeight = 0;
    // 视频旋转角度
    int32_t videoRotation = 0;
    // 视频采样率
    double frameRate = 0.0;
    // 视频比特率
    int64_t bitrate = 10 * 1024 * 1024; // 10Mbps;
    // 帧间隔
    int64_t frameInterval = 0;
    // 视频像素
    OH_AVPixelFormat pixelFormat = AV_PIXEL_FORMAT_NV12;
    // 视频编解码码率模式：恒定码率模式
    uint32_t bitrateMode = CBR;
    // 关键帧间隔
    int32_t iFrameInterval = 100;
    // 视频YUV值域标志：1表示full range
    int32_t rangFlag = 1;
    // 音频采样格式
    int32_t audioSampleFormat = 0;
    // 音频样本位数
    int32_t audioSampleRate = 0;
    // 音频通道数
    int32_t audioChannelCount = 0;
    // 音频编码通道布局
    int64_t audioChannelLayout = 0;
    // 时长，微妙
    int64_t duration = 0;

    // 进度时间戳线程安全函数
    napi_threadsafe_function timestampFn;
    // 回调函数参数
    void *callbackData = nullptr;

    // 播放状态变化线程安全函数
    napi_threadsafe_function stateChangeFn;
    // 播放状态回调函数参数
    void *stateCallbackData = nullptr;

    // 处理音频并发打断线程安全函数
    napi_threadsafe_function audioInterruptFn;
    // 音频并发打断回调函数参数
    void *interruptCallbackData = nullptr;

    // 音频流输出设备变化及原因线程安全函数
    napi_threadsafe_function outputDeviceChangeFn;
    // 音频并发打断回调函数参数
    void *outputDeviceChangeCallbackData = nullptr;

    // 处理编解码码流发生变化线程安全函数
    napi_threadsafe_function avcodecStreamChangeFn;
    // 音频并发打断回调函数参数
    void *avcodecStreamCallbackData = nullptr;

    // 处理音频错误线程安全函数
    napi_threadsafe_function audioErrorFn;
    // 音频错误回调函数参数
    void *audioErrorCallbackData = nullptr;

    // 处理编解码错误线程安全函数
    napi_threadsafe_function avcodecErrorFn;
    // 编解码错误回调函数参数
    void *avcodecErrorCallbackData = nullptr;

    // 视频是否HDRVivid
    int32_t isHDRVivid = 0;
    // 视频编码档次：HEVC编码档次为主档次
    int32_t hevcProfile = HEVC_PROFILE_MAIN;
    // 视频色域：BT2020色域
    OH_ColorPrimary primary = COLOR_PRIMARY_BT2020;
    // 视频转移特性：PQ传递函数
    OH_TransferCharacteristic transfer = TRANSFER_CHARACTERISTIC_PQ;
    // 矩阵系统：BT2020_CL转换矩阵
    OH_MatrixCoefficient matrix = MATRIX_COEFFICIENT_BT2020_CL;
    // 本地平台化窗口，表示图形队列的生产者端
    OHNativeWindow *window = nullptr;
};

#endif // AVCODEC_SAMPLE_INFO_H