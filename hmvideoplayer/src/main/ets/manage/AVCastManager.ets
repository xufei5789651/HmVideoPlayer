import { avSession } from "@kit.AVSessionKit"
import { BusinessError } from '@kit.BasicServicesKit';
import { AvSessionManager } from "./AvSessionManager";
import { CastMediaInfo } from "../media/CastMediaInfo";
import { CastDeviceCallback } from "../media/CastDeviceCallback";

const TAG: string = 'AVCastManager';

/**
 * 投播管理器：封装 AVCastController 的投播控制能力，
 * 配合 HmCastPicker（AVCastPicker 封装）完成音视频投播。
 */
export class AVCastManager {
  private session?: avSession.AVSession = undefined;
  private castController?: avSession.AVCastController = undefined;
  private castDeviceCallback?: CastDeviceCallback = undefined;
  private playbackStateChangeCallback?: (state: avSession.AVPlaybackState) => void = undefined;
  private seekDoneCallback?: (positionMs: number) => void = undefined;
  private isRemoteConnected: boolean = false;

  /**
   * 注册投播设备监听：
   * 内部完成投播能力声明（setExtras url-cast）并监听 AVSession 设备变化，
   * 远端投播设备连接成功后自动获取 AVCastController。
   */
  setCastDeviceListener(avSessionManager: AvSessionManager, callback: CastDeviceCallback) {
    let session: avSession.AVSession | undefined = avSessionManager.getSession();
    if (!session) {
      console.error(TAG, "setCastDeviceListener failed, session is not created")
      return
    }
    this.session = session
    // const 保证闭包内保持非空收窄
    const activeSession: avSession.AVSession = session
    this.castDeviceCallback = callback
    // 声明当前应用支持投播（投播开发指导要求）
    session.setExtras({ 'requireAbilityList': ['url-cast'] }).catch((error: BusinessError) => {
      console.error(TAG, `setExtras failed, code: ${error.code}, message: ${error.message}`)
    })
    session.on('outputDeviceChange', (state: avSession.ConnectionState, device: avSession.OutputDeviceInfo) => {
      let currentDevice: avSession.DeviceInfo | undefined = device?.devices?.[0]
      if (state === avSession.ConnectionState.STATE_CONNECTED &&
        currentDevice?.castCategory === avSession.AVCastCategory.CATEGORY_REMOTE) {
        // 远端投播设备连接成功，获取投播控制器
        console.info(TAG, "remote cast device connected")
        this.isRemoteConnected = true
        this.getCastController(activeSession)
      } else if (state === avSession.ConnectionState.STATE_DISCONNECTED && this.isRemoteConnected) {
        // 远端投播设备断开
        console.info(TAG, "remote cast device disconnected")
        this.isRemoteConnected = false
        this.castController = undefined
        this.castDeviceCallback?.onDisconnected?.()
      }
    })
  }

  /**
   * 开始投播：prepare + start 投播媒体资源
   * @param castMediaInfo 投播媒体信息
   * @returns 是否成功发起投播
   */
  async startCast(castMediaInfo: CastMediaInfo): Promise<boolean> {
    if (!this.castController) {
      console.error(TAG, "startCast failed, cast controller is not ready, remote device not connected")
      return false
    }
    let playItem: avSession.AVQueueItem = {
      itemId: 0,
      description: {
        assetId: castMediaInfo.assetId,
        title: castMediaInfo.title,
        mediaUri: castMediaInfo.mediaUri,
        mediaType: 'VIDEO',
        startPosition: castMediaInfo.startPosition,
        duration: castMediaInfo.duration,
        mediaImage: castMediaInfo.cover
      }
    }
    try {
      // 准备播放，进行资源加载和缓冲，不会触发真正的播放
      await this.castController.prepare(playItem)
      // 启动播放，真正触发对端播放（需在 prepare 成功后调用）
      await this.castController.start(playItem)
      console.info(TAG, "startCast success")
      return true
    } catch (error) {
      let err = error as BusinessError
      console.error(TAG, `startCast failed, code: ${err.code}, message: ${err.message}`)
      return false
    }
  }

  /**
   * 结束投播
   */
  async stopCast(avSessionManager: AvSessionManager) {
    let session: avSession.AVSession | undefined = avSessionManager.getSession();
    if (!session) {
      console.error(TAG, "stopCast failed, session is not created")
      return
    }
    try {
      await session.stopCasting()
      console.info(TAG, "stopCast success")
    } catch (error) {
      let err = error as BusinessError
      console.error(TAG, `stopCast failed, code: ${err.code}, message: ${err.message}`)
    }
  }

  /**
   * 远端播放
   */
  async play(): Promise<void> {
    await this.sendControlCommand('play')
  }

  /**
   * 远端暂停
   */
  async pause(): Promise<void> {
    await this.sendControlCommand('pause')
  }

  /**
   * 远端进度跳转
   * @param positionMs 进度（ms）
   */
  async seek(positionMs: number): Promise<void> {
    await this.sendControlCommand('seek', positionMs)
  }

  /**
   * 远端音量设置
   * @param volume 音量
   */
  async setVolume(volume: number): Promise<void> {
    await this.sendControlCommand('setVolume', volume)
  }

  /**
   * 监听远端播放状态变化
   */
  onCastPlaybackStateChange(callback: (state: avSession.AVPlaybackState) => void) {
    this.playbackStateChangeCallback = callback
    this.castController?.on('playbackStateChange', 'all', callback)
  }

  /**
   * 监听远端进度调节完成（快进、快退、进度条拖拽完毕）
   */
  onCastSeekDone(callback: (positionMs: number) => void) {
    this.seekDoneCallback = callback
    this.castController?.on('seekDone', callback)
  }

  /**
   * 获取远端当前播放状态
   */
  async getCastPlaybackState(): Promise<avSession.AVPlaybackState | undefined> {
    if (!this.castController) {
      console.error(TAG, "getCastPlaybackState failed, cast controller is not ready")
      return undefined
    }
    try {
      return await this.castController.getAVPlaybackState()
    } catch (error) {
      let err = error as BusinessError
      console.error(TAG, `getCastPlaybackState failed, code: ${err.code}, message: ${err.message}`)
      return undefined
    }
  }

  /**
   * 释放投播资源
   */
  release() {
    if (this.castController) {
      try {
        this.castController.off('playbackStateChange')
        this.castController.off('seekDone')
        this.castController.release().catch((error: BusinessError) => {
          console.error(TAG, `release failed, code: ${error.code}, message: ${error.message}`)
        })
      } catch (error) {
        let err = error as BusinessError
        console.error(TAG, `release failed, code: ${err.code}, message: ${err.message}`)
      }
      this.castController = undefined
    }
    this.session?.off('outputDeviceChange')
    this.isRemoteConnected = false
    this.playbackStateChangeCallback = undefined
    this.seekDoneCallback = undefined
    this.castDeviceCallback = undefined
  }

  private async getCastController(session: avSession.AVSession) {
    try {
      this.castController = await session.getAVCastController()
      console.info(TAG, "getAVCastController success")
      // 注册延迟监听（设备连接前注册的回调在此补挂）
      if (this.playbackStateChangeCallback) {
        this.castController.on('playbackStateChange', 'all', this.playbackStateChangeCallback)
      }
      if (this.seekDoneCallback) {
        this.castController.on('seekDone', this.seekDoneCallback)
      }
      this.castDeviceCallback?.onConnected?.()
    } catch (error) {
      let err = error as BusinessError
      console.error(TAG, `getAVCastController failed, code: ${err.code}, message: ${err.message}`)
    }
  }

  private async sendControlCommand(command: avSession.AVCastControlCommandType, parameter?: number) {
    if (!this.castController) {
      console.error(TAG, `sendControlCommand ${command} failed, cast controller is not ready`)
      return
    }
    let avCommand: avSession.AVCastControlCommand = {
      command: command
    }
    if (parameter !== undefined) {
      avCommand.parameter = parameter
    }
    try {
      await this.castController.sendControlCommand(avCommand)
      console.info(TAG, `sendControlCommand ${command} success`)
    } catch (error) {
      let err = error as BusinessError
      console.error(TAG, `sendControlCommand ${command} failed, code: ${err.code}, message: ${err.message}`)
    }
  }
}
