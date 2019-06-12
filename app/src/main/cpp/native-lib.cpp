#include <jni.h>
#include <string>
#include <android/native_window_jni.h>

extern "C" {
#include "libavcodec/avcodec.h"
}

ANativeWindow *window = 0;

extern "C" JNIEXPORT void JNICALL
Java_com_yau_videoplayer_player_VideoPlayer_nPrepare(JNIEnv *env, jobject instance,
                                                     jstring dataSource_) {
    const char *dataSource = env->GetStringUTFChars(dataSource_, 0);



    env->ReleaseStringUTFChars(dataSource_, dataSource);
}

extern "C" JNIEXPORT void JNICALL
Java_com_yau_videoplayer_player_VideoPlayer_nSetSurface(JNIEnv *env, jobject instance,
                                                        jobject surface) {
    if (window) {
        ANativeWindow_release(window);
        window = 0;
    }
    // 创建新的窗口用于视频显示
    window = ANativeWindow_fromSurface(env, surface);
}

extern "C" JNIEXPORT void JNICALL
Java_com_yau_videoplayer_player_VideoPlayer_nStart(JNIEnv *env, jobject instance) {

}