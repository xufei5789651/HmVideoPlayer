#include "OHAudioRenderCallback.h"
#include "CallBackContext.h"
#include "CodecUserData.h"
#include "SampleInfo.h"
#include "utils/LogUtils.h"
#include "utils/ResultCode.h"

constexpr int32_t BYTES_PER_SAMPLE_2 = 2;

int32_t OHAudioRenderCallback::OnRenderWriteData(OH_AudioRenderer *renderer, void *userData, void *buffer, int32_t length) {
    (void)renderer;
    (void)length;
    CodecUserData *codecUserData = static_cast<CodecUserData *>(userData);

    // 将要播放的数据按长度写入缓冲区
    uint8_t *dest = (uint8_t *)buffer;
    size_t index = 0;
    std::unique_lock<std::mutex> lock(codecUserData->outputMutex);
    
    // 从队列中检索要播放的数据长度
    while (!codecUserData->renderQueue.empty() && index < length) {
        dest[index++] = codecUserData->renderQueue.front();
        codecUserData->renderQueue.pop();
    }
    LOGD("render BufferLength:%{public}d Out buffer count: %{public}u, renderQueue.size: %{public}u "
                        "renderReadSize: %{public}u",
                        length, codecUserData->outputFrameCount, (unsigned int)codecUserData->renderQueue.size(),
                        (unsigned int)index);
    
    codecUserData->frameWrittenForSpeed += length / codecUserData->speed / codecUserData->sampleInfo->
        audioChannelCount / BYTES_PER_SAMPLE_2;
    codecUserData->currentPosAudioBufferPts = codecUserData->endPosAudioBufferPts - codecUserData->
        renderQueue.size() / codecUserData->sampleInfo->audioSampleRate / codecUserData->sampleInfo->
        audioChannelCount / BYTES_PER_SAMPLE_2;
    
    if (codecUserData->renderQueue.size() < length) {
        codecUserData->renderCond.notify_all();
    }
    return 0;
}

OH_AudioData_Callback_Result OHAudioRenderCallback::OnWriteDataCallback(OH_AudioRenderer *renderer, void *userData, void *audioData, int32_t audioDataSize) {
    (void)renderer;
    CodecUserData *codecUserData = static_cast<CodecUserData *>(userData);

    // 将要播放的数据按长度写入缓冲区
    uint8_t *dest = (uint8_t *)audioData;
    size_t index = 0;
    std::unique_lock<std::mutex> lock(codecUserData->outputMutex);
    
    // 从队列中检索要播放的数据长度
    while (!codecUserData->renderQueue.empty() && index < audioDataSize) {
        dest[index++] = codecUserData->renderQueue.front();
        codecUserData->renderQueue.pop();
    }
    LOGD("render BufferLength:%{public}d Out buffer count: %{public}u, renderQueue.size: %{public}u "
                        "renderReadSize: %{public}u",
                        audioDataSize, codecUserData->outputFrameCount, (unsigned int)codecUserData->renderQueue.size(),
                        (unsigned int)index);
    
    codecUserData->frameWrittenForSpeed += audioDataSize / codecUserData->speed / codecUserData->sampleInfo->
        audioChannelCount / BYTES_PER_SAMPLE_2;
    codecUserData->currentPosAudioBufferPts = codecUserData->endPosAudioBufferPts - codecUserData->
        renderQueue.size() / codecUserData->sampleInfo->audioSampleRate / codecUserData->sampleInfo->
        audioChannelCount / BYTES_PER_SAMPLE_2;
    
    if (codecUserData->renderQueue.size() < audioDataSize) {
        codecUserData->renderCond.notify_all();
    }
    return AUDIO_DATA_CALLBACK_RESULT_VALID;
}

int32_t OHAudioRenderCallback::OnRenderStreamEvent(OH_AudioRenderer *renderer, void *userData,
                                                   OH_AudioStream_Event event) {
    LOGD("OHAudioRenderCallback OnRenderStreamEvent");
    if (userData == nullptr) {
        LOGE("OHAudioRenderCallback OnRenderStreamEvent userData is nullptr");
        return RESULT_CODE_ERROR;
    }
    (void)renderer;
    
    CodecUserData *param = reinterpret_cast<CodecUserData *>(userData);
    if (!param) {
        LOGE("OHAudioRenderCallback OnRenderStreamEvent CodecUserData is nullptr");
        return RESULT_CODE_ERROR;
    }
    
    CallBackContext *callback = reinterpret_cast<CallBackContext *>(param->sampleInfo->outputDeviceChangeCallbackData);
    if (!callback) {
        LOGE("OHAudioRenderCallback OnRenderStreamEvent CallBackContext is nullptr");
        return RESULT_CODE_ERROR;
    }
    
    callback->routingChange=event;
    
    napi_status result = napi_acquire_threadsafe_function(param->sampleInfo->outputDeviceChangeFn);
    if (result != napi_ok) {
        LOGE("OHAudioRenderCallback OnRenderStreamEvent start napi_acquire_threadsafe_function failed");
        return RESULT_CODE_ERROR;
    }

    result = napi_call_threadsafe_function(param->sampleInfo->outputDeviceChangeFn, callback, napi_tsfn_nonblocking);
    if (result != napi_ok) {
        LOGE("OHAudioRenderCallback OnRenderStreamEvent start napi_acquire_threadsafe_function failed");
        return RESULT_CODE_ERROR;
    }
    return RESULT_CODE_OK;
}

void OHAudioRenderCallback::OnOutputDeviceChangeCallback(OH_AudioRenderer *renderer, void *userData,
                                                   OH_AudioStream_DeviceChangeReason reason) {
    LOGD("OHAudioRenderCallback OnRenderStreamEvent");
    if (userData == nullptr) {
        LOGE("OHAudioRenderCallback OnRenderStreamEvent userData is nullptr");
        return;
    }
    (void)renderer;
    
    CodecUserData *param = reinterpret_cast<CodecUserData *>(userData);
    if (!param) {
        LOGE("OHAudioRenderCallback OnRenderStreamEvent CodecUserData is nullptr");
        return;
    }
    
    CallBackContext *callback = reinterpret_cast<CallBackContext *>(param->sampleInfo->outputDeviceChangeCallbackData);
    if (!callback) {
        LOGE("OHAudioRenderCallback OnRenderStreamEvent CallBackContext is nullptr");
        return;
    }
    
    callback->routingChange=reason;
    
    napi_status result = napi_acquire_threadsafe_function(param->sampleInfo->outputDeviceChangeFn);
    if (result != napi_ok) {
        LOGE("OHAudioRenderCallback OnRenderStreamEvent start napi_acquire_threadsafe_function failed");
        return;
    }

    result = napi_call_threadsafe_function(param->sampleInfo->outputDeviceChangeFn, callback, napi_tsfn_nonblocking);
    if (result != napi_ok) {
        LOGE("OHAudioRenderCallback OnRenderStreamEvent start napi_acquire_threadsafe_function failed");
        return;
    }
    return;
}

int32_t OHAudioRenderCallback::OnRenderInterruptEvent(OH_AudioRenderer *renderer, void *userData,
                                                      OH_AudioInterrupt_ForceType type, OH_AudioInterrupt_Hint hint) {
    LOGD("OHAudioRenderCallback OnRenderInterruptEvent");
    if (userData == nullptr) {
        LOGE("OHAudioRenderCallback OnRenderInterruptEvent userData is nullptr");
        return RESULT_CODE_ERROR;
    }
    (void)renderer;
    
    CodecUserData *param = reinterpret_cast<CodecUserData *>(userData);
    if (!param) {
        LOGE("OHAudioRenderCallback OnRenderInterruptEvent CodecUserData is nullptr");
        return RESULT_CODE_ERROR;
    }
    
    CallBackContext *callback = reinterpret_cast<CallBackContext *>(param->sampleInfo->interruptCallbackData);
    if (!callback) {
        LOGE("OHAudioRenderCallback OnRenderInterruptEvent CallBackContext is nullptr");
        return RESULT_CODE_ERROR;
    }
    
    callback->hint=hint;
    callback->forceType=type;

    napi_status result = napi_acquire_threadsafe_function(param->sampleInfo->audioInterruptFn);
    if (result != napi_ok) {
        LOGE("OHAudioRenderCallback OnRenderInterruptEvent start napi_acquire_threadsafe_function failed");
        return RESULT_CODE_ERROR;
    }

    result = napi_call_threadsafe_function(param->sampleInfo->audioInterruptFn, callback, napi_tsfn_nonblocking);
    if (result != napi_ok) {
        LOGE("OHAudioRenderCallback OnRenderInterruptEvent start napi_acquire_threadsafe_function failed");
        return RESULT_CODE_ERROR;
    }
    return RESULT_CODE_OK;
}

void OHAudioRenderCallback::OnInterruptCallback(OH_AudioRenderer *renderer, void *userData,
                                                      OH_AudioInterrupt_ForceType type, OH_AudioInterrupt_Hint hint) {
    LOGD("OHAudioRenderCallback OnRenderInterruptEvent");
    if (userData == nullptr) {
        LOGE("OHAudioRenderCallback OnRenderInterruptEvent userData is nullptr");
        return;
    }
    (void)renderer;
    
    CodecUserData *param = reinterpret_cast<CodecUserData *>(userData);
    if (!param) {
        LOGE("OHAudioRenderCallback OnRenderInterruptEvent CodecUserData is nullptr");
        return;
    }
    
    CallBackContext *callback = reinterpret_cast<CallBackContext *>(param->sampleInfo->interruptCallbackData);
    if (!callback) {
        LOGE("OHAudioRenderCallback OnRenderInterruptEvent CallBackContext is nullptr");
        return;
    }
    
    callback->hint=hint;
    callback->forceType=type;

    napi_status result = napi_acquire_threadsafe_function(param->sampleInfo->audioInterruptFn);
    if (result != napi_ok) {
        LOGE("OHAudioRenderCallback OnRenderInterruptEvent start napi_acquire_threadsafe_function failed");
        return;
    }

    result = napi_call_threadsafe_function(param->sampleInfo->audioInterruptFn, callback, napi_tsfn_nonblocking);
    if (result != napi_ok) {
        LOGE("OHAudioRenderCallback OnRenderInterruptEvent start napi_acquire_threadsafe_function failed");
        return;
    }
    return;
}

int32_t OHAudioRenderCallback::OnRenderError(OH_AudioRenderer *renderer, void *userData, OH_AudioStream_Result error) {
    LOGD("OHAudioRenderCallback OnRenderError");
    if (userData == nullptr) {
        LOGE("OHAudioRenderCallback OnRenderError userData is nullptr");
        return RESULT_CODE_ERROR;
    }
    (void)renderer;
    
    CodecUserData *param = reinterpret_cast<CodecUserData *>(userData);
    if (!param) {
        LOGE("OHAudioRenderCallback OnRenderError CodecUserData is nullptr");
        return RESULT_CODE_ERROR;
    }
    
    CallBackContext *callback = reinterpret_cast<CallBackContext *>(param->sampleInfo->audioErrorCallbackData);
    if (!callback) {
        LOGE("OHAudioRenderCallback OnRenderError CallBackContext is nullptr");
        return RESULT_CODE_ERROR;
    }

    callback->errorAudioCode = error;
    
    napi_status result = napi_acquire_threadsafe_function(param->sampleInfo->audioErrorFn);
    if (result != napi_ok) {
        LOGE("OHAudioRenderCallback OnRenderError start napi_acquire_threadsafe_function failed");
        return RESULT_CODE_ERROR;
    }

    result = napi_call_threadsafe_function(param->sampleInfo->audioErrorFn, callback, napi_tsfn_nonblocking);
    if (result != napi_ok) {
        LOGE("OHAudioRenderCallback OnRenderError start napi_acquire_threadsafe_function failed");
        return RESULT_CODE_ERROR;
    }
    return RESULT_CODE_OK;
}

void OHAudioRenderCallback::OnErrorCallback(OH_AudioRenderer *renderer, void *userData, OH_AudioStream_Result error) {
    LOGD("OHAudioRenderCallback OnRenderError");
    if (userData == nullptr) {
        LOGE("OHAudioRenderCallback OnRenderError userData is nullptr");
        return ;
    }
    (void)renderer;
    
    CodecUserData *param = reinterpret_cast<CodecUserData *>(userData);
    if (!param) {
        LOGE("OHAudioRenderCallback OnRenderError CodecUserData is nullptr");
        return;
    }
    
    CallBackContext *callback = reinterpret_cast<CallBackContext *>(param->sampleInfo->audioErrorCallbackData);
    if (!callback) {
        LOGE("OHAudioRenderCallback OnRenderError CallBackContext is nullptr");
        return;
    }

    callback->errorAudioCode = error;
    
    napi_status result = napi_acquire_threadsafe_function(param->sampleInfo->audioErrorFn);
    if (result != napi_ok) {
        LOGE("OHAudioRenderCallback OnRenderError start napi_acquire_threadsafe_function failed");
        return;
    }

    result = napi_call_threadsafe_function(param->sampleInfo->audioErrorFn, callback, napi_tsfn_nonblocking);
    if (result != napi_ok) {
        LOGE("OHAudioRenderCallback OnRenderError start napi_acquire_threadsafe_function failed");
        return;
    }
    return;
}
