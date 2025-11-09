import { HmVideoPlayer } from '@glodentime/hmvideoplayer';
import { display } from '@kit.ArkUI';

@Entry
@Component
struct Index {
  private display = display.getDefaultDisplaySync();
  private heightPx = (this.display.width * 1080 / 1920) + 'px';
  @State URL: string = 'https://consumer.huawei.com/content/dam/huawei-cbg-site/cn/mkt/pdp/phones/ah-pro-plus/video/kv-intro-pop.mp4';
  @State duration: number = 0;
  @State timestamp: number = 0;
  @State stateChange: number = 0;

  build() {
    Column() {
      Column() {
        Text('hmvideoplayer')
          .width('100%')
          .font({
            size: 32,
            weight: 700
          })
          .fontColor('#e5000000')
      }
      .margin({ top: 8 })
      .padding({
        left: '16vp'
      })
      .width($r('app.string.full_width'))
      .height($r('app.float.index_column_height'))

      XComponent({
        id: 'player',
        type: XComponentType.SURFACE,
        libraryname: 'hmvideoplayer'
      })
        .height(this.heightPx)
        .width('100%')
        .margin({ top: 8 })

      Column({ space: 8 }) {
        Column() {
          Button('init')
            .onClick(() => {
              HmVideoPlayer.initWithURL(this.URL);
            })
            .size({
              width: '100%',
              height: '35vp'
            })
        }
        .padding({
          left: '16vp',
          right: '16vp'
        })
        .width($r('app.string.full_width'))
        .height($r('app.float.index_column_height'))

        Column() {
          Button('play')
            .onClick(() => {
              HmVideoPlayer.play();
            })
            .size({
              width: '100%',
              height: '35vp'
            })
        }
        .padding({
          left: '16vp',
          right: '16vp'
        })
        .width($r('app.string.full_width'))
        .height($r('app.float.index_column_height'))

        Column() {
          Button('pause')
            .onClick(() => {
              HmVideoPlayer.pause();
            })
            .size({
              width: '100%',
              height: '35vp'
            })
        }
        .padding({
          left: '16vp',
          right: '16vp'
        })
        .width($r('app.string.full_width'))
        .height($r('app.float.index_column_height'))

        Column() {
          Button('resume')
            .onClick(() => {
              HmVideoPlayer.resume();
            })
            .size({
              width: '100%',
              height: '35vp'
            })
        }
        .padding({
          left: '16vp',
          right: '16vp'
        })
        .width($r('app.string.full_width'))
        .height($r('app.float.index_column_height'))

        Column() {
          Button('stop')
            .onClick(() => {
              HmVideoPlayer.stop();
            })
            .size({
              width: '100%',
              height: '35vp'
            })
        }
        .padding({
          left: '16vp',
          right: '16vp'
        })
        .width($r('app.string.full_width'))
        .height($r('app.float.index_column_height'))

        Column() {
          Button('release')
            .onClick(() => {
              HmVideoPlayer.release();
            })
            .size({
              width: '100%',
              height: '35vp'
            })
        }
        .padding({
          left: '16vp',
          right: '16vp'
        })
        .width($r('app.string.full_width'))
        .height($r('app.float.index_column_height'))

        Column() {
          Button('duration:' + this.duration)
            .onClick(() => {
              this.duration = HmVideoPlayer.getDuration();
            })
            .size({
              width: '100%',
              height: '35vp'
            })
        }
        .padding({
          left: '16vp',
          right: '16vp'
        })
        .width($r('app.string.full_width'))
        .height($r('app.float.index_column_height'))

        Column() {
          Button('timestamp:' + this.timestamp)
            .onClick(() => {
              HmVideoPlayer.onTimeUpdate((timestamp: number) => {
                this.timestamp = timestamp;
              });
            })
            .size({
              width: '100%',
              height: '35vp'
            })
        }
        .padding({
          left: '16vp',
          right: '16vp'
        })
        .width($r('app.string.full_width'))
        .height($r('app.float.index_column_height'))

        Column() {
          Button('stateChange:' + this.stateChange)
            .onClick(() => {
              HmVideoPlayer.onStateChange((state: number) => {
                this.stateChange = state;
              });
            })
            .size({
              width: '100%',
              height: '35vp'
            })
        }
        .padding({
          left: '16vp',
          right: '16vp'
        })
        .width($r('app.string.full_width'))
        .height($r('app.float.index_column_height'))

        Column() {
          Button($r('app.string.rate1'))
            .onClick(() => {
              HmVideoPlayer.ratePlay(1);
            })
            .size({
              width: '100%',
              height: '35vp'
            })
        }
        .padding({
          left: '16vp',
          right: '16vp',
        })
        .width($r('app.string.full_width'))
        .height($r('app.float.index_column_height'))

        Column() {
          Button($r('app.string.rate2'))
            .onClick(() => {
              HmVideoPlayer.ratePlay(2);
            })
            .size({
              width: '100%',
              height: '35vp'
            })
        }
        .padding({
          left: '16vp',
          right: '16vp'
        })
        .width($r('app.string.full_width'))
        .height($r('app.float.index_column_height'))

      }
      .layoutWeight(1)
      .width($r('app.string.full_width'))
      .height($r('app.string.full_width'))
      .justifyContent(FlexAlign.End)
      .padding({ bottom: '16vp' })
    }
    .height('100%')
    .width('100%')
  }
}