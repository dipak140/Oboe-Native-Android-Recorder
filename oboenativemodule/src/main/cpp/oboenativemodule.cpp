#include <jni.h>
#include "LiveEffectEngine.h"

static const int kOboeApiAAudio = 0;
static const int kOboeApiOpenSLES = 1;
static LiveEffectEngine * engine = nullptr;
JavaVM* g_javaVM = nullptr; // Define JavaVM pointer
jobject g_callbackObject = nullptr; // Define g_callbackObject

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_javaVM = vm; // Store the JavaVM pointer
    return JNI_VERSION_1_6; // Return the JNI version you're using
}

extern "C" {
    JNIEXPORT jboolean JNICALL
    Java_in_reconv_oboenativemodule_LiveEffectEngine_create(JNIEnv * env,
                                                      jclass) {
        if (engine == nullptr) {
            engine = new LiveEffectEngine();
        }

        return (engine != nullptr) ? JNI_TRUE : JNI_FALSE;
    }

    JNIEXPORT void JNICALL
    Java_in_reconv_oboenativemodule_LiveEffectEngine_delete(JNIEnv * env, jclass) {
        if (engine) {
            engine -> setEffectOn(false);
            delete engine;
            engine = nullptr;
        }
    }

    JNIEXPORT jboolean JNICALL
    Java_in_reconv_oboenativemodule_LiveEffectEngine_setEffectOn(
            JNIEnv * env, jclass, jboolean isEffectOn) {
        if (engine == nullptr) {
            return JNI_FALSE;
        }
        return engine -> setEffectOn(isEffectOn) ? JNI_TRUE : JNI_FALSE;
    }

    JNIEXPORT void JNICALL
    Java_in_reconv_oboenativemodule_LiveEffectEngine_setRecordingDeviceId(
            JNIEnv * env, jclass, jint deviceId) {
        if (engine == nullptr) {
            return;
        }

        engine -> setRecordingDeviceId(deviceId);
    }

    JNIEXPORT void JNICALL
    Java_in_reconv_oboenativemodule_LiveEffectEngine_setPlaybackDeviceId(
            JNIEnv * env, jclass, jint deviceId) {
        if (engine == nullptr) {
            return;
        }

        engine -> setPlaybackDeviceId(deviceId);
    }

    JNIEXPORT jboolean JNICALL
    Java_in_reconv_oboenativemodule_LiveEffectEngine_setAPI(JNIEnv * env,
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
    Java_in_reconv_oboenativemodule_LiveEffectEngine_isAAudioRecommended(
            JNIEnv * env, jclass type) {
        if (engine == nullptr) {
            return JNI_FALSE;
        }
        return engine -> isAAudioRecommended() ? JNI_TRUE : JNI_FALSE;
    }

    JNIEXPORT void JNICALL
    Java_in_reconv_oboenativemodule_LiveEffectEngine_native_1setDefaultStreamValues(JNIEnv * env,
                                                                              jclass type,
                                                                              jint sampleRate,
                                                                              jint framesPerBurst) {
        oboe::DefaultStreamValues::SampleRate = (int32_t) sampleRate;
        oboe::DefaultStreamValues::FramesPerBurst = (int32_t) framesPerBurst;
    }

    JNIEXPORT void JNICALL
    Java_in_reconv_oboenativemodule_LiveEffectEngine_setVolume(JNIEnv *env , jclass type, jfloat volume) {
        engine ->setVolume(volume);
    }

    JNIEXPORT void JNICALL
    Java_in_reconv_oboenativemodule_LiveEffectEngine_startRecording(
            JNIEnv * env, jclass, jstring fullPathToFile, jint inputPresetPreference, jlong startRecordingTimestamp) {
        if (engine == nullptr) {
            return;
        }

        oboe::InputPreset inputPreset;
        switch (inputPresetPreference) {
            case 10:
                inputPreset = oboe::InputPreset::VoicePerformance;
                break;
            case 9:
                inputPreset = oboe::InputPreset::Unprocessed;
                break;
            default:
                return;
        }

        const char *path = (*env).GetStringUTFChars(fullPathToFile, 0);
        engine -> startRecording(path, inputPreset, startRecordingTimestamp);
    }

    JNIEXPORT void JNICALL
    Java_in_reconv_oboenativemodule_LiveEffectEngine_startRecordingWithoutFile(
            JNIEnv * env, jclass, jstring fullPathToFile, jstring musicFilePath,
            jint inputPresetPreference, jlong startRecordingTimestamp) {
        if (engine == nullptr) {
            return;
        }

        oboe::InputPreset inputPreset;
        switch (inputPresetPreference) {
            case 10:
                inputPreset = oboe::InputPreset::VoicePerformance;
                break;
            case 9:
                inputPreset = oboe::InputPreset::Unprocessed;
                break;
            default:
                return;
        }

        const char *path = (*env).GetStringUTFChars(fullPathToFile, 0);
        const char *musicPath = (*env).GetStringUTFChars(musicFilePath, 0);
        engine -> startRecordingWithoutFile(path, musicPath, inputPreset, startRecordingTimestamp);
    }


JNIEXPORT void JNICALL
    Java_in_reconv_oboenativemodule_LiveEffectEngine_stopRecording(
            JNIEnv * env, jclass) {
        if (engine == nullptr) {
            return;
        }
        engine ->stopRecording();
    }

    JNIEXPORT void JNICALL
    Java_in_reconv_oboenativemodule_LiveEffectEngine_pauseRecording(
            JNIEnv * env, jclass) {
        if (engine == nullptr) {
            return;
        }
        engine ->pauseRecording();
    }

    JNIEXPORT void JNICALL
    Java_in_reconv_oboenativemodule_LiveEffectEngine_resumeRecording(
            JNIEnv * env, jclass) {
        if (engine == nullptr) {
            return;
        }
        engine ->resumeRecording();
    }

    JNIEXPORT int JNICALL
    Java_in_reconv_oboenativemodule_LiveEffectEngine_getRecordingDelay(
            JNIEnv * env, jclass) {
        if (engine == nullptr) {
            return 0;
        }
        return engine ->getStartRecordingDelay();
    }


}



extern "C"
JNIEXPORT void JNICALL
Java_in_reconv_oboenativemodule_LiveEffectEngine_setCallbackObject(JNIEnv* env, jclass clazz, jobject callbackObject) {
    // Delete the previous global reference if it exists
    if (g_callbackObject != nullptr) {
        env->DeleteGlobalRef(g_callbackObject);
    }
    // Create a new global reference to the callback object
    g_callbackObject = env->NewGlobalRef(callbackObject);
}