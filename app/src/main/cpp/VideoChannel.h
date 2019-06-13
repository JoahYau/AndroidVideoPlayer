//
// Created by Administrator on 2019/6/12.
//

#ifndef VIDEOPLAYER_VIDEOCHANNEL_H
#define VIDEOPLAYER_VIDEOCHANNEL_H

#include "BaseChannel.h"

typedef void (*RenderFrame) (uint8_t *data, int linesize, int width, int height);

class VideoChannel : public BaseChannel {
public:
    VideoChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *codecContext);
    virtual void play();
    virtual void stop();
    void decodePacket();
    void synchronizeFrame();
    void setRenderCallback(RenderFrame renderFrame);

private:
    pthread_t pid_video_play;
    pthread_t pid_synchronize;
    RenderFrame renderFrame;
};


#endif //VIDEOPLAYER_VIDEOCHANNEL_H
