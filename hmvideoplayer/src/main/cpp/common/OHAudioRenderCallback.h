#ifndef HMVIDEOPLAYER_OHAUDIORENDERCALLBACK_H
#define HMVIDEOPLAYER_OHAUDIORENDERCALLBACK_H

#include <cstdint>
#include <ohaudio/native_audiostream_base.h>
class OHAudioRenderCallback {
public:
    // 该函数指针将指向用于写入音频数据的回调函数
    static int32_t OnRenderWriteData(OH_AudioRenderer *renderer, void *userData, void *buffer, int32_t length);
    static OH_AudioData_Callback_Result OnWriteDataCallback(OH_AudioRenderer *renderer, void *userData, void *audioData,
                                                            int32_t audioDataSize);

    // 该函数指针将指向用于处理音频播放流事件的回调函数
    static int32_t OnRenderStreamEvent(OH_AudioRenderer *renderer, void *userData, OH_AudioStream_Event event);
    static void OnOutputDeviceChangeCallback(OH_AudioRenderer *renderer, void *userData,
                                             OH_AudioStream_DeviceChangeReason reason);

    // 该函数指针将指向用于处理音频播放中断事件的回调函数
    static int32_t OnRenderInterruptEvent(OH_AudioRenderer *renderer, void *userData, OH_AudioInterrupt_ForceType type,
                                          OH_AudioInterrupt_Hint hint);
    static void OnInterruptCallback(OH_AudioRenderer *renderer, void *userData, OH_AudioInterrupt_ForceType type,
                                    OH_AudioInterrupt_Hint hint);

    // 该函数指针将指向用于处理音频播放错误结果的回调函数
    static int32_t OnRenderError(OH_AudioRenderer *renderer, void *userData, OH_AudioStream_Result error);
    static void OnErrorCallback(OH_AudioRenderer *renderer, void *userData, OH_AudioStream_Result error);
};

#endif //HMVIDEOPLAYER_OHAUDIORENDERCALLBACK_H
