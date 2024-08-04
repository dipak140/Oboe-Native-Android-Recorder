#include <jni.h>

#include "LiveEffectEngine.h"

static const int kOboeApiAAudio = 0;
static const int kOboeApiOpenSLES = 1;
static LiveEffectEngine * engine = nullptr;

extern "C" {
JNIEXPORT jboolean JNICALL
Java_in_reconv_oboenative_LiveEffectEngine_create(JNIEnv * env,
                                                                jclass) {
    if (engine == nullptr) {
        engine = new LiveEffectEngine();
    }

    return (engine != nullptr) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_in_reconv_oboenative_LiveEffectEngine_delete(JNIEnv * env, jclass) {
    if (engine) {
        engine -> setEffectOn(false);
        delete engine;
        engine = nullptr;
    }
}

JNIEXPORT jboolean JNICALL
Java_in_reconv_oboenative_LiveEffectEngine_setEffectOn(
        JNIEnv * env, jclass, jboolean isEffectOn) {
    if (engine == nullptr) {
        return JNI_FALSE;
    }

    return engine -> setEffectOn(isEffectOn) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_in_reconv_oboenative_LiveEffectEngine_setRecordingDeviceId(
        JNIEnv * env, jclass, jint deviceId) {
    if (engine == nullptr) {
        return;
    }

    engine -> setRecordingDeviceId(deviceId);
}

JNIEXPORT void JNICALL
Java_in_reconv_oboenative_LiveEffectEngine_setPlaybackDeviceId(
        JNIEnv * env, jclass, jint deviceId) {
    if (engine == nullptr) {
        return;
    }

    engine -> setPlaybackDeviceId(deviceId);
}

JNIEXPORT jboolean JNICALL
Java_in_reconv_oboenative_LiveEffectEngine_setAPI(JNIEnv * env,
                                                                jclass type,
                                                                jint apiType) {
    if (engine == nullptr) {
        return JNI_FALSE;
    }

    oboe::AudioApi audioApi;
    switch (apiType) {
        case kOboeApiAAudio:
            audioApi = oboe::AudioApi::AAudio;
            break;
        case kOboeApiOpenSLES:
            audioApi = oboe::AudioApi::OpenSLES;
            break;
        default:
            return JNI_FALSE;
    }

    return engine -> setAudioApi(audioApi) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_in_reconv_oboenative_LiveEffectEngine_isAAudioRecommended(
        JNIEnv * env, jclass type) {
    if (engine == nullptr) {
        return JNI_FALSE;
    }
    return engine -> isAAudioRecommended() ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_in_reconv_oboenative_LiveEffectEngine_native_1setDefaultStreamValues(JNIEnv * env,
                                                                                        jclass type,
                                                                                        jint sampleRate,
                                                                                        jint framesPerBurst) {
    oboe::DefaultStreamValues::SampleRate = (int32_t) sampleRate;
    oboe::DefaultStreamValues::FramesPerBurst = (int32_t) framesPerBurst;
}
}