//
// Created by Administrator on 2019/6/12.
//

#include "VideoChannel.h"
extern "C" {
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

void dropPacket(queue<AVPacket *> &q) {
    while (!q.empty()) {
        LOGE("=========丢帧==========");

        AVPacket *packet = q.front();
        if (packet->flags != AV_PKT_FLAG_KEY) {
            q.pop();
            BaseChannel::releaseAVPacket(packet);
        } else {
            break;
        }
    }
}

void dropFrame(queue<AVFrame *> &q) {
    if (!q.empty()) {
        AVFrame *frame = q.front();
        q.pop();
        BaseChannel::releaseAVFrame(frame);
    }
}

VideoChannel::VideoChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *codecContext, AVRational time_base)
        : BaseChannel(id, javaCallHelper, codecContext, time_base) {
    frame_queue.setReleaseHandle(releaseAVFrame);
    frame_queue.setSyncHandle(dropFrame);
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
        LOGE("解码一帧数据");

        clock = frame->pts * av_q2d(time_base);
        double audio_clock = audioChannel->clock;
        // 每帧延时时间
        double frame_delay = 1.0 / fps;
        // 解码时间，计算方法参考repeat_pict的注释
        double extra_delay = frame->repeat_pict / (2 * fps);
        double delay = frame_delay + extra_delay;
        double diff = clock - audio_clock;
        LOGE("相差--------------->%d", diff);
        if (clock > audio_clock) {
            // 视频超前
            if (diff > 1) {
                av_usleep(delay * 2 * 1000 * 1000);
            } else {
                av_usleep((delay + diff) * 1000 * 1000);
            }
        } else {
            // 视频落后
            if (diff > 1) {
                // 不休眠
            } else if (diff >= 0.05) {
                // 需要追赶，丢帧
                releaseAVFrame(frame);
                frame_queue.sync();
            } else {
                av_usleep((delay + diff) * 1000 * 1000);
            }
        }

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

void VideoChannel::setFps(int fps) {
    this->fps = fps;
}
