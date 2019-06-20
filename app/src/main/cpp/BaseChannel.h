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
#include "libavutil/time.h"
}

class BaseChannel {
public:
    BaseChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext, AVRational time_base) : channelId(id),
                                                                                            javaCallHelper(javaCallHelper),
                                                                                            avCodecContext(avCodecContext),
                                                                                            time_base(time_base) {
        pkt_queue.setReleaseHandle(releaseAVPacket);
        frame_queue.setReleaseHandle(releaseAVFrame);
    };
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

    static void releaseAVPacket(AVPacket *&packet) {
        if (packet) {
            av_packet_free(&packet);
            packet = nullptr;
        }
    };

    static void releaseAVFrame(AVFrame *&frame) {
        if (frame) {
            av_frame_free(&frame);
            frame = nullptr;
        }
    };

    void clear() {
        stopWork();
        pkt_queue.clear();
        frame_queue.clear();
        startWork();
    }

    void startWork() {
        pkt_queue.setWork(1);
        frame_queue.setWork(1);
    }

    void stopWork() {
        pkt_queue.setWork(0);
        frame_queue.setWork(0);
    }

    virtual void play() = 0;
    virtual void stop() = 0;

    SafeQueue<AVPacket *> pkt_queue;
    SafeQueue<AVFrame *> frame_queue;
    volatile int channelId;
    volatile bool isPlaying;
    AVCodecContext *avCodecContext;
    JavaCallHelper *javaCallHelper;
    AVRational time_base;
    double clock = 0;
};


#endif //VIDEOPLAYER_BASECHANNEL_H
