//
// Created by Administrator on 2019/6/12.
//

#ifndef VIDEOPLAYER_AUDIOCHANNEL_H
#define VIDEOPLAYER_AUDIOCHANNEL_H

#include <SLES/OpenSLES_Android.h>
#include "BaseChannel.h"
extern "C" {
#include <libswresample/swresample.h>
}

class AudioChannel : public BaseChannel {
public:
    AudioChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *codecContext);
    virtual void play();
    virtual void stop();
    void initOpenSL();
    void decode();
    int getPcm();

private:
    pthread_t pid_audio_play;
    pthread_t pid_audio_decode;
    SwrContext *swr_ctx = nullptr;
    int out_channels;
    int out_sample_size;
    int out_sample_rate;

public:
    uint8_t *buffer;
};


#endif //VIDEOPLAYER_AUDIOCHANNEL_H
