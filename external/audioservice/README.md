### AudioService
本工程实现一个后台音乐播放器，接收前台UI进程的播放请求，实现在单窗口系统中UI与音乐播放逻辑剥离。主要实现的功能包括：切换歌曲、歌曲定位、专辑信息获取等。

在 SDK 中配合 Qt 的 musicPlayer 应用进行使用，由前台的Qt UI进程接收用户请求，通过消息队列定义通信端口与后台服务通信，而音乐播放的主要逻辑则使用 gstreamer 实现。

### 编译方式
可以使用 buildroot 默认交叉编译配置文件(需包含Gstreamer库)
```
$ mkdir build
$ cd build 
$ cmake -D CMAKE_TOOLCHAIN_FILE="../../../buildroot/output/host/usr/share/buildroot/toolchainfile.cmake" ..
```

### 接口使用说明
工程包含音乐播放常用的功能 Gstreamer 实现，接口封装在 **include/AudioInterfaceProvider.cpp** 中，接口封装文件定义了通信的端口、消息、协议等。

另外除了主动的接口使用，工程会主动向客户端发送一些状态，例如：播放错误状态、当前播放进度等等。

主要接口提供：
接口 | 说明
:--|:--
play / pause / stop | 播放状态控制
state | 获取当前的播放状态
duration | 获取当前歌曲的时长
setVolume | 设置音量，控制范围为0~100
volume | 获取当前的音量
setPosition | 设置当前歌曲的播放进度
position | 获取当前歌曲的播放进度
currentMedia | 获取当前播放歌曲路径
getArtist | 获取当前播放歌曲歌手
getTitle | 获取当前播放歌曲歌名
currentPlayModeChanged | 修改当前播放列表循环模式

服务主动上报给客户端的状态包括：
```
msg_type标识
1. RCV_TYPE_MEDIA_STATE_CHANGED
    - 媒体管道流状态改变，携带参数为新媒体状态
    - 参数类型：enum MediaStatus
2. RCV_TYPE_ERROR
    - 媒体流发生错误，携带参数为错误原因
    - 参数类型：enum Error
3. RCV_TYPE_STATE_CHANGED
    - 播放状态改变，携带参数为新播放状态
    - 参数类型：enum State
4. RCV_TYPE_POSITION_CHANGE
    - 媒体当前播放进度发生改变
    - 携带参数为新的进度时长
5. RCV_TYPE_DURATION_CHANGE
    - 媒体总时长发生改变时，也就是切换媒体时
    - 携带参数为新的歌曲播放时长
```
<br><br><br>