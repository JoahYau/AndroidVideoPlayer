//
// Created by Administrator on 2019/6/12.
//

#include "YauFFmpeg.h"
#include "macro.h"

void *prepareFFmpeg_(void *args) {
    // 这里是回调函数，不能通过this来访问成员，所以要将整个对象传过来
    auto *yauFFmpeg = static_cast<YauFFmpeg *>(args);
    yauFFmpeg->prepareFFmpeg();
    return nullptr;
}

void *play_(void *args) {
    // 这里是回调函数，不能通过this来访问成员，所以要将整个对象传过来
    auto *yauFFmpeg = static_cast<YauFFmpeg *>(args);
    yauFFmpeg->play();
    return nullptr;
}

YauFFmpeg::YauFFmpeg(JavaCallHelper *javaCallHelper_, const char *dataSource) : javaCallHelper(javaCallHelper_) {
    url = new char[strlen(dataSource) + 1];
    strcpy(url, dataSource);
}

YauFFmpeg::~YauFFmpeg() {
    if (formatContext) {
        avformat_close_input(&formatContext);
    }
}

void YauFFmpeg::prepare() {
    pthread_create(&pid_prepare, nullptr, prepareFFmpeg_, this);
}

void YauFFmpeg::prepareFFmpeg() {
    // 这里是子线程，但是可以访问到对象的属性
    avformat_network_init();
    formatContext = avformat_alloc_context();
    AVDictionary *opts = nullptr;
    av_dict_set(&opts, "timeout", "3000000", 0);

    // 打开文件，第三个参数强制指定AVFormatContext中的AVInputFormat，设置为空则ffmpeg自动检测AVInputFormat
    // 输入文件的封装格式
//    av_find_input_format("avi");
    if (avformat_open_input(&formatContext, url, nullptr, &opts)) {
        javaCallHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL);
        return;
    }

    // 查找流信息
    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        javaCallHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_FIND_STREAMS);
        return;
    }

    for (int i = 0; i < formatContext->nb_streams; ++i) {
        AVCodecParameters *codecpar = formatContext->streams[i]->codecpar;
        // 找到解码器
        AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);
        if (!codec) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL);
            return;
        }

        // 创建解码器上下文
        AVCodecContext *codecContext = avcodec_alloc_context3(codec);
        if (!codecContext) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            return;
        }

        // 复制参数
        if (avcodec_parameters_to_context(codecContext, codecpar) < 0) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            return;
        }

        // 打开解码器
        if (avcodec_open2(codecContext, codec, nullptr)) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL);
            return;
        }

        if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            // 音频
            audioChannel = new AudioChannel(i, javaCallHelper, codecContext);
        } else if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            // 视频
            videoChannel = new VideoChannel(i, javaCallHelper, codecContext);
            videoChannel->setRenderCallback(renderFrame);
        }
    }

    // 音视频都没有
    if (!audioChannel && !videoChannel) {
        javaCallHelper->onError(THREAD_CHILD, FFMPEG_NOMEDIA);
        return;
    }

    javaCallHelper->onPrepare(THREAD_CHILD);
}

void YauFFmpeg::start() {
    isPlaying = true;
    if (audioChannel) {
        audioChannel->play();
    }
    if (videoChannel) {
        videoChannel->play();
    }

    pthread_create(&pid_play, nullptr, play_, this);
}

void YauFFmpeg::play() {
    int ret;
    while (isPlaying) {
        // 生产速度远大于消费速度，所以这里达到一定数量则延缓10ms
        if ((audioChannel && audioChannel->pkt_queue.size() > 100) ||
                (videoChannel && videoChannel->pkt_queue.size() > 100)) {
            av_usleep(1000 * 10);
            continue;
        }

        AVPacket *packet = av_packet_alloc();
        ret = av_read_frame(formatContext, packet);
        if (ret == 0) {
            if (audioChannel && packet->stream_index == audioChannel->channelId) {
//                audioChannel->pkt_queue.enQueue(packet);
            } else if (videoChannel && packet->stream_index == videoChannel->channelId) {
                videoChannel->pkt_queue.enQueue(packet);
            }
        } else if (ret == AVERROR_EOF) {
            if (videoChannel->pkt_queue.empty() && videoChannel->frame_queue.empty() &&
                audioChannel->pkt_queue.empty() && audioChannel->frame_queue.empty()) {
                LOGE("播放完毕");
                break;
            }
        } else {
            break;
        }
    }

    isPlaying = false;
    if (audioChannel) {
        audioChannel->stop();
    }
    if (videoChannel) {
        videoChannel->stop();
    }
}

void YauFFmpeg::setRenderCallback(RenderFrame renderFrame) {
    this->renderFrame = renderFrame;
}
