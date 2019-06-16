//
// Created by Administrator on 2019/6/12.
//

#ifndef VIDEOPLAYER_AUDIOCHANNEL_H
#define VIDEOPLAYER_AUDIOCHANNEL_H

#include <SLES/OpenSLES_Android.h>
#include "BaseChannel.h"

class AudioChannel : public BaseChannel {
public:
    AudioChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *codecContext);
    virtual void play();
    virtual void stop();

    void initOpenSL();

private:
    pthread_t *pid_audio_play;
    pthread_t *pid_audio_decode;
};


#endif //VIDEOPLAYER_AUDIOCHANNEL_H
