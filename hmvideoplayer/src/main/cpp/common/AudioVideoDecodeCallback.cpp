#include "AudioVideoDecodeCallback.h"
#include "AVCodecSampleLog.h"
#include "CodecUserData.h"
#include "SampleInfo.h"

void AudioVideoDecodeCallback::OnCodecError(OH_AVCodec *codec, int32_t errorCode, void *userData) {
    (void)codec;
    (void)errorCode;
    (void)userData;
    AVCODEC_SAMPLE_LOGE("On codec error, error code: %{public}d", errorCode);
}

void AudioVideoDecodeCallback::OnCodecFormatChange(OH_AVCodec *codec, OH_AVFormat *format, void *userData) {
    AVCODEC_SAMPLE_LOGI("On codec format change");
}

void AudioVideoDecodeCallback::OnNeedInputBuffer(OH_AVCodec *codec, uint32_t index, OH_AVBuffer *buffer, void *userData) {
    if (userData == nullptr) {
        return;
    }
    (void)codec;
    CodecUserData *codecUserData = static_cast<CodecUserData *>(userData);
    std::unique_lock<std::mutex> lock(codecUserData->inputMutex);
    codecUserData->inputBufferInfoQueue.emplace(index, buffer);
    codecUserData->inputCond.notify_all();
}

void AudioVideoDecodeCallback::OnNewOutputBuffer(OH_AVCodec *codec, uint32_t index, OH_AVBuffer *buffer, void *userData) {
    if (userData == nullptr) {
        return;
    }
    (void)codec;
    CodecUserData *codecUserData = static_cast<CodecUserData *>(userData);
    std::unique_lock<std::mutex> lock(codecUserData->outputMutex);
    codecUserData->outputBufferInfoQueue.emplace(index, buffer);
    codecUserData->outputCond.notify_all();
}
