import { common, wantAgent } from "@kit.AbilityKit"
import { backgroundTaskManager } from "@kit.BackgroundTasksKit"

export class BackgroundPlayManager {
  static async startBackground(context: common.BaseContext, agentInfo: wantAgent.WantAgentInfo) {
    const agent = await wantAgent.getWantAgent(agentInfo)
    backgroundTaskManager.startBackgroundRunning(context, backgroundTaskManager.BackgroundMode.AUDIO_PLAYBACK, agent)
  }

  static async stopBackground(context: common.BaseContext) {
    backgroundTaskManager.stopBackgroundRunning(context)
  }
}