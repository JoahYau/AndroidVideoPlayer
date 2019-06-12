//
// Created by Administrator on 2019/6/12.
//

#ifndef VIDEOPLAYER_AUDIOCHANNEL_H
#define VIDEOPLAYER_AUDIOCHANNEL_H

#include "BaseChannel.h"

class AudioChannel : public BaseChannel {
public:
    AudioChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *codecContext);

    virtual void play();

    virtual void stop();
};


#endif //VIDEOPLAYER_AUDIOCHANNEL_H
