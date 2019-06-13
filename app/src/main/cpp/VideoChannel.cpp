//
// Created by Administrator on 2019/6/12.
//

#include "VideoChannel.h"
extern "C" {
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

VideoChannel::VideoChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *codecContext)
        : BaseChannel(id, javaCallHelper, codecContext) {

}

void *decode_(void *args) {
    auto *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->decodePacket();
    return nullptr;
}

void *synchronize_(void *args) {
    auto *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->synchronizeFrame();
    return nullptr;
}

void VideoChannel::play() {
    pkt_queue.setWork(1);
    frame_queue.setWork(1);
    isPlaying = true;

    pthread_create(&pid_video_play, nullptr, decode_, this);
    pthread_create(&pid_synchronize, nullptr, synchronize_, this);
}

void VideoChannel::stop() {

}

// 解码线程
void VideoChannel::decodePacket() {
    AVPacket *packet = nullptr;
    while (isPlaying) {
        int ret = pkt_queue.deQueue(packet);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }

        ret = avcodec_send_packet(avCodecContext, packet);
        releaseAVPacket(packet);
        if (ret == AVERROR(EAGAIN)) {
            // 需要更多数据
            continue;
        } else if (ret < 0) {
            // 失败
            break;
        }

        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, frame);

        // 压缩数据，解压
        // frame队列也要限制
        frame_queue.enQueue(frame);
        while (frame_queue.size() > 100 && isPlaying) {
            av_usleep(1000 * 10);
        }
    }
    releaseAVPacket(packet);
}

// 播放线程
void VideoChannel::synchronizeFrame() {
    SwsContext *swsContext = sws_getContext(
            avCodecContext->width, avCodecContext->height, avCodecContext->pix_fmt,
            avCodecContext->width, avCodecContext->height, AV_PIX_FMT_RGBA,
            SWS_BILINEAR, nullptr, nullptr, nullptr);

    uint8_t *dst_data[4]; // argb
    int dst_linesize[4];
    // 声明一个容器，一帧大小
    av_image_alloc(dst_data, dst_linesize,
                    avCodecContext->width, avCodecContext->height, AV_PIX_FMT_RGBA, 1);

    AVFrame *frame = nullptr;
    while (isPlaying) {
        int ret = frame_queue.deQueue(frame);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }

        sws_scale(swsContext, frame->data, frame->linesize, 0,
                    frame->height, dst_data, dst_linesize);
        if (renderFrame) {
            renderFrame(dst_data[0], dst_linesize[0], avCodecContext->width, avCodecContext->height);
        }
        // 16ms播放一帧
        LOGE("解码一帧数据");
        av_usleep(16 * 1000);
        releaseAVFrame(frame);
    }

    isPlaying = false;
    av_freep(&dst_data[0]);
    releaseAVFrame(frame);
    sws_freeContext(swsContext);
}

void VideoChannel::setRenderCallback(RenderFrame renderFrame) {
    this->renderFrame = renderFrame;
}
