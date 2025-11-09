import { HmVideoPlayer, StateCode } from '@glodentime/hmvideoplayer';
import { display } from '@kit.ArkUI';
import { secondToTime } from '../utils/Utils';

const WIDTH_PX: number = 1920;
const HEIGHT_PX: number = 1080;
const SECOND_TO_MS: number = 1000;

@Entry
@ComponentV2
struct MainPage {
  display = display.getDefaultDisplaySync();
  xComponentController: XComponentController = new XComponentController();
  heightPx = (this.display.width * HEIGHT_PX / WIDTH_PX) + 'px';
  URL: string = 'https://consumer.huawei.com/content/dam/huawei-cbg-site/cn/mkt/pdp/phones/ah-pro-plus/video/kv-intro-pop.mp4';
  @Local isPlay: boolean = false;
  @Local duration: number = 0;
  @Local totalTimeStr: string = '00:00';
  @Local currentTimestamp: number = 0;
  @Local currentTimestampStr: string = '00:00';

  onPageShow(): void {
    this.setVideoPlayerCallback();
  }

  setVideoPlayerCallback() {
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

  private setVideoDuration() {
    this.duration = Math.floor(HmVideoPlayer.getDuration() / SECOND_TO_MS);
    this.totalTimeStr = secondToTime(Math.floor(HmVideoPlayer.getDuration() / SECOND_TO_MS));
  }

  private setUpdateTimeCallback() {
    HmVideoPlayer.onTimeUpdate((timestamp: number) => {
      this.currentTimestamp = Math.floor(timestamp / SECOND_TO_MS);
      this.currentTimestampStr = secondToTime(Math.floor(timestamp / SECOND_TO_MS));
    });
  }

  onPageHide(): void {
    HmVideoPlayer.stop();
    HmVideoPlayer.release();
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