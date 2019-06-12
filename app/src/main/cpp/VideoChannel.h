//
// Created by Administrator on 2019/6/12.
//

#ifndef VIDEOPLAYER_VIDEOCHANNEL_H
#define VIDEOPLAYER_VIDEOCHANNEL_H

#include "JavaCallHelper.h"
extern "C" {
#include "libavcodec/avcodec.h"
}

class VideoChannel {
public:
    VideoChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *codecContext);
};


#endif //VIDEOPLAYER_VIDEOCHANNEL_H
