import { avSession } from "@kit.AVSessionKit"
import { StateCode } from "../common/StateCode";
import { MediaInfo } from "../media/MediaInfo"
import { HmVideoPlayer } from "./HmVideoPlayer"
import { common, wantAgent } from "@kit.AbilityKit";
import { AvSessionCallback } from "../media/AvSessionCallback";
import { JSON } from "@kit.ArkTS";
import { BusinessError } from '@kit.BasicServicesKit';

const TAG: string = 'AvSessionManager';

export class AvSessionManager {
  private avSession?: avSession.AVSession = undefined;
  private mediaInfo?: MediaInfo = undefined;
  private currentTimestamp: number = 0;
  private callbacks: Set<AvSessionCallback> = new Set();

  setAvSessionPlaybackState(currentTimestamp: number,state: number){
    this.currentTimestamp = currentTimestamp;
    if (this.avSession) {
      let avPlaybackState = this.setPlaybackState(state)
      this.avSession.setAVPlaybackState(avPlaybackState)
    }
  }


  setAvSessionSeekChanged(currentTimestamp: number){
    this.currentTimestamp = currentTimestamp;
    if (this.avSession) {
      this.avSession.setAVPlaybackState({
        state: avSession.PlaybackState.PLAYBACK_STATE_PLAY,
        position: {
          elapsedTime: currentTimestamp,
          updateTime: new Date().getTime()
        },
        duration: HmVideoPlayer.getDuration()
      })
    }
  }


  async bindAvSession(context: common.BaseContext, sessionName: string, type: avSession.AVSessionType,
    agentInfo: wantAgent.WantAgentInfo) {
    try {
      await this.create(context, sessionName, type)
      await this.setLaunchInfo(agentInfo)
      await this.bind()
    } catch (e) {
      console.error(TAG, "bindAvSession error: " + JSON.stringify(e))
    }
  }

  private async create(context: common.BaseContext, sessionName: string, type: avSession.AVSessionType) {
    console.info(TAG, "create session name= " + sessionName + ", type= " + type)
    this.avSession = await avSession.createAVSession(context, sessionName, type)
    console.info(TAG, "create sessionId= " + this.avSession.sessionId)
  }

  private async bind() {
    if (this.avSession) {
      console.info(TAG, "bind session")
      this.setAvSessionListener()
      try {
        await this.avSession.activate()
      } catch (error) {
        let err= error as BusinessError
        console.error(TAG,`err code: ${err.code},message:${err.message}`)
      }
    }
  }

  private async setLaunchInfo(agentInfo: wantAgent.WantAgentInfo) {
    if (this.avSession) {
      console.info(TAG, "setup launch app= " + JSON.stringify(agentInfo))
      let agent = await wantAgent.getWantAgent(agentInfo)
      await this.avSession.setLaunchAbility(agent)
    }
  }

  setMediaSource(mediaInfo: MediaInfo) {
    this.mediaInfo = mediaInfo
  }

  private setPlaybackState(state: number): avSession.AVPlaybackState {
    console.log(TAG,`setPlaybackState state: ${state}`);
    let avState = avSession.PlaybackState.PLAYBACK_STATE_INITIAL
    switch (state) {
      case StateCode.PREPARED:
        this.setMediaInfo();
        avState = avSession.PlaybackState.PLAYBACK_STATE_PREPARE;
        break
      case StateCode.PLAYING:
        avState = avSession.PlaybackState.PLAYBACK_STATE_PLAY;
        break
      case StateCode.PAUSED:
        avState = avSession.PlaybackState.PLAYBACK_STATE_PAUSE;
        break
      case StateCode.STOPPED:
        avState = avSession.PlaybackState.PLAYBACK_STATE_STOP;
        break
      case StateCode.COMPLETED:
        avState = avSession.PlaybackState.PLAYBACK_STATE_COMPLETED;
        break
      case StateCode.RELEASE:
        avState = avSession.PlaybackState.PLAYBACK_STATE_RELEASED;
        break
      case StateCode.ERROR:
        avState = avSession.PlaybackState.PLAYBACK_STATE_ERROR;
        break
    }
    let result: avSession.AVPlaybackState = {
      state: avState,
      position: {
        elapsedTime: this.currentTimestamp,
        updateTime: new Date().getTime()
      },
      duration: HmVideoPlayer.getDuration()
    }
    return result
  }

  private setMediaInfo() {
    if (this.avSession && this.mediaInfo) {
      const data: avSession.AVMetadata = {
        assetId: this.avSession.sessionId,
        title: this.mediaInfo.title,
        mediaImage: this.mediaInfo.cover,
        duration: HmVideoPlayer.getDuration()
      }
      this.avSession.setAVMetadata(data)
    }
  }

  private setAvSessionListener(){
    if (this.avSession) {
      // 播放
      this.avSession.on('play', () => {
        this.callbacks.forEach((callback) => {
          callback.play()
        })
      })

      // 暂停
      this.avSession.on('pause', () => {
        this.callbacks.forEach((callback) => {
          callback.pause()
        })
      })

      // 跳转至指定进度
      this.avSession.on('seek', (time) => {
        this.callbacks.forEach((callback) => {
          callback.seek(time)
        })
      })

      // 快进
      this.avSession.on('fastForward', (time) => {
        if (time) {
          this.callbacks.forEach((callback) => {
            callback.fastForward(time)
          })
        }
      })

      // 快退
      this.avSession.on('rewind', (time) => {
        if (time) {
          this.callbacks.forEach((callback) => {
            callback.rewind(time)
          })
        }
      })

      // 下一首
      this.avSession.on('playNext', () => {
        this.callbacks.forEach((callback) => {
          callback.onNext()
        })
      })

      // 上一首
      this.avSession.on('playPrevious', () => {
        this.callbacks.forEach((callback) => {
          callback.onPrevious()
        })
      })
    }
  }

  addAvSessionCallback(callback: AvSessionCallback) {
    this.callbacks.add(callback)
  }

  removeAvSessionCallback(callback: AvSessionCallback) {
    this.callbacks.delete(callback)
  }

  release() {
    this.callbacks.clear()
    if (this.avSession) {
      this.avSession.setAVMetadata({ assetId: "" })
      this.avSession.off("play")
      this.avSession.off("pause")
      this.avSession.off("seek")
      this.avSession.off("fastForward")
      this.avSession.off("rewind")
      this.avSession.deactivate()
      this.avSession.destroy()
    }
  }
}