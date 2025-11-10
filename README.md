# HmVideoPlayer
基于HarmonyOSNext SDK 一款视频播放器

## 特性
- 视频编解码：支持H.264、H.265
- 流媒体：支持HTTP/HTTPS、HLS协议
- 支持HDR Vivid、Audio Vivid 播放

## 效果图

<img src="screenshots/Screenshot_video.png" width='320' height="640">

## 依赖方式
```
ohpm install @glodentime/hmvideoplayer
```

## 使用示例
- 基础用法
```typescript

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
}

// 监听状态变化
setVideoPlayerCallback(){
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
  })

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
  HmVideoPlayer.release();
}
```

## 接口能力
- HmVideoPlayer 方法

  | 接口 | 参数 | 返回值 | 说明 |
  | --- | --- | --- | --- |
  | initWithURL |url: string  | void |加载网络资源  |
  | initWithLocal | inputFileFd: number,inputFileOffset: number,inputFileSize: number | void | 加载本地视频 |
  |play  | void | void | 开始播放 |
  | pause | void | void | 暂停播放 |
  | resume | void | void | 恢复播放 |
  | stop| void | void | 停止播放 |
  |release  |void  | void | 释放播放器 |
  |getDuration  |void  | number | 获取媒体资源的总时长 |
  |seek  |position: bigint  | void | 跳转至指定进度 |
  |onTimeUpdate  |callback: (timestamp: number) => void | void | 获取当前播放进度时间戳 |
  |onStateChange  |callback: (state: number) => void  | void | 注册播放状态变更监听 |

## 开源协议
本项目基于 Apache License 2.0 ，在拷贝和借鉴代码时，请大家务必注明出处。
