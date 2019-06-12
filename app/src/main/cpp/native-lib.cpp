#include <jni.h>
#include <string>
extern "C" {
#include <libavutil/avutil.h>
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_yau_videoplayer_MainActivity_stringFromJNI(JNIEnv *env, jobject) {
    return env->NewStringUTF(av_version_info());
}
