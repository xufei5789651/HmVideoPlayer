# HmVideoPlayer
基于HarmonyOSNext SDK 一款视频播放器

## 特性
- 视频编解码：支持H.264、H.265
- 流媒体：支持HTTP/HTTPS、HLS协议
- 支持HDR Vivid、Audio Vivid 播放
- 支持倍速播放
- 支持绑定播控中心
- 支持后台播放控制

## 效果图

<img src="../screenshots/Screenshot_video.png" width='320' height="640">

<img src="../screenshots/Screenshot_avSession.png" width='320' height="640">

## 依赖方式
```
ohpm install @glodentime/hmvideoplayer
```

## 使用示例

```

XComponent({
  id: 'player',
  type: XComponentType.SURFACE,
  libraryname: 'hmvideoplayer',
  controller: this.xComponentController
})
  .onLoad(() => {
    this.xComponentController.setXComponentSurfaceRect({ surfaceWidth: WIDTH_PX, surfaceHeight: HEIGHT_PX });
    HmVideoPlayer.initWithURL(this.URL);
  })
  .width('100%')
  .height('100%')


onPageShow():void {
  this.setVideoPlayerCallback();
  this.setVideoPlayerCallback();
}

setAvSession() {
    // 设置avSession 相关方法
    this.avSessionManager = new AvSessionManager();
    let info: MediaInfo = new MediaInfo('wild fold', 'https://so1.360tres.com/dr/220__/t011e1ce4509d68021b.jpg');
    this.avSessionManager.setMediaSource(info);

    this.avSessionManager.bindAvSession(this.context, 'HmVideo', 'video', this.wantAgentInfo).then(() => {
      HmVideoPlayer.setBackgroundPlayEnable(this.context, this.wantAgentInfo, true)
    });

    this.avSessionManager.addAvSessionCallback(this.avSessionCallback);
  }

setVideoPlayerCallback() {
    // 注册播放状态变更监听
    HmVideoPlayer.onStateChange((state: number) => {
      switch (state) {
        case StateCode.PREPARED:
          this.isPlay = false;
          this.setVideoDuration();
          HmVideoPlayer.play();
          break;
        case StateCode.PLAYING:
          this.isPlay = true;
          this.setUpdateTimeCallback();
          break;
        case StateCode.PAUSED:
          this.isPlay = false;
          break;
        case StateCode.STOPPED:
          this.isPlay = false;
          break;
        case StateCode.COMPLETED:
          this.isPlay = false;
          break;
        case StateCode.RELEASE:
          this.isPlay = false;
          break;
      }
      
      if (this.avSessionManager) {
        this.avSessionManager.setAvSessionPlaybackState(this.currentTimestamp* SECOND_TO_MS, state);
      }
    });

    // 注册音频焦点变化监听
    HmVideoPlayer.onAudioInterrupt((forceType: number, hint: number) => {
      if (forceType === audio.InterruptForceType.INTERRUPT_FORCE) {
        switch (hint) {
          case audio.InterruptHint.INTERRUPT_HINT_PAUSE:
            HmVideoPlayer.pause();
            break;
          case audio.InterruptHint.INTERRUPT_HINT_STOP:
            HmVideoPlayer.pause();
            break;
        }
      } else if (forceType === audio.InterruptForceType.INTERRUPT_SHARE) {
        switch (hint) {
          case audio.InterruptHint.INTERRUPT_HINT_RESUME:
            HmVideoPlayer.resume();
            break;
        }
      }
    });

    // 注册音频错误监听
    HmVideoPlayer.onAudioError((errorAudioCode: number) => {
      console.log(TAG,`errorAudioCode:${errorAudioCode}`);
    });
    
    // 注册输出设备变化监听
    HmVideoPlayer.onOutputDeviceChange((deviceChange: number) => {
      if (deviceChange === audio.AudioStreamDeviceChangeReason.REASON_OLD_DEVICE_UNAVAILABLE) {
        HmVideoPlayer.pause();
      } else if (deviceChange === audio.AudioStreamDeviceChangeReason.REASON_NEW_DEVICE_AVAILABLE) {
        HmVideoPlayer.resume();
      }
    })
    
    // 注册音视频编解码运行错误监听
    HmVideoPlayer.onCodecError((errorAvcodecCode: number) => {
      console.log(TAG,`errorAvcodecCode:${errorAvcodecCode}`);
    });

    // 注册码流信息变化，如：声道变化等
    HmVideoPlayer.onCodecFormatChange((videoWidth: number, videoHeight: number, audioSampleFormat: number,
      audioChannelCount: number, audioSampleRate: number, videoFrameRate: number) => {
      console.log(TAG,`videoWidth:${videoWidth},videoHeight:${videoHeight},audioSampleFormat:${audioSampleFormat},
      videoFrameRate:${videoFrameRate},audioChannelCount:${audioChannelCount},audioSampleRate:${audioSampleRate}`);
    });
}

// 获取视频时长
private setVideoDuration()
{
  this.duration = Math.floor(HmVideoPlayer.getDuration() / SECOND_TO_MS);
}

// 监听时间戳变化
private setUpdateTimeCallback()
{
  HmVideoPlayer.onTimeUpdate((timestamp: number) => {
    this.currentTimestamp = Math.floor(timestamp / SECOND_TO_MS);
  });
}

onPageHide():void {
  HmVideoPlayer.stop();
  HmVideoPlayer.setBackgroundPlayEnable(this.context, this.wantAgentInfo, false);
  HmVideoPlayer.release();
  // avSession 相关方法
  if (this.avSessionManager) {
    this.avSessionManager.removeAvSessionCallback(this.avSessionCallback);
    this.avSessionManager.release();
  }
}
```

## 接口能力
- HmVideoPlayer 方法

  | 接口                      | 参数                                                                                                                                                                | 返回值 | 说明               |
  |-------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------| --- |------------------|
  | initWithURL             | url: string                                                                                                                                                       | void | 加载网络资源           |
  | initWithLocal           | inputFileFd: number,inputFileOffset: number,inputFileSize: number                                                                                                 | void | 加载本地视频           |
  | play                    | void                                                                                                                                                              | void | 开始播放             |
  | pause                   | void                                                                                                                                                              | void | 暂停播放             |
  | resume                  | void                                                                                                                                                              | void | 恢复播放             |
  | stop                    | void                                                                                                                                                              | void | 停止播放             |
  | release                 | void                                                                                                                                                              | void | 释放播放器            |
  | getDuration             | void                                                                                                                                                              | number | 获取媒体资源的总时长       |
  | seek                    | position: bigint                                                                                                                                                  | void | 跳转至指定进度          |
  | onTimeUpdate            | callback: (timestamp: number) => void                                                                                                                             | void | 获取当前播放进度时间戳      |
  | onStateChange           | callback: (state: number) => void                                                                                                                                 | void | 注册播放状态变更监听       |
  | onAudioInterrupt        | callback: (forceType: number, hint: number) => void                                                                                                               | void | 注册音频焦点变化监听       |
  | onOutputDeviceChange    | callback: (deviceChange: number) => void                                                                                                                          | void | 注册输出设备变化监听       |
  | onAudioError            | callback: (errorAudioCode: number) => void                                                                                                                        | void | 注册音频错误监听         |
  | onCodecFormatChange     | callback: (videoWidth: number, videoHeight: number, audioSampleFormat: number,audioChannelCount: number, audioSampleRate: number, videoFrameRate: number) => void | void | 注册码流信息变化，如：声道变化等 |
  | onCodecError            | callback: (errorAvcodecCode: number) => void                                                                                                                      | void | 注册音视频编解码运行错误监听   |
  | ratePlay                | speed: number                                                                                                                                                     |void | 倍速播放             |
  | setBackgroundPlayEnable | backgroundPlay: boolean                                                                                                                                           | void | 设置是否开启后台长时播放     |

- AvSessionManager 播控中心

  | 接口                   | 参数 | 返回值 | 说明           |
    |----------------------| --- | --- |--------------|
  | bindAvSession        |context:BaseContext, sessioname:string, type:AVSessionType, agentInfo:WantAgentInfo  | void | 绑定播控中心       |
  | addAvSessionCallback | callback: AvSessionCallback | void | 添加播控中心操作事件监听 |
  | removeAvSessionCallback | callback: AvSessionCallback | void | 移除播控中心操作事件监听 |
  | setAvSessionPlaybackState | currentTimestamp: number,state: number | void | 设置播控中心的状态    |
  | setAvSessionSeekChanged | currentTimestamp: number | void | 设置播控中心的进度条   |

- AvSessionCallback 播控中心事件回调

  | 属性       | 类型       | 说明    |
    | ---------- | ---------- |-------|
  | onNext     | () => void | 播放上一首 |
  | onPrevious | () => void | 播放下一首 |
  | onPrevious | () => void | 播放下一首 |
  | play | () => void | 播放    |
  | pause | () => void | 暂停    |
  | seek | () => void | 同步进度  |
  | fastForward | () => void | 同步进度  |
  | rewind | () => void | 同步进度  |

## 开源协议
本项目基于 Apache License 2.0 ，在拷贝和借鉴代码时，请大家务必注明出处。
