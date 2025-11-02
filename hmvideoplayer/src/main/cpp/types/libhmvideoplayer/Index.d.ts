export const initWithURL: (url: string) => any;

export const initWithLocal: (inputFileFd: number,inputFileOffset: number,inputFileSize: number) => any;

export const play: () => any;

export const pause: () => any;

export const resume: () => any;

export const stop: () => any;

export const release: () => any;

export const getDuration: () => any;

export const seek: (position: bigint) => any;

export const onTimeUpdate: (callback: (timestamp: number) => void) => any;

export const onStateChange: (callback: (state: number) => void) => any;

export const onAudioInterrupt: (callback: (forceType: number,hint:number) => void) => any;

export const ratePlay: (speed: number) => any;