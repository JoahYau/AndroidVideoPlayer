//
// Created by Administrator on 2019/6/12.
//

#include "AudioChannel.h"

AudioChannel::AudioChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *codecContext)
        : BaseChannel(id, javaCallHelper, codecContext) {

}

void *audioPlay_(void *args) {
    auto audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->initOpenSL();
    return nullptr;
}

void *audioDecode_(void *args) {
    auto audioChannel = static_cast<AudioChannel *>(args);
    return nullptr;
}

void AudioChannel::play() {
    pkt_queue.setWork(1);
    frame_queue.setWork(1);
    isPlaying = true;

    pthread_create(pid_audio_play, nullptr, audioPlay_, this);
    pthread_create(pid_audio_decode, nullptr, audioPlay_, this);
}

void AudioChannel::stop() {

}

// 当第一次手动调用该方法进行启动后，后面会不断回调
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    
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
    SLDataLocator_AndroidSimpleBufferQueue androidQueue = {SL_DATALOCATOR_ANDROIDBUFFERQUEUE, 2};
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
