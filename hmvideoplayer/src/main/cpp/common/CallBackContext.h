#ifndef HMVIDEOPLAYER_CALLBACKCONTEXT_H
#define HMVIDEOPLAYER_CALLBACKCONTEXT_H

#include <cstdint>
#include <napi/native_api.h>

struct CallBackContext {
    napi_env env = nullptr;
    napi_ref callbackRef = nullptr;
    int64_t timestamp;
    int64_t state;
};
#endif //HMVIDEOPLAYER_CALLBACKCONTEXT_H
