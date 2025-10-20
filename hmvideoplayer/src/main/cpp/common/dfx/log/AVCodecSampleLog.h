#ifndef AVCODEC_SAMPLE_LOG_H
#define AVCODEC_SAMPLE_LOG_H

#include <hilog/log.h>
#include <cinttypes>

#undef LOG_DOMAIN
#define LOG_DOMAIN 0x0002B66

#define AVCODEC_SAMPLE_LOG_FREQ_LIMIT(frequency)                                                                       \
    if (1) {                                                                                                           \
        thread_local uint64_t currentTimes = 0;                                                                        \
        if (currentTimes++ % ((uint64_t)(frequency)) != 0) {                                                           \
            break;                                                                                                     \
        }                                                                                                              \
    }

#define AVCODEC_SAMPLE_LOG(func, fmt, args...)                                                                         \
    do {                                                                                                               \
        (void)func(LOG_APP, "{%{public}s():%{public}d} " fmt, __FUNCTION__, __LINE__, ##args);                         \
    } while (0)

#define AVCODEC_SAMPLE_LOGF(fmt, ...) AVCODEC_SAMPLE_LOG(OH_LOG_FATAL, fmt, ##__VA_ARGS__)
#define AVCODEC_SAMPLE_LOGE(fmt, ...) AVCODEC_SAMPLE_LOG(OH_LOG_ERROR, fmt, ##__VA_ARGS__)
#define AVCODEC_SAMPLE_LOGW(fmt, ...) AVCODEC_SAMPLE_LOG(OH_LOG_WARN, fmt, ##__VA_ARGS__)
#define AVCODEC_SAMPLE_LOGI(fmt, ...) AVCODEC_SAMPLE_LOG(OH_LOG_INFO, fmt, ##__VA_ARGS__)
#define AVCODEC_SAMPLE_LOGD(fmt, ...) AVCODEC_SAMPLE_LOG(OH_LOG_DEBUG, fmt, ##__VA_ARGS__)
#define AVCODEC_SAMPLE_LOGD_LIMIT(frequency, fmt, ...)                                                                 \
    do {                                                                                                               \
        AVCODEC_SAMPLE_LOG_FREQ_LIMIT(frequency);                                                                      \
        AVCODEC_SAMPLE_LOGD(fmt, ##__VA_ARGS__);                                                                       \
    } while (0)

#define CHECK_AND_RETURN_RET_LOG(cond, ret, fmt, ...)                                                                  \
    do {                                                                                                               \
        if (!(cond)) {                                                                                                 \
            AVCODEC_SAMPLE_LOGE(fmt, ##__VA_ARGS__);                                                                   \
            return ret;                                                                                                \
        }                                                                                                              \
    } while (0)

#define CHECK_AND_RETURN_LOG(cond, fmt, ...)                                                                           \
    do {                                                                                                               \
        if (!(cond)) {                                                                                                 \
            AVCODEC_SAMPLE_LOGE(fmt, ##__VA_ARGS__);                                                                   \
            return;                                                                                                    \
        }                                                                                                              \
    } while (0)

#define CHECK_AND_BREAK_LOG(cond, fmt, ...)                                                                            \
    if (1) {                                                                                                           \
        if (!(cond)) {                                                                                                 \
            AVCODEC_SAMPLE_LOGW(fmt, ##__VA_ARGS__);                                                                   \
            break;                                                                                                     \
        }                                                                                                              \
    } else                                                                                                             \
        void(0)

#define CHECK_AND_CONTINUE_LOG(cond, fmt, ...)                                                                         \
    if (1) {                                                                                                           \
        if (!(cond)) {                                                                                                 \
            AVCODEC_SAMPLE_LOGW(fmt, ##__VA_ARGS__);                                                                   \
            continue;                                                                                                  \
        }                                                                                                              \
    } else                                                                                                             \
        void(0)

#endif // AVCODEC_SAMPLE_LOG_H