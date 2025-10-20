#ifndef NATIVE_XCOMPONENT_PLUGIN_MANAGER_H
#define NATIVE_XCOMPONENT_PLUGIN_MANAGER_H

#include <ace/xcomponent/native_interface_xcomponent.h>
#include <js_native_api.h>
#include <js_native_api_types.h>
#include <napi/native_api.h>
#include <string>
#include <unordered_map>
#include "native_window/external_window.h"

#include "PluginRender.h"

namespace NativeXComponentSample {
class PluginManager {
public:
    ~PluginManager();

    static PluginManager *GetInstance() { return &PluginManager::pluginManager; }

    void SetNativeXComponent(std::string &id, OH_NativeXComponent *nativeXComponent);
    PluginRender *GetRender(std::string &id);
    void Export(napi_env env, napi_value exports);
    OHNativeWindow *pluginWindow_;

private:
    static PluginManager pluginManager;

    std::unordered_map<std::string, OH_NativeXComponent *> nativeXComponentMap;
    std::unordered_map<std::string, PluginRender *> pluginRenderMap;
};
} // namespace NativeXComponentSample
#endif // NATIVE_XCOMPONENT_PLUGIN_MANAGER_H