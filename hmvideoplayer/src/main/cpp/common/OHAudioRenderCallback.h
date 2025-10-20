#ifndef HMVIDEOPLAYER_OHAUDIORENDERCALLBACK_H
#define HMVIDEOPLAYER_OHAUDIORENDERCALLBACK_H

#include <ohaudio/native_audiostream_base.h>
class OHAudioRenderCallback {
    public:
    // 该函数指针将指向用于写入音频数据的回调函数
    static int32_t OnRenderWriteData(OH_AudioRenderer *renderer, void *userData, void *buffer, int32_t length);
    // 该函数指针将指向用于处理音频播放流事件的回调函数
    static int32_t OnRenderStreamEvent(OH_AudioRenderer *renderer, void *userData, OH_AudioStream_Event event);
    // 该函数指针将指向用于处理音频播放中断事件的回调函数
    static int32_t OnRenderInterruptEvent(OH_AudioRenderer *renderer, void *userData, OH_AudioInterrupt_ForceType type,OH_AudioInterrupt_Hint hint);
    // 该函数指针将指向用于处理音频播放错误结果的回调函数
    static int32_t OnRenderError(OH_AudioRenderer *renderer, void *userData, OH_AudioStream_Result error);
};

#endif //HMVIDEOPLAYER_OHAUDIORENDERCALLBACK_H
