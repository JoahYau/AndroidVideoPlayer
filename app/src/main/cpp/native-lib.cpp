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