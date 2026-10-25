# 1.0.5
移除 setAvSessionPlaybackState 的 currentTimestamp 参数，播控中心进度统一由 setAvSessionSeekChanged 同步，并修复暂停中拖动进度条时播控中心状态显示不一致的问题

# 1.0.4
修复 onAudioError/onCodecError 回调不生效、音频打断回调被覆盖、EOS 后音频输入线程未退出、快速状态切换回调丢失、回调上下文误释放、视频解码宽高配置互换、HTTP 协议无法播放及线程创建失败路径死锁问题

# 1.0.3
增加播控中心、后台长时任务

# 1.0.2
增加注册音频错误监听、注册音视频编解码运行错误监听、注册码流信息变化，如：声道变化等

# 1.0.1
增加注册音频焦点变化监听、注册输出设备变化监听

# 1.0.0
基于HarmonyOS SDK 一款视频播放器
