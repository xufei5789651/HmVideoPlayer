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
        this.avSessionManager.setAvSessionPlaybackState(state);
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
    const seconds = Math.floor(timestamp / SECOND_TO_MS);
    // 整秒变化时同步一次播控中心进度，避免高频推送
    if (seconds !== this.currentTimestamp) {
      this.currentTimestamp = seconds;
      if (this.avSessionManager) {
        this.avSessionManager.setAvSessionSeekChanged(seconds * SECOND_TO_MS);
      }
    }
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
  | setAvSessionPlaybackState | state: number | void | 设置播控中心的状态    |
  | setAvSessionSeekChanged | currentTimestamp: number | void | 设置播控中心的进度条   |

- AvSessionCallback 播控中心事件回调

  | 属性       | 类型       | 说明    |
    | ---------- | ---------- |-------|
  | onNext     | () => void | 播放上一首 |
  | onPrevious | () => void | 播放下一首 |
  | play | () => void | 播放    |
  | pause | () => void | 暂停    |
  | seek | () => void | 同步进度  |
  | fastForward | () => void | 前进    |
  | rewind | () => void | 后退    |

## 音视频投播

支持通过投播组件选择投播设备（默认样式和自定义样式），将音视频投播到远端设备播放，并提供远端播放控制能力（`AVCastPicker` / `AVCastController` 封装）。

### 使用示例

```
import { AVCastManager, CastDeviceCallback, CastMediaInfo, HmCastPicker } from '@glodentime/hmvideoplayer';

// 1. 注册投播设备监听（bindAvSession 之后调用），远端设备连接成功后自动获取投播控制器
this.castManager = new AVCastManager();
let castDeviceCallback: CastDeviceCallback = {
  onConnected: (): void => {
    // 远端设备连接成功：暂停本机播放，携带当前进度发起投播
    HmVideoPlayer.pause();
    let castMediaInfo = new CastMediaInfo('HmVideo', 'HmVideo', this.URL);
    castMediaInfo.duration = HmVideoPlayer.getDuration();
    castMediaInfo.startPosition = this.currentTimestamp * 1000;
    this.castManager.startCast(castMediaInfo);
  },
  onDisconnected: (): void => {
    // 远端设备断开：回到本机继续播放
    HmVideoPlayer.resume();
  }
};
this.castManager.setCastDeviceListener(this.avSessionManager, castDeviceCallback);

// 2. 投播中控制远端播放
this.castManager.play();               // 远端播放
this.castManager.pause();              // 远端暂停
this.castManager.seek(position);       // 远端进度跳转（ms）
this.castManager.setVolume(1);         // 远端音量设置

// 3. 监听远端播放状态与进度
this.castManager.onCastPlaybackStateChange((state: AVPlaybackState) => {});
this.castManager.onCastSeekDone((positionMs: number) => {});

// 4. 结束投播
this.castManager.stopCast(this.avSessionManager);

// 5. 投播入口组件（默认样式：系统投播图标）
HmCastPicker({ sessionType: 'video' })

// 6. 投播入口组件（自定义样式）
@Builder
castButtonBuilder() {
  Text('投播')
}

HmCastPicker({
  sessionType: 'video',
  customPickBuilder: (): void => {
    this.castButtonBuilder()
  }
})
```

### 接口能力

- AVCastManager 投播管理

  | 接口 | 参数 | 返回值 | 说明 |
  | --- | --- | --- | --- |
  | setCastDeviceListener | avSessionManager: AvSessionManager, callback: CastDeviceCallback | void | 注册投播设备监听，远端连接成功后自动获取投播控制器 |
  | startCast | castMediaInfo: CastMediaInfo | Promise<boolean> | 开始投播（prepare + start） |
  | stopCast | avSessionManager: AvSessionManager | Promise<void> | 结束投播 |
  | play | void | Promise<void> | 远端播放 |
  | pause | void | Promise<void> | 远端暂停 |
  | seek | positionMs: number | Promise<void> | 远端进度跳转 |
  | setVolume | volume: number | Promise<void> | 远端音量设置 |
  | onCastPlaybackStateChange | callback: (state: AVPlaybackState) => void | void | 监听远端播放状态变化 |
  | onCastSeekDone | callback: (positionMs: number) => void | void | 监听远端进度调节完成 |
  | getCastPlaybackState | void | Promise<AVPlaybackState \| undefined> | 获取远端当前播放状态 |
  | release | void | void | 释放投播资源 |

- HmCastPicker 投播组件

  | 属性 | 类型 | 说明 |
  | --- | --- | --- |
  | normalColor | Color \| number \| string | 正常状态颜色 |
  | activeColor | Color \| number \| string | 设备连接成功状态颜色 |
  | sessionType | string | 会话类型，默认 'video' |
  | onStateChange | (state: AVCastPickerState) => void | 投播面板显示/消失回调 |
  | customPickBuilder | CustomBuilder | 自定义样式；不传为系统默认样式 |

- CastMediaInfo 投播媒体信息

  | 属性 | 类型 | 说明 |
  | --- | --- | --- |
  | assetId | string | 媒体唯一标识 |
  | title | string | 标题 |
  | mediaUri | string | 网络资源地址 |
  | duration | number | 时长（ms） |
  | startPosition | number | 起播/续播进度（ms） |
  | cover | string | 封面地址 |

- CastDeviceCallback 投播设备事件回调

  | 属性 | 类型 | 说明 |
  | --- | --- | --- |
  | onConnected | () => void | 远端投播设备连接成功 |
  | onDisconnected | () => void | 远端投播设备断开 |

## 开源协议
本项目基于 Apache License 2.0 ，在拷贝和借鉴代码时，请大家务必注明出处。
