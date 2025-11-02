#ifndef HMVIDEOPLAYER_CALLBACKCONTEXT_H
#define HMVIDEOPLAYER_CALLBACKCONTEXT_H

#include <cstdint>
#include <napi/native_api.h>

struct CallBackContext {
    napi_env env = nullptr;
    napi_ref callbackRef = nullptr;
    int32_t forceType;
    int32_t routingChange;
    int32_t hint;
    int32_t errorAudioCode;
    int32_t errorAvcodecCode;
    int32_t audioSampleFormat;
    int32_t audioSampleRate;
    int32_t audioChannelCount;
    int64_t timestamp;
    int64_t state;
    int32_t videoWidth;
    int32_t videoHeight;
    double videoFrameRate;
};
#endif // HMVIDEOPLAYER_CALLBACKCONTEXT_H
