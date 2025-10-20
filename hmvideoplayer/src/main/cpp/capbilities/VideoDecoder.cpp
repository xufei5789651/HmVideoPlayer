#include "VideoDecoder.h"
#include "AudioVideoDecodeCallback.h"
#include "utils/LogUtils.h"
#include "utils/ResultCode.h"

namespace {
constexpr int LIMIT_LOGD_FREQUENCY = 50;
} // namespace

VideoDecoder::~VideoDecoder() { Release(); }

int32_t VideoDecoder::Create(const std::string &videoCodecMime) {
    decoder = OH_VideoDecoder_CreateByMime(videoCodecMime.c_str());
    if (decoder == nullptr) {
        LOGE("VideoDecoder Create failed");
        return RESULT_CODE_ERROR;
    }
    return RESULT_CODE_OK;
}

int32_t VideoDecoder::SetCallback(CodecUserData *codecUserData) {
    int32_t ret = AV_ERR_OK;
    // 注册异步回调函数，让应用可以响应视频解码器生成的事件。
    // 在调用OH_VideoDecoder_Prepare接口之前，必须调用此接口
    ret = OH_VideoDecoder_RegisterCallback(
        decoder,
        {AudioVideoDecodeCallback::OnCodecError, AudioVideoDecodeCallback::OnCodecFormatChange,
         AudioVideoDecodeCallback::OnNeedInputBuffer, AudioVideoDecodeCallback::OnNewOutputBuffer},
        codecUserData);
    if (ret != AV_ERR_OK) {
        LOGE("Set callback failed, ret: %{public}d", ret);
    }
    return RESULT_CODE_OK;
}

int32_t VideoDecoder::Configure(const SampleInfo &sampleInfo) {
    OH_AVFormat *format = OH_AVFormat_Create();
    if (format == nullptr) {
        LOGE("AVFormat create failed");
        return RESULT_CODE_ERROR;
    }

    OH_AVFormat_SetIntValue(format, OH_MD_KEY_ROTATION, sampleInfo.videoRotation);
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_HEIGHT, sampleInfo.videoWidth);
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_WIDTH, sampleInfo.videoHeight);
    OH_AVFormat_SetDoubleValue(format, OH_MD_KEY_FRAME_RATE, sampleInfo.frameRate);
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_PIXEL_FORMAT, sampleInfo.pixelFormat);

    LOGI("====== VideoDecoder config ======");
    LOGI("%{public}d*%{public}d, %{public}.1ffps", sampleInfo.videoWidth, sampleInfo.videoHeight, sampleInfo.frameRate);
    LOGI("====== VideoDecoder config ======");

    int ret = OH_VideoDecoder_Configure(decoder, format);
    if (ret != AV_ERR_OK) {
        LOGE("Config failed, ret: %{public}d", ret);
        return RESULT_CODE_ERROR;
    }

    OH_AVFormat_Destroy(format);
    format = nullptr;
    return RESULT_CODE_OK;
}

int32_t VideoDecoder::Config(const SampleInfo &sampleInfo, CodecUserData *codecUserData) {
    if (decoder == nullptr) {
        LOGE("Decoder is null");
        return RESULT_CODE_ERROR;
    }
    if (codecUserData == nullptr) {
        LOGE("Invalid param: codecUserData");
        return RESULT_CODE_ERROR;
    }

    // 配置视频解码器
    int32_t ret = Configure(sampleInfo);
    if (ret != AV_ERR_OK) {
        LOGE("Configure failed");
        return RESULT_CODE_ERROR;
    }

    // 设置surface
    if (sampleInfo.window != nullptr) {
        // 设置输出surface以提供视频解码输出
        int ret = OH_VideoDecoder_SetSurface(decoder, sampleInfo.window);

        if (ret != AV_ERR_OK && sampleInfo.window == nullptr) {
            LOGE("Set surface failed, ret: %{public}d", ret);
            return RESULT_CODE_ERROR;
        }
    }

    // 注册回调函数
    ret = SetCallback(codecUserData);
    if (ret != AV_ERR_OK) {
        LOGE("Set callback failed, ret: %{public}d", ret);
        return RESULT_CODE_ERROR;
    }

    // 解码器就绪
    {
        // 准备解码器的内部资源，在调用该接口之前，必须调用OH_VideoDecoder_Configure接口
        int ret = OH_VideoDecoder_Prepare(decoder);
        if (ret != AV_ERR_OK) {
            LOGE("Set callback failed, ret: %{public}d", ret);
            return RESULT_CODE_ERROR;
        }
    }

    return RESULT_CODE_OK;
}

int32_t VideoDecoder::Start() {
    if (decoder == nullptr) {
        LOGE("Decoder is null");
        return RESULT_CODE_ERROR;
    }
    // 调用OH_VideoDecoder_Prepare接口成功后调用此接口启动解码器。
    // 成功启动后，解码器将开始报告注册的回调事件
    int ret = OH_VideoDecoder_Start(decoder);
    if (ret != AV_ERR_OK) {
        LOGE("Start failed, ret: %{public}d", ret);
        return RESULT_CODE_ERROR;
    }
    return RESULT_CODE_OK;
}

int32_t VideoDecoder::PushInputBuffer(CodecBufferInfo &info) {
    if (decoder == nullptr) {
        LOGE("Decoder is null");
        return RESULT_CODE_ERROR;
    }
    int32_t ret = OH_VideoDecoder_PushInputBuffer(decoder, info.bufferIndex);
    if (ret != AV_ERR_OK) {
        LOGE("Push input data failed");
        return RESULT_CODE_ERROR;
    }
    return RESULT_CODE_OK;
}

int32_t VideoDecoder::FreeOutputBuffer(uint32_t bufferIndex, bool render) {
    if (decoder == nullptr) {
        LOGE("Decoder is null");
        return RESULT_CODE_ERROR;
    }

    int32_t ret = RESULT_CODE_ERROR;
    if (render) {
        // 将index对应的输出缓冲区返回到解码器，缓冲中携带解码输出数据
        // 并通知解码器完成在输出surface上渲染，输出缓冲包含解码数据
        ret = OH_VideoDecoder_RenderOutputBuffer(decoder, bufferIndex);
    } else {
        // 将处理后的输出缓冲区返回到解码器
        ret = OH_VideoDecoder_FreeOutputBuffer(decoder, bufferIndex);
    }

    if (ret != AV_ERR_OK) {
        LOGE("Free output data failed");
        return RESULT_CODE_ERROR;
    }
    return RESULT_CODE_OK;
}

int32_t VideoDecoder::Stop() {
    if (decoder != nullptr) {
        OH_VideoDecoder_Flush(decoder);
        OH_VideoDecoder_Stop(decoder);
    }
    return RESULT_CODE_OK;
}

int32_t VideoDecoder::Release() {
    if (decoder != nullptr) {
        OH_VideoDecoder_Destroy(decoder);
        decoder = nullptr;
    }
    return RESULT_CODE_OK;
}