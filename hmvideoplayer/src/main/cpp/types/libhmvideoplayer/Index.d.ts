export const init: (url: string) => any;

export const play: () => any;

export const pause: () => any;

export const resume: () => any;

export const stop: () => any;

export const release: () => any;

export const getDuration: () => any;

export const seek: (position: bigint) => any;

export const onTimeUpdate: (callback: (timestamp: number) => void) => any;

export const onStateChange: (callback: (state: number) => void) => any;

export const ratePlay: (speed: number) => any;