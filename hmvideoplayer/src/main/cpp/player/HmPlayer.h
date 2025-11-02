#ifndef VIDEO_CODEC_SAMPLE_PLAYER_NATIVE_H
#define VIDEO_CODEC_SAMPLE_PLAYER_NATIVE_H

#include <js_native_api.h>
#include <js_native_api_types.h>
#include <memory>
#include <uv.h>
#include "napi/native_api.h"
#include "MediaPlayManager.h"
#include "dfx/error/AVCodecSampleError.h"
#include "AVCodecSampleLog.h"
#include "PluginManager.h"

class HmPlayer {
private:
    static void onTimeUpdateCallJS(napi_env env, napi_value js_callBack,void *context,void *data);
    
public:
    static napi_value initWithURL(napi_env env, napi_callback_info info);
    
    static napi_value initWithLocal(napi_env env, napi_callback_info info);
    
    static napi_value play(napi_env env, napi_callback_info info);
    
    static napi_value pause(napi_env env, napi_callback_info info);
    
    static napi_value resume(napi_env env, napi_callback_info info);
    
    static napi_value stop(napi_env env, napi_callback_info info);
    
    static napi_value release(napi_env env, napi_callback_info info);
    
    static napi_value getDuration(napi_env env, napi_callback_info info);
    
    static napi_value seek(napi_env env, napi_callback_info info);
    
    static napi_value onTimeUpdate(napi_env env, napi_callback_info info);
    
    static napi_value onStateChange(napi_env env, napi_callback_info info);
    
    static napi_value RatePlay(napi_env env, napi_callback_info info);
    
    static napi_value onAudioInterrupt(napi_env env, napi_callback_info info);
};
#endif // VIDEO_CODEC_SAMPLE_PLAYER_NATIVE_H