import player from 'libhmvideoplayer.so';

export class HmVideoPlayer {
  /**
   * 加载网络资源
   * @param url
   */
  static initWithURL(url: string) {
    player.initWithURL(url);
  }

  /**
   * 加载本地视频
   * @param inputFileFd
   * @param inputFileOffset
   * @param inputFileSize
   */
  static initWithLocal(inputFileFd: number,inputFileOffset: number,inputFileSize: number) {
    player.initWithLocal(inputFileFd,inputFileOffset,inputFileSize);
  }

  /**
   * 开始播放
   */
  static play() {
    player.play();
  }

  /**
   * 暂停播放
   */
  static pause() {
    player.pause();
  }

  /**
   * 恢复播放
   */
  static resume() {
    player.resume();
  }

  /**
   * 停止播放
   */
  static stop() {
    player.stop();
  }

  /**
   * 释放播放器
   */
  static release() {
    player.release();
  }

  /**
   * 获取媒体资源的总时长
   * @returns (单位：ms)
   */
  static getDuration(): number {
    return player.getDuration();
  }

  /**
   * 跳转至指定进度
   * @param position (单位：ms)
   */
  static seek(position: bigint) {
    player.seek(position);
  }

  /**
   * 获取当前播放进度时间戳
   * @param callback (单位：ms)
   */
  static onTimeUpdate(callback: (timestamp: number) => void) {
    player.onTimeUpdate(callback);
  }

  /**
   * 注册播放状态变更监听
   * @param callback
   */
  static onStateChange(callback: (state: number) => void) {
    player.onStateChange(callback);
  }

  /**
   * 注册音频焦点变化监听
   * @param callback
   */
  static onAudioInterrupt(callback: (forceType: number, hint: number) => void) {
    player.onAudioInterrupt(callback);
  }

  /**
   * 倍速播放
   * @param speed
   */
  static ratePlay(speed: number) {
    player.ratePlay(speed);
  }
}