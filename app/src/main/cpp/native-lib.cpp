#include <jni.h>
#include <string>
#include <android/native_window_jni.h>
#include "JavaCallHelper.h"
#include "YauFFmpeg.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

JavaCallHelper *javaCallHelper;
JavaVM *javaVM = nullptr;

ANativeWindow *window = nullptr;
YauFFmpeg *yauFFmpeg;

// 视频渲染
void renderFrame(uint8_t *data, int linesize, int width, int height) {
    // 设置窗口几何属性
    ANativeWindow_setBuffersGeometry(window, width, height, WINDOW_FORMAT_RGBA_8888);
    // 创建缓冲区并上锁
    ANativeWindow_Buffer window_buffer;
    if (ANativeWindow_lock(window, &window_buffer, nullptr)) {
        ANativeWindow_release(window);
        window = nullptr;
        return;
    }

    // window的缓冲区
    auto *window_data = static_cast<uint8_t *>(window_buffer.bits);
    // 这里的单位是字节，而拷贝要一次拷贝一组，由于有RGBA四个通道四个字节，所以这里乘以4
    int window_linesize = window_buffer.stride * 4;
    for (int i = 0; i < window_buffer.height; ++i) {
        // 偏移量各算各的，拷贝大小以目的地为准
        memcpy(window_data + i * window_linesize,
                data + i * linesize,
                window_linesize);
    }

    ANativeWindow_unlockAndPost(window);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVM = vm;
    return JNI_VERSION_1_4;
}

extern "C" JNIEXPORT void JNICALL
Java_com_yau_videoplayer_player_VideoPlayer_nPrepare(JNIEnv *env, jobject instance,
                                                     jstring dataSource_) {
    const char *dataSource = env->GetStringUTFChars(dataSource_, 0);

    javaCallHelper = new JavaCallHelper(javaVM, env, instance);
    yauFFmpeg = new YauFFmpeg(javaCallHelper, dataSource);
    yauFFmpeg->setRenderCallback(renderFrame);
    yauFFmpeg->prepare();

    env->ReleaseStringUTFChars(dataSource_, dataSource);
}

extern "C" JNIEXPORT void JNICALL
Java_com_yau_videoplayer_player_VideoPlayer_nSetSurface(JNIEnv *env, jobject instance,
                                                        jobject surface) {
    if (window) {
        ANativeWindow_release(window);
        window = nullptr;
    }
    // 创建新的窗口用于视频显示
    window = ANativeWindow_fromSurface(env, surface);
}

extern "C" JNIEXPORT void JNICALL
Java_com_yau_videoplayer_player_VideoPlayer_nStart(JNIEnv *env, jobject instance) {
    if (yauFFmpeg) {
        yauFFmpeg->start();
    }
}

extern "C" JNIEXPORT jint JNICALL
Java_com_yau_videoplayer_player_VideoPlayer_nGetDuration(JNIEnv *env, jobject instance) {
    if (yauFFmpeg) {
        return yauFFmpeg->getDuration();
    }
    return 0;
}

extern "C" JNIEXPORT void JNICALL
Java_com_yau_videoplayer_player_VideoPlayer_nSeekTo(JNIEnv *env, jobject instance, jint proress) {
    if (yauFFmpeg) {
        yauFFmpeg->seekTo(proress);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_yau_videoplayer_player_VideoPlayer_nStop(JNIEnv *env, jobject instance) {
    if (yauFFmpeg) {
        yauFFmpeg->stop();
        DELETE(yauFFmpeg);
    }
    DELETE(javaCallHelper);
}

extern "C" JNIEXPORT void JNICALL
Java_com_yau_videoplayer_player_VideoPlayer_nRelease(JNIEnv *env, jobject instance) {
    if (yauFFmpeg) {
        yauFFmpeg->stop();
        DELETE(yauFFmpeg);
    }
    DELETE(javaCallHelper);
    if (window) {
        ANativeWindow_release(window);
        window = nullptr;
    }
}