#ifndef NATIVE_XCOMPONENT_PLUGIN_RENDER_H
#define NATIVE_XCOMPONENT_PLUGIN_RENDER_H

#include <ace/xcomponent/native_interface_xcomponent.h>
#include <napi/native_api.h>
#include <string>
#include <unordered_map>

#include "EglCore.h"

namespace NativeXComponentSample {
class PluginRender {
public:
    explicit PluginRender(std::string &id);
    ~PluginRender() {
        if (eglCore != nullptr) {
            eglCore->Release();
            delete eglCore;
            eglCore = nullptr;
        }
    }
    static PluginRender *GetInstance(std::string &id);
    static void Release(std::string &id);
    void Export(napi_env env, napi_value exports);
    void OnSurfaceChanged(OH_NativeXComponent *component, void *window);
    void OnTouchEvent(OH_NativeXComponent *component, void *window);
    void RegisterCallback(OH_NativeXComponent *nativeXComponent);

public:
    static std::unordered_map<std::string, PluginRender *> instance;
    EGLCore *eglCore;
    std::string id;
    static int32_t hasDraw;
    static int32_t hasChangeColor;

private:
    OH_NativeXComponent_Callback renderCallback;
    OH_NativeXComponent_MouseEvent_Callback mouseCallback;
};
} // namespace NativeXComponentSample
#endif // NATIVE_XCOMPONENT_PLUGIN_RENDER_H