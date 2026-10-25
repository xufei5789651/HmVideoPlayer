import { AVCastPicker, AVCastPickerState } from '@kit.AVSessionKit';

/**
 * 投播组件（封装系统 AVCastPicker）：
 * 不传 customPickBuilder 时为系统默认样式；传入 customPickBuilder 时为自定义样式。
 */
@Component
export struct HmCastPicker {
  normalColor?: Color | number | string;
  activeColor?: Color | number | string;
  sessionType: string = 'video';
  onStateChange?: (state: AVCastPickerState) => void;
  customPickBuilder?: CustomBuilder;

  build() {
    AVCastPicker({
      normalColor: this.normalColor,
      activeColor: this.activeColor,
      sessionType: this.sessionType,
      onStateChange: this.onStateChange,
      customPicker: this.customPickBuilder
    })
  }
}
