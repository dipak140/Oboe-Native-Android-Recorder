#include <oboe/Oboe.h>
#include <jni.h>
#include <android/log.h>
#include <vector>
#include <mutex>

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "Oboe", __VA_ARGS__)

class AudioEngine : public oboe::AudioStreamCallback {
public:
    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) override {
        std::lock_guard<std::mutex> lock(mMutex);
        if (oboeStream == mRecordingStream) {
            float *floatData = static_cast<float *>(audioData);
            mRecordingBuffer.insert(mRecordingBuffer.end(), floatData, floatData + numFrames);
        } else if (oboeStream == mPlaybackStream) {
            float *floatData = static_cast<float *>(audioData);
            if (mPlaybackBufferIndex + numFrames <= mRecordingBuffer.size()) {
                std::copy(mRecordingBuffer.begin() + mPlaybackBufferIndex,
                          mRecordingBuffer.begin() + mPlaybackBufferIndex + numFrames,
                          floatData);
                mPlaybackBufferIndex += numFrames;
            } else {
                std::fill(floatData, floatData + numFrames, 0); // Fill with silence if the buffer is exhausted
            }
        }
        return oboe::DataCallbackResult::Continue;
    }

    bool startRecording() {
        oboe::AudioStreamBuilder builder;
        builder.setDirection(oboe::Direction::Input)
                ->setFormat(oboe::AudioFormat::Float)
                ->setChannelCount(oboe::ChannelCount::Mono)
                ->setSampleRate(oboe::kUnspecified)
                ->setCallback(this);

        oboe::Result result = builder.openStream(&mRecordingStream);
        if (result != oboe::Result::OK) {
            LOGE("Failed to open recording stream. Error: %s", oboe::convertToText(result));
            return false;
        }

        result = mRecordingStream->requestStart();
        if (result != oboe::Result::OK) {
            LOGE("Failed to start recording stream. Error: %s", oboe::convertToText(result));
            return false;
        }
        return true;
    }

    bool startPlayback() {
        oboe::AudioStreamBuilder builder;
        builder.setDirection(oboe::Direction::Output)
                ->setFormat(oboe::AudioFormat::Float)
                ->setChannelCount(oboe::ChannelCount::Mono)
                ->setSampleRate(oboe::kUnspecified)
                ->setCallback(this);

        oboe::Result result = builder.openStream(&mPlaybackStream);
        if (result != oboe::Result::OK) {
            LOGE("Failed to open playback stream. Error: %s", oboe::convertToText(result));
            return false;
        }

        result = mPlaybackStream->requestStart();
        if (result != oboe::Result::OK) {
            LOGE("Failed to start playback stream. Error: %s", oboe::convertToText(result));
            return false;
        }
        mPlaybackBufferIndex = 0;
        return true;
    }

    void stopRecording() {
        if (mRecordingStream != nullptr) {
            mRecordingStream->requestStop();
            mRecordingStream->close();
            mRecordingStream = nullptr;
        }
    }

    void stopPlayback() {
        if (mPlaybackStream != nullptr) {
            mPlaybackStream->requestStop();
            mPlaybackStream->close();
            mPlaybackStream = nullptr;
        }
    }

private:
    oboe::AudioStream *mRecordingStream = nullptr;
    oboe::AudioStream *mPlaybackStream = nullptr;
    std::vector<float> mRecordingBuffer;
    size_t mPlaybackBufferIndex = 0;
    std::mutex mMutex;
};

static AudioEngine audioEngine;

extern "C"
JNIEXPORT void JNICALL
Java_in_reconv_oboenative_MainActivity_startRecordingNative(JNIEnv *env, jobject instance) {
    audioEngine.startRecording();
}

extern "C"
JNIEXPORT void JNICALL
Java_in_reconv_oboenative_MainActivity_stopRecordingNative(JNIEnv *env, jobject instance) {
    audioEngine.stopRecording();
}

extern "C"
JNIEXPORT void JNICALL
Java_in_reconv_oboenative_MainActivity_startPlayback(JNIEnv *env, jobject instance) {
    audioEngine.startPlayback();
}

extern "C"
JNIEXPORT void JNICALL
Java_in_reconv_oboenative_MainActivity_stopPlayback(JNIEnv *env, jobject instance) {
    audioEngine.stopPlayback();
}
