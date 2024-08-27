#include <cassert>
#include <fstream>
#include <cstring>
#include <android/log.h>
#include "LiveEffectEngine.h"

LiveEffectEngine::LiveEffectEngine() {
    assert(mOutputChannelCount == mInputChannelCount);
}

void LiveEffectEngine::setRecordingDeviceId(int32_t deviceId) {
    mRecordingDeviceId = deviceId;
}

void LiveEffectEngine::setPlaybackDeviceId(int32_t deviceId) {
    mPlaybackDeviceId = deviceId;
}

bool LiveEffectEngine::isAAudioRecommended() {
    return oboe::AudioStreamBuilder::isAAudioRecommended();
}

bool LiveEffectEngine::setAudioApi(oboe::AudioApi api) {
    if (mIsEffectOn) return false;
    mAudioApi = api;
    return true;
}

bool LiveEffectEngine::setEffectOn(bool isOn, const char *fullPathToFile) {
    bool success = true;
    if (isOn != mIsEffectOn) {
        if (isOn) {
            // Open audio streams and start recording
            success = openStreams(fullPathToFile) == oboe::Result::OK;
            if (success) {
                startRecording(fullPathToFile, mSampleRate, mInputChannelCount);
                mIsEffectOn = isOn;
            }
        } else {
            // Stop recording and close streams
            stopRecording();
            closeStreams();
            mIsEffectOn = isOn;
        }
    }
    return success;
}

void LiveEffectEngine::startRecording(const std::string& filePath, int32_t sampleRate, int16_t numChannels) {
    mWavFilePath = filePath;
    mWavFileSampleRate = sampleRate;
    mWavFileNumChannels = numChannels;

    // Open the file in binary mode
    mWavFile.open(mWavFilePath, std::ios::binary);
    if (mWavFile.is_open()) {
        // Write the WAV header placeholder
        writeWavHeaderPlaceholder();
        mIsRecording = true;
    } else {
        __android_log_print(ANDROID_LOG_ERROR, "LiveEffectEngine", "Failed to open WAV file: %s", mWavFilePath.c_str());
    }
}

void LiveEffectEngine::stopRecording() {
    if (mRecordingStream) {
        mRecordingStream->requestStop();
        mRecordingStream->close();
    }

    if (mIsRecording && mWavFile.is_open()) {
        writeWavHeader(); // Write the final WAV header with correct sizes

        // Close the WAV file
        mWavFile.close();
        mIsRecording = false;
    }
}

void LiveEffectEngine::writeWavHeader() {
    if (!mWavFile.is_open()) {
        return;
    }

    // File size
    std::streampos fileSize = mWavFile.tellp();

    // Go to the beginning and write the correct header
    mWavFile.seekp(0, std::ios::beg);

    // RIFF header
    mWavFile.write("RIFF", 4);
    uint32_t chunkSize = static_cast<uint32_t>(fileSize) - 8;
    mWavFile.write(reinterpret_cast<const char*>(&chunkSize), 4);
    mWavFile.write("WAVE", 4);

    // fmt sub-chunk
    mWavFile.write("fmt ", 4);
    uint32_t subChunk1Size = 16; // For PCM
    mWavFile.write(reinterpret_cast<const char*>(&subChunk1Size), 4);
    uint16_t audioFormat = 1; // PCM = 1
    mWavFile.write(reinterpret_cast<const char*>(&audioFormat), 2);
    mWavFile.write(reinterpret_cast<const char*>(&mWavFileNumChannels), 2);
    mWavFile.write(reinterpret_cast<const char*>(&mWavFileSampleRate), 4);
    uint32_t byteRate = mWavFileSampleRate * mWavFileNumChannels * sizeof(int16_t);
    mWavFile.write(reinterpret_cast<const char*>(&byteRate), 4);
    uint16_t blockAlign = mWavFileNumChannels * sizeof(int16_t);
    mWavFile.write(reinterpret_cast<const char*>(&blockAlign), 2);
    uint16_t bitsPerSample = 16; // Assuming 16-bit PCM
    mWavFile.write(reinterpret_cast<const char*>(&bitsPerSample), 2);

    // data sub-chunk
    mWavFile.write("data", 4);
    uint32_t subChunk2Size = static_cast<uint32_t>(fileSize) - 44;
    mWavFile.write(reinterpret_cast<const char*>(&subChunk2Size), 4);
}


void LiveEffectEngine::writeWavHeaderPlaceholder() {
    char header[44] = { 0 };
    mWavFile.write(header, 44);
}

oboe::Result LiveEffectEngine::openStreams(const char *fullPathToFile) {
    oboe::AudioStreamBuilder inBuilder, outBuilder;
    setupPlaybackStreamParameters(&outBuilder);
    oboe::Result result = outBuilder.openStream(mPlayStream);
    if (result != oboe::Result::OK) {
        mSampleRate = 44100; // Fallback to a standard sample rate
        return result;
    } else {
        mSampleRate = mPlayStream->getSampleRate();
    }
    warnIfNotLowLatency(mPlayStream);

    setupRecordingStreamParameters(&inBuilder, mSampleRate);
    result = inBuilder.openStream(mRecordingStream);
    if (result != oboe::Result::OK) {
        closeStream(mPlayStream);
        return result;
    }
    warnIfNotLowLatency(mRecordingStream);

    mDuplexStream = std::make_unique<FullDuplexPass>();
    mDuplexStream->setSharedInputStream(mRecordingStream);
    mDuplexStream->setSharedOutputStream(mPlayStream);
    mDuplexStream->start();

    return result;
}

oboe::AudioStreamBuilder *LiveEffectEngine::setupRecordingStreamParameters(
        oboe::AudioStreamBuilder *builder, int32_t sampleRate) {
    builder->setDeviceId(mRecordingDeviceId)
            ->setDirection(oboe::Direction::Input)
            ->setSampleRate(sampleRate)
            ->setChannelCount(mInputChannelCount);
    return setupCommonStreamParameters(builder);
}

oboe::AudioStreamBuilder *LiveEffectEngine::setupPlaybackStreamParameters(
        oboe::AudioStreamBuilder *builder) {
    builder->setDataCallback(this)
            ->setErrorCallback(this)
            ->setDeviceId(mPlaybackDeviceId)
            ->setDirection(oboe::Direction::Output)
            ->setChannelCount(mOutputChannelCount);
    return setupCommonStreamParameters(builder);
}

oboe::AudioStreamBuilder *LiveEffectEngine::setupCommonStreamParameters(
        oboe::AudioStreamBuilder *builder) {
    builder->setAudioApi(mAudioApi)
            ->setFormat(mFormat)
            ->setFormatConversionAllowed(true)
            ->setPerformanceMode(oboe::PerformanceMode::LowLatency);
    return builder;
}

void LiveEffectEngine::closeStreams() {
    if (mDuplexStream) {
        mDuplexStream->stop();
    }
    closeStream(mPlayStream);
    closeStream(mRecordingStream);
    mDuplexStream.reset();
}

void LiveEffectEngine::closeStream(std::shared_ptr<oboe::AudioStream> &stream) {
    if (stream) {
        if (stream->getState() != oboe::StreamState::Closed) {
            oboe::Result result = stream->stop();
            if (result != oboe::Result::OK && result != oboe::Result::ErrorClosed) {
                __android_log_print(ANDROID_LOG_ERROR, "LiveEffectEngine", "Error stopping stream: %s", oboe::convertToText(result));
            }
            result = stream->close();
            if (result != oboe::Result::OK && result != oboe::Result::ErrorClosed) {
                __android_log_print(ANDROID_LOG_ERROR, "LiveEffectEngine", "Error closing stream: %s", oboe::convertToText(result));
            }
        }
        stream.reset();
    }
}


oboe::DataCallbackResult LiveEffectEngine::onAudioReady(
        oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) {
    float *floatData = static_cast<float*>(audioData);

    if (mIsRecording && mWavFile.is_open()) {
        for (int i = 0; i < numFrames * mOutputChannelCount; ++i) {
            int16_t pcmSample = static_cast<int16_t>(floatData[i] * 32767.0f);
            mWavFile.write(reinterpret_cast<const char*>(&pcmSample), sizeof(pcmSample));
        }
    }

    // Apply volume and pass data to the playback stream
    for (int i = 0; i < numFrames * mOutputChannelCount; ++i) {
        floatData[i] *= mVolume;
    }
    return mDuplexStream->onAudioReady(oboeStream, audioData, numFrames);
}

void LiveEffectEngine::warnIfNotLowLatency(std::shared_ptr<oboe::AudioStream> &stream) {
    if (stream->getPerformanceMode() != oboe::PerformanceMode::LowLatency) {
        __android_log_print(ANDROID_LOG_WARN, "LiveEffectEngine", "Stream is NOT low latency.");
    }
}

void LiveEffectEngine::setVolume(float volume) {
    mVolume = volume;
}
