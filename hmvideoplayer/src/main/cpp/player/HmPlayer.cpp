#include "HmPlayer.h"
#include "CallBackContext.h"
#include "SampleInfo.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0xFF00
#define LOG_TAG "hmvideoplayer"

bool isRelease;
SampleInfo sampleInfo;

napi_value HmPlayer::RatePlay(napi_env env, napi_callback_info info) {
    double speed;
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    napi_get_value_double(env, args[0], &speed);
    MediaPlayManager::GetInstance().SetSpeed(static_cast<float>(speed));
    return nullptr;
}

napi_value HmPlayer::init(napi_env env, napi_callback_info info) {
    SampleInfo sampleInfo;
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        AVCODEC_SAMPLE_LOGD("init get params failed");
        return nullptr;
    }

    char url[256];
    size_t strSize;
    status = napi_get_value_string_utf8(env, args[0], url, sizeof(url), &strSize);
    if (status != napi_ok) {
        AVCODEC_SAMPLE_LOGD("init get url failed");
        return nullptr;
    }

    if (strSize > 0) {
        std::string urlStr(url);
        sampleInfo.url = urlStr;
    }
    isRelease = false;
    int32_t resultCode = MediaPlayManager::GetInstance().Init(sampleInfo);

    napi_value result;
    napi_create_int32(env, resultCode, &result);
    return result;
}

napi_value HmPlayer::play(napi_env env, napi_callback_info info) {
    int32_t resultCode = MediaPlayManager::GetInstance().Start();
    napi_value result;
    napi_create_int32(env, resultCode, &result);
    return result;
}

napi_value HmPlayer::pause(napi_env env, napi_callback_info info) {
    int32_t resultCode = MediaPlayManager::GetInstance().Pause();
    napi_value result;
    napi_create_int32(env, resultCode, &result);
    return result;
}

napi_value HmPlayer::resume(napi_env env, napi_callback_info info) {
    int32_t resultCode = MediaPlayManager::GetInstance().Resume();
    napi_value result;
    napi_create_int32(env, resultCode, &result);
    return result;
}

napi_value HmPlayer::stop(napi_env env, napi_callback_info info) {
    int32_t resultCode = MediaPlayManager::GetInstance().Stop();
    napi_value result;
    napi_create_int32(env, resultCode, &result);
    return result;
}

napi_value HmPlayer::release(napi_env env, napi_callback_info info) {
    isRelease = true;
    MediaPlayManager::GetInstance().Release();
    return nullptr;
}

napi_value HmPlayer::seek(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        LOGE("seek get params failed");
        return nullptr;
    }

    int64_t position = 0;
    bool lossless = true;
    status = napi_get_value_bigint_int64(env, args[0], &position, &lossless);
    if (status != napi_ok) {
        LOGE("seek get position failed");
        return nullptr;
    }

    int32_t resultCode = MediaPlayManager::GetInstance().Seek(position);
    napi_value result;
    napi_create_int32(env, resultCode, &result);
    return result;
}

napi_value HmPlayer::getDuration(napi_env env, napi_callback_info info) {
    int64_t resultCode = MediaPlayManager::GetInstance().GetDuration();
    napi_value result;
    napi_create_int64(env, resultCode, &result);
    return result;
}

void HmPlayer::onTimeUpdateCallJS(napi_env env, napi_value js_callBack, void *context, void *data) {
    CallBackContext *argContext = reinterpret_cast<CallBackContext *>(data);
    if (argContext == nullptr) {
        return;
    }

    napi_get_reference_value(env, argContext->callbackRef, &js_callBack);
    napi_value argv;
    napi_create_int32(env, argContext->timestamp, &argv);
    napi_call_function(env, nullptr, js_callBack, 1, &argv, nullptr);
    if (argContext && isRelease) {
        napi_delete_reference(env, argContext->callbackRef);
        delete argContext;
        argContext = nullptr;
    }
}

napi_value HmPlayer::onTimeUpdate(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value js_callback;
    napi_status status = napi_get_cb_info(env, info, &argc, &js_callback, nullptr, nullptr);
    if (status != napi_ok) {
        LOGE("onTimeUpdate get params failed");
        return nullptr;
    }

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, js_callback, &valueType);
    if (valueType != napi_valuetype::napi_function) {
        return nullptr;
    }

    napi_value workName;
    napi_create_string_utf8(env, "onTimeUpdate", NAPI_AUTO_LENGTH, &workName);
    napi_create_threadsafe_function(env, nullptr, nullptr, workName, 0, 1, nullptr, nullptr, nullptr,
                                    onTimeUpdateCallJS, &sampleInfo.timestampFn);

    auto argContext = new CallBackContext();
    sampleInfo.callbackData = argContext;

    argContext->env = env;
    napi_create_reference(env, js_callback, 1, &argContext->callbackRef);
    MediaPlayManager::GetInstance().setTimeStampCallback(sampleInfo);
    return nullptr;
}

void stateChangeCallback(napi_env env, napi_value js_callBack, void *context, void *contextData) {
    CallBackContext *callBackContext = reinterpret_cast<CallBackContext *>(contextData);
    if (callBackContext == nullptr) {
        LOGE("stateChangeCallback function callBackContext nullptr");
        return;
    }

    napi_value callback = nullptr;
    napi_get_reference_value(callBackContext->env, callBackContext->callbackRef, &callback);

    napi_value argv;
    napi_create_int32(callBackContext->env, callBackContext->state, &argv);
    napi_call_function(callBackContext->env, nullptr, callback, 1, &argv, nullptr);

    LOGD("HmPlayer stateChangeCallback is execute...%{public}ld", callBackContext->state);
    if (callBackContext && isRelease) {
        napi_delete_reference(callBackContext->env, callBackContext->callbackRef);
        delete callBackContext;
        callBackContext = nullptr;
    }
}

napi_value HmPlayer::onStateChange(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value js_callback;
    napi_status status = napi_get_cb_info(env, info, &argc, &js_callback, nullptr, nullptr);
    if (status != napi_ok) {
        LOGE("HmPlayer onStateChange get params failed");
        return nullptr;
    }

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, js_callback, &valueType);
    if (valueType != napi_valuetype::napi_function) {
        LOGE("HmPlayer js_callback failed");
        return nullptr;
    }

    napi_value workName;
    napi_create_string_utf8(env, "onStateChange", NAPI_AUTO_LENGTH, &workName);
    napi_create_threadsafe_function(env, nullptr, nullptr, workName, 0, 1, nullptr, nullptr, nullptr,
                                    stateChangeCallback, &sampleInfo.stateChangeFn);

    auto callBackContext = new CallBackContext();
    sampleInfo.stateCallbackData = callBackContext;

    callBackContext->env = env;
    napi_create_reference(env, js_callback, 1, &callBackContext->callbackRef);

    MediaPlayManager::GetInstance().setStateChangeCallback(sampleInfo);
    LOGD("HmPlayer onStateChange is execute ...");
    return nullptr;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor classProp[] = {
        {"init", nullptr, HmPlayer::init, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"play", nullptr, HmPlayer::play, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"pause", nullptr, HmPlayer::pause, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"resume", nullptr, HmPlayer::resume, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"stop", nullptr, HmPlayer::stop, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"release", nullptr, HmPlayer::release, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getDuration", nullptr, HmPlayer::getDuration, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"seek", nullptr, HmPlayer::seek, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"onTimeUpdate", nullptr, HmPlayer::onTimeUpdate, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"onStateChange", nullptr, HmPlayer::onStateChange, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"ratePlay", nullptr, HmPlayer::RatePlay, nullptr},
    };

    NativeXComponentSample::PluginManager::GetInstance()->Export(env, exports);
    napi_define_properties(env, exports, sizeof(classProp) / sizeof(classProp[0]), classProp);
    return exports;
}
EXTERN_C_END

static napi_module PlayerModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "hmvideoplayer",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterPlayerModule(void) { napi_module_register(&PlayerModule); }