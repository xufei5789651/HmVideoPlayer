import { AvSessionCallback, AvSessionManager, HmVideoPlayer, MediaInfo, StateCode } from '@glodentime/hmvideoplayer';
import { display } from '@kit.ArkUI';
import { secondToTime } from '../utils/Utils';
import { audio } from '@kit.AudioKit';
import { common, wantAgent } from '@kit.AbilityKit';

const WIDTH_PX: number = 1920;
const HEIGHT_PX: number = 1080;
const SECOND_TO_MS: number = 1000;

const TAG: string = 'MainPage';

@Entry
@ComponentV2
struct MainPage {
  avSessionManager?: AvSessionManager;
  display = display.getDefaultDisplaySync();
  xComponentController: XComponentController = new XComponentController();
  heightPx = (this.display.width * HEIGHT_PX / WIDTH_PX) + 'px';
  context = this.getUIContext().getHostContext() as common.UIAbilityContext;
  URL: string =
    'https://consumer.huawei.com/content/dam/huawei-cbg-site/cn/mkt/pdp/phones/ah-pro-plus/video/kv-intro-pop.mp4';
  @Local isPlay: boolean = false;
  @Local duration: number = 0;
  @Local totalTimeStr: string = '00:00';
  @Local currentTimestamp: number = 0;
  @Local currentTimestampStr: string = '00:00';
  wantAgentInfo: wantAgent.WantAgentInfo = {
    wants: [{
      bundleName: this.context.abilityInfo.bundleName,
      abilityName: this.context.abilityInfo.name
    }],
    requestCode: 0x112,
    actionType: wantAgent.OperationType.START_ABILITIES,
    wantAgentFlags: [wantAgent.WantAgentFlags.UPDATE_PRESENT_FLAG]
  }
  avSessionCallback: AvSessionCallback = {
    onNext: () => {
      // todo 下一个视频
    },
    onPrevious: () => {
      // todo 上一个视频
    },
    play: (): void => {
      HmVideoPlayer.resume()
    },
    pause: (): void => {
      HmVideoPlayer.pause()
    },
    seek: (time: number): void => {
      HmVideoPlayer.seek(BigInt(time))
    },
    fastForward: (time: number): void => {
      HmVideoPlayer.seek(BigInt(this.currentTimestamp + time))
    },
    rewind: (time: number): void => {
      HmVideoPlayer.seek(BigInt(this.currentTimestamp - time))
    }
  };

  onPageShow(): void {
    this.setAvSession();
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
      console.log(TAG, `errorAudioCode:${errorAudioCode}`);
    });

    // 注册输出设备变化监听
    HmVideoPlayer.onOutputDeviceChange((deviceChange: number) => {
      if (deviceChange === audio.AudioStreamDeviceChangeReason.REASON_OLD_DEVICE_UNAVAILABLE) {
        HmVideoPlayer.pause();
      } else if (deviceChange === audio.AudioStreamDeviceChangeReason.REASON_NEW_DEVICE_AVAILABLE) {
        HmVideoPlayer.resume();
      }
    });

    // 注册音视频编解码运行错误监听
    HmVideoPlayer.onCodecError((errorAvcodecCode: number) => {
      console.log(TAG, `errorAvcodecCode:${errorAvcodecCode}`);
    });

    // 注册码流信息变化，如：声道变化等
    HmVideoPlayer.onCodecFormatChange((videoWidth: number, videoHeight: number, audioSampleFormat: number,
      audioChannelCount: number, audioSampleRate: number, videoFrameRate: number) => {
      console.log(TAG, `videoWidth:${videoWidth},videoHeight:${videoHeight},audioSampleFormat:${audioSampleFormat},
      videoFrameRate:${videoFrameRate},audioChannelCount:${audioChannelCount},audioSampleRate:${audioSampleRate}`);
    });
  }

  private setVideoDuration() {
    this.duration = Math.floor(HmVideoPlayer.getDuration() / SECOND_TO_MS);
    this.totalTimeStr = secondToTime(Math.floor(HmVideoPlayer.getDuration() / SECOND_TO_MS));
  }

  private setUpdateTimeCallback() {
    HmVideoPlayer.onTimeUpdate((timestamp: number) => {
      this.currentTimestamp = Math.floor(timestamp / SECOND_TO_MS);
      this.currentTimestampStr = secondToTime(Math.floor(timestamp / SECOND_TO_MS));
      if (this.avSessionManager) {
        this.avSessionManager.setAvSessionSeekChanged(timestamp);
      }
    });
  }

  onPageHide(): void {
    HmVideoPlayer.stop();
    HmVideoPlayer.setBackgroundPlayEnable(this.context, this.wantAgentInfo, false);
    HmVideoPlayer.release();
    // avSession 相关方法
    if (this.avSessionManager) {
      this.avSessionManager.removeAvSessionCallback(this.avSessionCallback);
      this.avSessionManager.release();
    }
  }

  build() {
    Column() {
      Stack({ alignContent: Alignment.Bottom }) {
        XComponent({
          id: 'player',
          type: XComponentType.SURFACE,
          libraryname: 'hmvideoplayer',
          controller: this.xComponentController
        })
          .onLoad(() => {
            console.log(TAG, '--->onLoad');
            this.xComponentController.setXComponentSurfaceRect({ surfaceWidth: WIDTH_PX, surfaceHeight: HEIGHT_PX });
            HmVideoPlayer.initWithURL(this.URL);
          })
          .width('100%')
          .height('100%')

        Row() {
          Image(this.isPlay ? $r('app.media.pause') : $r('app.media.play_fill'))
            .width(24)
            .height(24)
            .margin({ left: 8 })
            .onClick(() => {
              if (this.isPlay) {
                HmVideoPlayer.pause()
              } else {
                HmVideoPlayer.resume()
              }
            })

          Text(this.currentTimestampStr)
            .fontSize(9)
            .fontColor(Color.White)
            .width(38)
            .margin({ left: 8, right: 1 })

          Slider({
            value: this.currentTimestamp,
            min: 0,
            step: 1,
            max: this.duration
          })
            .layoutWeight(1)
            .selectedColor(Color.Blue)
            .trackColor(Color.Gray)
            .onChange((value: number, mode: SliderChangeMode) => {
              if (mode === SliderChangeMode.Begin || mode === SliderChangeMode.End) {
                this.currentTimestamp = value;
                HmVideoPlayer.seek(BigInt(value * SECOND_TO_MS));
              }
            })

          Text(this.totalTimeStr)
            .fontSize(9)
            .fontColor(Color.White)
            .width(38)
            .margin({ left: 1, right: 3 })
        }
        .width('100%')
        .height(40)
        .alignItems(VerticalAlign.Center)
        .backgroundColor('#80000000')
      }
      .width('100%')
      .background(Color.Black)
      .height(this.heightPx)
    }
    .width('100%')
    .height('100%')
  }
}