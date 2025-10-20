#ifndef HMVIDEOPLAYER_LOGUTILS_H
#define HMVIDEOPLAYER_LOGUTILS_H
#include "hilog/log.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0x3200  // 全局domain宏，标识业务领域
#define LOG_TAG "hmvideoplayer" // 全局tag宏，标识模块日志tag

#define LOGD(...) OH_LOG_DEBUG(LOG_APP, __VA_ARGS__)
#define LOGI(...) OH_LOG_INFO(LOG_APP, __VA_ARGS__)
#define LOGW(...) OH_LOG_WARN(LOG_APP, __VA_ARGS__)
#define LOGE(...) OH_LOG_ERROR(LOG_APP, __VA_ARGS__)
#define LOGF(...) OH_LOG_FATAL(LOG_APP, __VA_ARGS__)
#endif //HMVIDEOPLAYER_LOGUTILS_H
