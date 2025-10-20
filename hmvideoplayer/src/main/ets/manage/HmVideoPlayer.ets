import player from 'libhmvideoplayer.so';

export function init(url: string) {
  player.init(url);
}

export function play() {
  player.play();
}

export function pause() {
  player.pause();
}

export function resume() {
  player.resume();
}

export function stop() {
  player.stop();
}

export function release() {
  player.release();
}

export function getDuration() :number {
  return player.getDuration();
}

export function seek(position: bigint) {
  player.seek(position);
}

export function onTimeUpdate(callback: (timestamp: number) => void) {
  player.onTimeUpdate(callback);
}

export function onStateChange(callback: (state: number) => void) {
  player.onStateChange(callback);
}

export function ratePlay(speed: number) {
  player.ratePlay(speed);
}