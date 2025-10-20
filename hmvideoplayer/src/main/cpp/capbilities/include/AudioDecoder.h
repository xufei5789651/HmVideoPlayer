#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include "CodecUserData.h"
#include "SampleInfo.h"
#include "multimedia/player_framework/native_avcodec_audiocodec.h"
#include "multimedia/player_framework/native_avbuffer_info.h"
#include "AudioVideoDecodeCallback.h"

class AudioDecoder {
public:
    AudioDecoder() = default;
    ~AudioDecoder();

    int32_t Create(const std::string &codecMime);
    int32_t Config(const SampleInfo &sampleInfo, CodecUserData *codecUserData);
    int32_t Start();
    int32_t PushInputBuffer(CodecBufferInfo &info);
    int32_t FreeOutputBuffer(uint32_t bufferIndex, bool render);
     // 停止
    int32_t Stop();
    int32_t Release();
    
private:
    int32_t SetCallback(CodecUserData *codecUserData);
    int32_t Configure(const SampleInfo &sampleInfo);
    
    bool isAVBufferMode = false;
    OH_AVCodec *decoder;
};
#endif // AUDIODECODER_H