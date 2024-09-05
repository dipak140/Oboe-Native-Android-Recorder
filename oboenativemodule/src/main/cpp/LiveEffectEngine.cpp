#include <cassert>
#include <fstream>
#include <cstring>
#include <android/log.h>
#include "LiveEffectEngine.h"

namespace little_endian_io
{
    template <typename Word>
    std::ostream& write_word( std::ostream& outs, Word value, unsigned size = sizeof( Word ) )
    {
        for (; size; --size, value >>= 8)
            outs.put( static_cast <char> (value & 0xFF) );
        return outs;
    }
}
using namespace little_endian_io;

LiveEffectEngine::LiveEffectEngine() {
    assert(mOutputChannelCount == mInputChannelCount);
}

void LiveEffectEngine::setRecordingDeviceId(int32_t deviceId) {
//    mRecordingDeviceId = deviceId;
}

void LiveEffectEngine::setPlaybackDeviceId(int32_t deviceId) {
//    mPlaybackDeviceId = deviceId;
}

bool LiveEffectEngine::isAAudioRecommended() {
    return oboe::AudioStreamBuilder::isAAudioRecommended();
}

bool LiveEffectEngine::setAudioApi(oboe::AudioApi api) {
    if (mIsEffectOn) return false;
    mAudioApi = api;
    return true;
}

bool LiveEffectEngine::setEffectOn(bool isOn) {
    bool success = true;
    if (isOn != mIsEffectOn) {
        if (isOn) {
            // Open audio streams and start recording
            isStreamOpen = openPlaybackStream() == oboe::Result::OK;
            if (isStreamOpen) {
                mIsEffectOn = isOn;
            }
        } else {
            // Stop recording and close streams
            closeStreams();
            mIsEffectOn = isOn;
        }
    }
    return success;
}

// method when wave file headers are written in java itself.
void LiveEffectEngine::startRecordingNative(const char * filePath) {
    this->isRecording = true;
    oboe::AudioStreamBuilder builder;
    builder.setDirection(oboe::Direction::Input);
    builder.setPerformanceMode(oboe::PerformanceMode::None);
    builder.setFormat(oboe::AudioFormat::I16);
    builder.setChannelCount(oboe::ChannelCount::Mono);
    builder.setInputPreset(oboe::InputPreset::Generic);
    builder.setSharingMode(oboe::SharingMode::Exclusive);
    builder.setSampleRate(mSampleRate);
    builder.setSampleRateConversionQuality(oboe::SampleRateConversionQuality::Best);
    builder.setAudioApi(oboe::AudioApi::AAudio);

    // Wave file generating stuff (from https://www.cplusplus.com/forum/beginner/166954/)
    int sampleRate = mSampleRate;
    int bitsPerSample = 16; // multiple of 8
    int numChannels = 1; // 2 for stereo, 1 for mono

    std::ofstream f;
    //const char *path = "/storage/emulated/0/Music/record.wav";
    const char *path = filePath;
    f.open(path, std::ios::binary);

    if (!f.is_open()) {
        __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "Failed to open file %s", path);
        return;
    }
    oboe::Result r = builder.openStream(&stream);
    if (r != oboe::Result::OK) {
        return;
    }

    r = stream->requestStart();
    if (r != oboe::Result::OK) {
        return;
    }

    auto a = stream->getState();
    if (a == oboe::StreamState::Started) {

        constexpr int kMillisecondsToRecord = 20;
        auto requestedFrames = (int32_t) (kMillisecondsToRecord *
                                          (stream->getSampleRate() / oboe::kMillisPerSecond));
        __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "requestedFrames = %d",
                            requestedFrames);

        int16_t mybuffer[requestedFrames];
        constexpr int64_t kTimeoutValue = 3 * oboe::kNanosPerMillisecond;

        int framesRead = 0;
        do {
            auto result = stream->read(mybuffer, requestedFrames, 0);
            if (result != oboe::Result::OK) {
                break;
            }
            framesRead = result.value();
            __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "framesRead = %d",
                                framesRead);
            if (framesRead > 0) {
                break;
            }
        } while (framesRead != 0);

        while (isRecording) {
            auto result = stream->read(mybuffer, requestedFrames, kTimeoutValue * 1000);
            if (result == oboe::Result::OK) {
                auto nbFramesRead = result.value();
                for (int i = 0; i < nbFramesRead; i++) {
                    write_word(f, (int) (mybuffer[i]), 2);
                }
            } else {
                auto error = convertToText(result.error());
                __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "error = %s", error);
            }
        }

        __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "Requesting stop");
        f.close();
    }
}

void LiveEffectEngine::startRecording(const char * filePath) {
    __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "Starting recording natively at %s", filePath);
    this->isRecording = true;
    oboe::AudioStreamBuilder builder;
    builder.setDirection(oboe::Direction::Input);
    builder.setPerformanceMode(oboe::PerformanceMode::None);
    builder.setFormat(oboe::AudioFormat::I16);
    builder.setChannelCount(oboe::ChannelCount::Mono);
    builder.setInputPreset(oboe::InputPreset::VoiceCommunication);
    builder.setSharingMode(oboe::SharingMode::Exclusive);
    builder.setSampleRate(mSampleRate);
    builder.setSampleRateConversionQuality(oboe::SampleRateConversionQuality::Best);
    builder.setAudioApi(oboe::AudioApi::AAudio);
    builder.openStream(mRecordingStream);

    // Wave file generating stuff (from https://www.cplusplus.com/forum/beginner/166954/)
    int sampleRate = mSampleRate;
    int bitsPerSample = 16; // multiple of 8
    int numChannels = 1; // 2 for stereo, 1 for mono

    std::ofstream f;
    //const char *path = "/storage/emulated/0/Music/record.wav";
    const char *path = filePath;
    f.open(path, std::ios::binary);
    // Write the file headers
    f << "RIFF----WAVEfmt ";     // (chunk size to be filled in later)
    write_word( f,     16, 4 );  // no extension data
    write_word( f,      1, 2 );  // PCM - integer samples
    write_word( f,      numChannels, 2 );  // one channel (mono) or two channels (stereo file)
    write_word( f,  mSampleRate, 4 );  // samples per second (Hz)
    //write_word( f, 176400, 4 );  // (Sample Rate * BitsPerSample * Channels) / 8
    write_word( f,(mSampleRate * bitsPerSample * numChannels) / 8, 4 );  // (Sample Rate * BitsPerSample * Channels) / 8
    write_word( f,      4, 2 );  // data block size (size of two integer samples, one for each channel, in bytes)
    write_word( f,     bitsPerSample, 2 );  // number of bits per sample (use a multiple of 8)

    // Write the data chunk header
    size_t data_chunk_pos = f.tellp();
    f << "data----";  // (chunk size to be filled in later)
    // f.flush();

    // Write the audio samples
    constexpr double two_pi = 6.283185307179586476925286766559;
    constexpr double max_amplitude = 32760;  // "volume"

    oboe::Result r = builder.openStream(&stream);
    if (r != oboe::Result::OK) {
        return;
    }

    r = stream->requestStart();
    if (r != oboe::Result::OK) {
        return;
    }

    auto a = stream->getState();
    if (a == oboe::StreamState::Started) {

        constexpr int kMillisecondsToRecord = 20;
        auto requestedFrames = (int32_t) (kMillisecondsToRecord * (stream->getSampleRate() / oboe::kMillisPerSecond));
        __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "requestedFrames = %d", requestedFrames);

        int16_t mybuffer[requestedFrames];
        constexpr int64_t kTimeoutValue = 3 * oboe::kNanosPerMillisecond;

        int framesRead = 0;
        do {
            auto result = stream->read(mybuffer, requestedFrames, 0);
            if (result != oboe::Result::OK) {
                break;
            }
            framesRead = result.value();
            __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "framesRead = %d", framesRead);
            if (framesRead > 0) {
                break;
            }
        } while (framesRead != 0);

        while (isRecording) {
            auto result = stream->read(mybuffer, requestedFrames, kTimeoutValue * 1000);
            if (result == oboe::Result::OK) {
                auto nbFramesRead = result.value();
                for (int i = 0; i < nbFramesRead; i++) {
                    write_word( f, (int)(mybuffer[i]), 2 );
                }
            } else {
                auto error = convertToText(result.error());
                __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "error = %s", error);
            }
        }

        __android_log_print(ANDROID_LOG_INFO, "OboeAudioRecorder", "Requesting stop");

        // (We'll need the final file size to fix the chunk sizes above)
        size_t file_length = f.tellp();

        // Fix the data chunk header to contain the data size
        f.seekp( data_chunk_pos + 4 );
        write_word( f, file_length - data_chunk_pos + 8 );

        // Fix the file header to contain the proper RIFF chunk size, which is (file size - 8) bytes
        f.seekp( 0 + 4 );
        write_word( f, file_length - 8, 4 );
        f.close();
    }
}


void LiveEffectEngine::stopRecording() {
    this->isRecording = false;
    stream->requestStop();
    stream->close();
}


oboe::Result LiveEffectEngine::openPlaybackStream() {
    oboe::AudioStreamBuilder inBuilder, outBuilder;
    setupPlaybackStreamParameters(&outBuilder);
    oboe::Result result = outBuilder.openStream(mPlayStream);
    if (result != oboe::Result::OK) {
        mSampleRate = oboe::kUnspecified;
        return result;
    } else {
        // The input stream needs to run at the same sample rate as the output.
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
//    builder->setDeviceId(mRecordingDeviceId)
    builder->setDirection(oboe::Direction::Input)
            ->setSampleRate(sampleRate)
            ->setChannelCount(mInputChannelCount);
    return setupCommonStreamParameters(builder);
}

oboe::AudioStreamBuilder *LiveEffectEngine::setupPlaybackStreamParameters(
        oboe::AudioStreamBuilder *builder) {
    builder->setDataCallback(this)
            ->setErrorCallback(this)
//            ->setDeviceId(mPlaybackDeviceId)
            ->setDirection(oboe::Direction::Output)
            ->setChannelCount(mOutputChannelCount);
    return setupCommonStreamParameters(builder);
}

oboe::AudioStreamBuilder *LiveEffectEngine::setupCommonStreamParameters(
        oboe::AudioStreamBuilder *builder) {
    builder->setAudioApi(mAudioApi)
            ->setFormat(mFormat)
            ->setFormatConversionAllowed(true)
            ->setPerformanceMode(oboe::PerformanceMode::None);
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
