#ifndef HMVIDEOPLAYER_CODECUSERDATA_H
#define HMVIDEOPLAYER_CODECUSERDATA_H

#include "SampleInfo.h"
#include <condition_variable>
#include <queue>

/**
 * 编解码Buffer数据
 */
struct CodecBufferInfo {
    uint32_t bufferIndex = 0;
    uintptr_t *buffer = nullptr;
    uint8_t *bufferAddr = nullptr;
    // pts:此缓冲区的显示时间戳(以微妙为单位)
    // size:缓冲区包含的数据的大学(以字节为单位)
    // offset:此缓冲区中有效数据的起始偏移量
    // flags:此缓冲区具有的标志，AVCODEC_BUFFER_FLAGS_NONE：表示为普通帧
    OH_AVCodecBufferAttr attr = {0, 0, 0, AVCODEC_BUFFER_FLAGS_NONE};

    explicit CodecBufferInfo(uint8_t *addr) : bufferAddr(addr){};
    
    CodecBufferInfo(uint8_t *addr, int32_t bufferSize)
        : bufferAddr(addr), attr({0, bufferSize, 0, AVCODEC_BUFFER_FLAGS_NONE}){};

    CodecBufferInfo(uint32_t argBufferIndex, OH_AVBuffer *argBuffer)
        : bufferIndex(argBufferIndex), buffer(reinterpret_cast<uintptr_t *>(argBuffer)) {
        OH_AVBuffer_GetBufferAttr(argBuffer, &attr);
    };
};

/**
 * 音视频解码用户数据结构体
 */
struct CodecUserData {
public:
    SampleInfo *sampleInfo = nullptr;
    // 输入队列
    uint32_t inputFrameCount = 0;
    std::mutex inputMutex;
    std::condition_variable inputCond;
    std::queue<CodecBufferInfo> inputBufferInfoQueue;
    // 输出队列
    uint32_t outputFrameCount = 0;
    std::mutex outputMutex;
    std::condition_variable outputCond;
    std::queue<CodecBufferInfo> outputBufferInfoQueue;
    // 渲染队列
    std::queue<unsigned char> renderQueue;
    std::mutex renderMutex;
    std::condition_variable renderCond;
    // 速率
    int64_t speed = 1.0f;
    int64_t frameWrittenForSpeed = 0;
    int64_t endPosAudioBufferPts = 0;
    int64_t currentPosAudioBufferPts = 0;
};
#endif //HMVIDEOPLAYER_CODECUSERDATA_H
