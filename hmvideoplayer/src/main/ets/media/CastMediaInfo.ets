export class CastMediaInfo {
  assetId: string;
  title: string;
  mediaUri: string;
  duration: number = 0;
  startPosition: number = 0;
  cover?: string;

  constructor(assetId: string, title: string, mediaUri: string) {
    this.assetId = assetId
    this.title = title
    this.mediaUri = mediaUri
  }
}
