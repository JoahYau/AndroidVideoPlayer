# VideoPlayer

## 0、说明

一个简单的视频播放器，通过ffmpeg实现核心功能，学习用。

## 1、播放器架构

整体功能解耦，native-lib只负责jni相关操作，而音视频的解码分别抽出一层，中间通过控制层传递、分发。

![image](https://www.cnblogs.com/images/cnblogs_com/joahyau/1479526/o_player_struct.png)

## 2、音视频解码层

通过自己管理队列的方式，可以给队列加限制，当队列大于一定数量时，则停止生产，避免出现内存消耗过大的情况。

管理packet、frame两个队列，可避免出现死锁的情况。

![image](https://www.cnblogs.com/images/cnblogs_com/joahyau/1479526/o_video_channel.png)

![image](https://www.cnblogs.com/images/cnblogs_com/joahyau/1479526/o_audio_channel.png)
