#include "AudioDecoder.h"
#include "utils/LogUtils.h"
#include "utils/ResultCode.h"

AudioDecoder::~AudioDecoder() { Release(); }

int32_t AudioDecoder::Create(const std::string &codecMime) {
    // 根据MIME类型创建音频解码器实例
    decoder = OH_AudioCodec_CreateByMime(codecMime.c_str(), false);
    if (decoder == nullptr) {
        LOGE("Create failed");
        return RESULT_CODE_ERROR;
    }
    return RESULT_CODE_OK;
}

int32_t AudioDecoder::SetCallback(CodecUserData *codecUserData) {
    int32_t ret = AV_ERR_OK;
    // todo 设置异步回调函数，使应用可以响应音频解码器生成的事件
    // todo 需要OH_AudioDecoder_SetCallback()该方法替换
    ret = OH_AudioCodec_RegisterCallback(
        decoder,
        {AudioVideoDecodeCallback::OnCodecError, AudioVideoDecodeCallback::OnCodecFormatChange,
         AudioVideoDecodeCallback::OnNeedInputBuffer, AudioVideoDecodeCallback::OnNewOutputBuffer},
        codecUserData);
    if (ret != AV_ERR_OK) {
        LOGE("Set callback failed, ret: %{public}d", ret);
        return RESULT_CODE_ERROR;
    }
    return RESULT_CODE_OK;
}

int32_t AudioDecoder::Configure(const SampleInfo &sampleInfo) {
    OH_AVFormat *format = OH_AVFormat_Create();
    if (format == nullptr) {
        LOGE("AVFormat create failed");
        return RESULT_CODE_ERROR;
    }
    // 配置音轨的信息：原始格式、通道、采样率、通道布局
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_AUDIO_SAMPLE_FORMAT, SAMPLE_S16LE);
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_AUD_CHANNEL_COUNT, sampleInfo.audioChannelCount);
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_AUD_SAMPLE_RATE, sampleInfo.audioSampleRate);
    OH_AVFormat_SetLongValue(format, OH_MD_KEY_CHANNEL_LAYOUT, sampleInfo.audioChannelLayout);
    LOGI("====== AudioDecoder config ======");

    // 要配置音频解码器，通常需要配置从容器中提取的音频描述信息 
    //todo 使用OH_AudioDecoder_Configure()替换
    int ret = OH_AudioCodec_Configure(decoder, format);
    LOGI("====== AudioDecoder config ======");
    if (ret != AV_ERR_OK) {
        LOGE("Config failed, ret: %{public}d", ret);
        return RESULT_CODE_ERROR;
    }
    // 释放指针
    OH_AVFormat_Destroy(format);
    format = nullptr;
    return RESULT_CODE_OK;
}

int32_t AudioDecoder::Config(const SampleInfo &sampleInfo, CodecUserData *codecUserData) {
    if (decoder == nullptr) {
        LOGE("Decoder is null");
        return RESULT_CODE_ERROR;
    }
    if (codecUserData == nullptr) {
        LOGE("Invalid param: codecUserData");
        return RESULT_CODE_ERROR;
    }
    // 音频解码配置信息
    int32_t ret = Configure(sampleInfo);
    if (ret != AV_ERR_OK) {
        LOGE("Configure failed");
        return RESULT_CODE_ERROR;
    }

     // 设置音频解码回调函数
    ret = SetCallback(codecUserData);
    if (ret != AV_ERR_OK) {
        LOGE("Set callback failed, ret: %{public}d", ret);
        return RESULT_CODE_ERROR;
    }

    // 解码器就绪
    {
        int ret = OH_AudioCodec_Prepare(decoder);
        if (ret != AV_ERR_OK) {
            LOGE("Prepare failed, ret: %{public}d", ret);
            return RESULT_CODE_ERROR;
        }
    }

    return RESULT_CODE_OK;
}

int32_t AudioDecoder::Start() {
    if (decoder == nullptr) {
        LOGE("Decoder is null");
        return RESULT_CODE_ERROR;
    }
    // Prepare成功后调用此接口启动编解码
    // 启动后，编解码器将开始上报OH_AVCodecOnNeedInputBuffer事件
    int ret = OH_AudioCodec_Start(decoder);
    if (ret != AV_ERR_OK) {
        LOGE("Start failed, ret: %{public}d", ret);
        return RESULT_CODE_ERROR;
    }
    return RESULT_CODE_OK;
}

int32_t AudioDecoder::PushInputBuffer(CodecBufferInfo &info) {
    if (decoder == nullptr) {
        LOGE("Decoder is null");
        return RESULT_CODE_ERROR;
    }
    // 设置数据缓冲区的pts、size、offset、flags高频属性参数 
    int32_t ret = OH_AVBuffer_SetBufferAttr(reinterpret_cast<OH_AVBuffer *>(info.buffer), &info.attr);
     if (ret != AV_ERR_OK) {
        LOGE("Set avbuffer attr failed");
        return RESULT_CODE_ERROR;
    }
    // 写入待解码的数据
    ret = OH_AudioCodec_PushInputBuffer(decoder, info.bufferIndex);
    if (ret != AV_ERR_OK) {
        LOGE("Push input data failed");
        return RESULT_CODE_ERROR;
    }
    return RESULT_CODE_OK;
}

int32_t AudioDecoder::FreeOutputBuffer(uint32_t bufferIndex, bool render) {
    if (decoder == nullptr) {
        LOGE("Decoder is null");
        return RESULT_CODE_ERROR;
    }

    int32_t ret = RESULT_CODE_OK;
    ret = OH_AudioCodec_FreeOutputBuffer(decoder, bufferIndex);
    
    if (ret != AV_ERR_OK) {
        LOGE("Free output data failed");
        return RESULT_CODE_ERROR;
    }
    return RESULT_CODE_OK;
}

int32_t AudioDecoder::Stop() {
    if (decoder != nullptr) {
        OH_AudioCodec_Flush(decoder);
        OH_AudioCodec_Stop(decoder);
    }
    return RESULT_CODE_OK;
}

int32_t AudioDecoder::Release() {
    if (decoder != nullptr) {
        OH_AudioCodec_Destroy(decoder);
        decoder = nullptr;
    }
    return RESULT_CODE_OK;
}