#include "OHAudioRenderCallback.h"
#include "CodecUserData.h"
#include "SampleInfo.h"
#include "utils/LogUtils.h"

constexpr int32_t BYTES_PER_SAMPLE_2 = 2;

// Custom write data function
int32_t OHAudioRenderCallback::OnRenderWriteData(OH_AudioRenderer *renderer, void *userData, void *buffer, int32_t length) {
    (void)renderer;
    (void)length;
    CodecUserData *codecUserData = static_cast<CodecUserData *>(userData);

    // Write the data to be played to the buffer by length
    uint8_t *dest = (uint8_t *)buffer;
    size_t index = 0;
    std::unique_lock<std::mutex> lock(codecUserData->outputMutex);
    // Retrieve the length of the data to be played from the queue
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
// Customize the audio stream event function
int32_t OHAudioRenderCallback::OnRenderStreamEvent(OH_AudioRenderer *renderer, void *userData, OH_AudioStream_Event event) {
    (void)renderer;
    (void)userData;
    (void)event;
    // Update the player status and interface based on the audio stream event information represented by the event
    return 0;
}
// Customize the audio interrupt event function
int32_t OHAudioRenderCallback::OnRenderInterruptEvent(OH_AudioRenderer *renderer, void *userData,
                                               OH_AudioInterrupt_ForceType type, OH_AudioInterrupt_Hint hint) {
    (void)renderer;
    (void)userData;
    (void)type;
    (void)hint;
    // Update the player status and interface based on the audio interrupt information indicated by type and hint
    return 0;
}
// Custom exception callback functions
int32_t OHAudioRenderCallback::OnRenderError(OH_AudioRenderer *renderer, void *userData, OH_AudioStream_Result error) {
    (void)renderer;
    (void)userData;
    (void)error;
    LOGE("OnRenderError");
    // Handle the audio exception information based on the error message
    return 0;
}