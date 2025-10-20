import { display } from '@kit.ArkUI';
import player from 'libhmvideoplayer.so';
import { CommonConstants as Const } from '../common/CommonConstants';

const TAG: string = Const.INDEX_TAG;

@Component
export struct TestPage {
  @State duration: number = 0;
  @State timestamp: number = 0;
  @State stateChange: number = 0;
  @State URL: string = 'https://consumer.huawei.com/content/dam/huawei-cbg-site/cn/mkt/pdp/phones/ah-pro-plus/video/kv-intro-pop.mp4';
  private display = display.getDefaultDisplaySync();
  private heightPx = (this.display.width * Const.DEFAULT_HEIGHT / Const.DEFAULT_WIDTH) + Const.PX;

  ratePlay(speed: number) {
    player.ratePlay(speed);
  }

  build() {
    Column() {
      Column() {
        Text('hmvideoplayer')
          .width(Const.FULL_SIZE)
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
        .width(Const.FULL_SIZE)
        .margin({ top: 8 })

      Column({ space: 8 }) {
        Column() {
          Button('init')
            .onClick(() => {
              player.init(this.URL);
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
              player.play();
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
              player.pause();
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
              player.resume();
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
              player.stop();
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
              player.release();
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
              this.duration = player.getDuration();
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
              player.onTimeUpdate((timestamp: number) => {
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
              player.onStateChange((state: number) => {
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
              this.ratePlay(1);
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
              this.ratePlay(2);
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
    .height($r('app.string.full_width'))
    .backgroundColor('#F1F3F5')
    .expandSafeArea([SafeAreaType.SYSTEM], [SafeAreaEdge.TOP, SafeAreaEdge.BOTTOM])
  }
}
