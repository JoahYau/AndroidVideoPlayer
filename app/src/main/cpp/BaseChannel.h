//
// Created by Administrator on 2019/6/12.
//

#ifndef VIDEOPLAYER_BASECHANNEL_H
#define VIDEOPLAYER_BASECHANNEL_H

#include "safe_queue.h"
#include "JavaCallHelper.h"
#include "macro.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/frame.h"
}

class BaseChannel {
public:
    BaseChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext) : channelId(id),
                                                                                            javaCallHelper(javaCallHelper),
                                                                                            avCodecContext(avCodecContext) {};
    virtual ~BaseChannel() {
        if (avCodecContext) {
            avcodec_close(avCodecContext);
            avcodec_free_context(&avCodecContext);
            avCodecContext = nullptr;
        }
        pkt_queue.clear();
        frame_queue.clear();
        LOGE("释放队列：%d, %d", pkt_queue.size(), frame_queue.size());
    };

    virtual void play() = 0;
    virtual void stop() = 0;

    SafeQueue<AVPacket *> pkt_queue;
    SafeQueue<AVFrame *> frame_queue;
    volatile int channelId;
    volatile bool isPlaying;
    AVCodecContext *avCodecContext;
    JavaCallHelper *javaCallHelper;
};


#endif //VIDEOPLAYER_BASECHANNEL_H
