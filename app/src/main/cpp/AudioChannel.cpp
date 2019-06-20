//
// Created by Administrator on 2019/6/12.
//

#include "AudioChannel.h"

AudioChannel::AudioChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *codecContext, AVRational time_base)
        : BaseChannel(id, javaCallHelper, codecContext, time_base) {
    out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    out_sample_size = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    out_sample_rate = 44100;
    buffer = (uint8_t *) malloc(out_sample_rate * out_sample_size * out_channels);
}

void *audioPlay_(void *args) {
    auto audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->initOpenSL();
    return nullptr;
}

void *audioDecode_(void *args) {
    auto audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->decode();
    return nullptr;
}

void AudioChannel::play() {
    swr_ctx = swr_alloc_set_opts(0, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, out_sample_rate,
                        avCodecContext->channel_layout,
                        avCodecContext->sample_fmt,
                        avCodecContext->sample_rate, 0, 0);
    swr_init(swr_ctx);

    startWork();
    isPlaying = true;

    pthread_create(&pid_audio_play, nullptr, audioPlay_, this);
    pthread_create(&pid_audio_decode, nullptr, audioDecode_, this);
}

void AudioChannel::stop() {

}

// 当第一次手动调用该方法进行启动后，后面会不断回调
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    auto audioChannel = static_cast<AudioChannel *>(context);
    int dataLen = audioChannel->getPcm();
    if (dataLen > 0) {
        (*bq)->Enqueue(bq, audioChannel->buffer, dataLen);
    }
}

void AudioChannel::initOpenSL() {
    // 音频引擎
    SLEngineItf engineInterface = nullptr;
    // 音频对象
    SLObjectItf engineObject = nullptr;
    // 混音器
    SLObjectItf outputMixObject = nullptr;

    // 播放器
    SLObjectItf  bqPlayerObject = nullptr;
    // 回调接口
    SLPlayItf bqPlayerInterface = nullptr;
    // 缓冲队列
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = nullptr;

    // ------------------------------------------------------------
    // 创建引擎
    SLresult result;
    result = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
    if (result != SL_RESULT_SUCCESS) {
        return;
    }

    // 初始化引擎
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        return;
    }

    // 获取引擎接口，相当于SurfaceView的SurfaceHolder
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineInterface);
    if (result != SL_RESULT_SUCCESS) {
        return;
    }

    // ------------------------------------------------------------
    // 创建混音器
    result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 0, nullptr, nullptr);
    // 初始化混音器
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        return;
    }

    // ------------------------------------------------------------
    // 创建播放器
    SLDataLocator_AndroidSimpleBufferQueue androidQueue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM pcm = {SL_DATAFORMAT_PCM, // 播放pcm格式的数据
                            2, // 双声道
                            SL_SAMPLINGRATE_44_1, // 44.1KHz采样频率
                            SL_PCMSAMPLEFORMAT_FIXED_16, // 位数，16位
                            SL_PCMSAMPLEFORMAT_FIXED_16, // 与位数一致即可
                            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, // 立体声（前左前右）
                            SL_BYTEORDER_LITTLEENDIAN // 小端模式
    };
    SLDataSource slDataSource = {&androidQueue, &pcm};

    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, nullptr};

    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    result = (*engineInterface)->CreateAudioPlayer(engineInterface,
            &bqPlayerObject, // 播放器
            &slDataSource, // 播放器参数，播放缓冲队列、播放格式等
            &audioSnk, // 播放缓冲区
            1, // 回调接口数量，有些情况需要不同喇叭播放不同音源，这时需要设置不同回调接口
            ids, // 播放队列ID
            req // 是否采用内置的播放队列
            );

    // 初始化播放器
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);

    // ------------------------------------------------------------
    // 获取播放器接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerInterface);
    // 获取缓冲队列接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueue);
    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);

    // ------------------------------------------------------------
    (*bqPlayerInterface)->SetPlayState(bqPlayerInterface, SL_PLAYSTATE_PLAYING);

    // ------------------------------------------------------------
    bqPlayerCallback(bqPlayerBufferQueue, this);
}

void AudioChannel::decode() {
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
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret < 0) {
            break;
        }

        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret < 0) {
            break;
        }

        frame_queue.enQueue(frame);
        while (frame_queue.size() > 100 && isPlaying) {
            av_usleep(10 * 1000);
        }
    }
}

int AudioChannel::getPcm() {
    AVFrame *frame = nullptr;
    int data_size = 0;
    while (isPlaying) {
        int ret = frame_queue.deQueue(frame);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret < 0) {
            break;
        }

        uint64_t dst_nb_samples = av_rescale_rnd(
                swr_get_delay(swr_ctx, frame->sample_rate) + frame->nb_samples,
                out_sample_rate,
                frame->sample_rate,
                AV_ROUND_UP);
        // 转换，返回值为转换后的sample个数
        int nb = swr_convert(swr_ctx, &buffer, dst_nb_samples,
                             (const uint8_t **) frame->data, frame->nb_samples);
        // 转换后多少数据，44100 * 2 * 2
        data_size = nb * out_channels * out_sample_size;

        // pts代表时间进度，time_base是单位
        clock = frame->pts * av_q2d(time_base);

        break;
    }
    releaseAVFrame(frame);
    return data_size;
}
