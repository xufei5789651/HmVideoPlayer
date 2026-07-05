export class MediaInfo {
  title?: string;
  cover?: string | PixelMap;

  constructor(title: string, cover: string | PixelMap) {
    this.title = title
    this.cover = cover
  }
}