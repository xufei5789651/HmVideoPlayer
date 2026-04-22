export const initWithURL: (url: string) => any;

export const initWithLocal: (inputFileFd: number, inputFileOffset: number, inputFileSize: number) => any;

export const play: () => any;

export const pause: () => any;

export const resume: () => any;

export const stop: () => any;

export const release: () => any;

export const getDuration: () => any;

export const seek: (position: bigint) => any;

export const onTimeUpdate: (callback: (timestamp: number) => void) => any;

export const onStateChange: (callback: (state: number) => void) => any;

export const onAudioInterrupt: (callback: (forceType: number, hint: number) => void) => any;

export const onOutputDeviceChange: (callback: (deviceChange: number) => void) => any;

export const onAudioError: (callback: (errorAudioCode: number) => void) => any;

export const onCodecError: (callback: (errorAvcodecCode: number) => void) => any;

export const onCodecFormatChange: (callback: (videoWidth: number, videoHeight: number, audioSampleFormat: number,
  audioChannelCount: number, audioSampleRate: number, videoFrameRate: number) => void) => any;

export const ratePlay: (speed: number) => any;