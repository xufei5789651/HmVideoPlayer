#ifndef DEMUXER_H
#define DEMUXER_H
#include "utils/LogUtils.h"
#include "utils/ResultCode.h"
#include <bits/alltypes.h>
#include "napi/native_api.h"
#include "multimedia/player_framework/native_avdemuxer.h"
#include "SampleInfo.h"
#include "dfx/error/AVCodecSampleError.h"
#include "AVCodecSampleLog.h"

class Demuxer {
public:
    Demuxer() = default;
    ~Demuxer();
    int32_t Create(SampleInfo &sampleInfo);
    int32_t ReadSample(int32_t trackId, OH_AVBuffer *buffer, OH_AVCodecBufferAttr &attr);
    int32_t Release();
     // 跳转到播放位置
    int32_t Seek(int64_t position);
    // 获取时长，单位：微妙
    int64_t GetDuration();
    int32_t GetVideoTrackId();
    int32_t GetAudioTrackId();
    
private:
    int32_t GetTrackInfo(std::shared_ptr<OH_AVFormat> sourceFormat, SampleInfo &info);
    OH_AVSource *source;
    OH_AVDemuxer *demuxer;
    // 视频轨道index
    int32_t videoTrackId;
    int32_t audioTrackId;
    SampleInfo info;
};

#endif // DEMUXER_H