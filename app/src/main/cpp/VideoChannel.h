//
// Created by Administrator on 2019/6/12.
//

#ifndef VIDEOPLAYER_VIDEOCHANNEL_H
#define VIDEOPLAYER_VIDEOCHANNEL_H

#include "BaseChannel.h"

class VideoChannel : public BaseChannel {
public:
    VideoChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *codecContext);

    virtual void play();

    virtual void stop();
};


#endif //VIDEOPLAYER_VIDEOCHANNEL_H
