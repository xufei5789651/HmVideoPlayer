#ifndef HMVIDEOPLAYER_AUDIOVIDEODECODECALLBACK_H
#define HMVIDEOPLAYER_AUDIOVIDEODECODECALLBACK_H

#include <multimedia/player_framework/native_avcodec_base.h>

class AudioVideoDecodeCallback {
public:
    // 音视频编解码(OH_AVCodecCallback)运行错误
    static void OnCodecError(OH_AVCodec *codec, int32_t errorCode, void *userData);
    // 音视频编解码(OH_AVCodecCallback)码流信息变化，如：声道变化等
    static void OnCodecFormatChange(OH_AVCodec *codec, OH_AVFormat *format, void *userData);
    // 音视频编解码(OH_AVCodecCallback)运行过程中需要新的输入数据，即解码器已准备好，可以输入数据
    static void OnNeedInputBuffer(OH_AVCodec *codec, uint32_t index, OH_AVBuffer *buffer, void *userData);
    // 音视频编解码(OH_AVCodecCallback)运行过程中产生了新的输出数据，即解码完成
    static void OnNewOutputBuffer(OH_AVCodec *codec, uint32_t index, OH_AVBuffer *buffer, void *userData);
};

#endif //HMVIDEOPLAYER_AUDIOVIDEODECODECALLBACK_H
