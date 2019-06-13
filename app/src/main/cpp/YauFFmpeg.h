//
// Created by Administrator on 2019/6/12.
//

#ifndef VIDEOPLAYER_YAUFFMPEG_H
#define VIDEOPLAYER_YAUFFMPEG_H

#include <pthread.h>
#include <android/native_window.h>
#include "JavaCallHelper.h"
#include "VideoChannel.h"
#include "AudioChannel.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/time.h"
};

// 控制层
class YauFFmpeg {
public:
    YauFFmpeg(JavaCallHelper *javaCallHelper_, const char *dataSource);
    ~YauFFmpeg();
    void prepare();
    void prepareFFmpeg();
    void start();
    void play();
    void setRenderCallback(RenderFrame renderFrame);

private:
    bool isPlaying;
    char *url;
    pthread_t pid_prepare;
    pthread_t pid_play;
    VideoChannel *videoChannel;
    AudioChannel *audioChannel;
    AVFormatContext *formatContext;
    JavaCallHelper *javaCallHelper;
    RenderFrame renderFrame;
};


#endif //VIDEOPLAYER_YAUFFMPEG_H
