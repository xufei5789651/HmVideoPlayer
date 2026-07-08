export interface AvSessionCallback {
  play: () => void,
  pause: () => void,
  seek: (time: number) => void,
  fastForward: (time: number) => void,
  rewind: (time: number) => void,
  onNext: () => void,
  onPrevious: () => void
}