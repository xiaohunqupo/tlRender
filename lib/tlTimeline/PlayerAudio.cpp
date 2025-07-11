// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/PlayerPrivate.h>

#include <tlTimeline/Util.h>

#include <feather-tk/core/Context.h>
#include <feather-tk/core/Format.h>

namespace tl
{
    namespace timeline
    {
        const audio::DeviceID& Player::getAudioDevice() const
        {
            return _p->audioDevice->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<audio::DeviceID> > Player::observeAudioDevice() const
        {
            return _p->audioDevice;
        }

        void Player::setAudioDevice(const audio::DeviceID& value)
        {
            FEATHER_TK_P();
            if (p.audioDevice->setIfChanged(value))
            {
                if (auto context = getContext())
                {
                    p.audioInit(context);
                }
            }
        }

        float Player::getVolume() const
        {
            return _p->volume->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<float> > Player::observeVolume() const
        {
            return _p->volume;
        }

        void Player::setVolume(float value)
        {
            FEATHER_TK_P();
            if (p.volume->setIfChanged(feather_tk::clamp(value, 0.F, 1.F)))
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                p.audioMutex.state.volume = value;
            }
        }

        bool Player::isMuted() const
        {
            return _p->mute->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<bool> > Player::observeMute() const
        {
            return _p->mute;
        }

        void Player::setMute(bool value)
        {
            FEATHER_TK_P();
            if (p.mute->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                p.audioMutex.state.mute = value;
            }
        }

        const std::vector<bool>& Player::getChannelMute() const
        {
            return _p->channelMute->get();
        }

        std::shared_ptr<feather_tk::IObservableList<bool> > Player::observeChannelMute() const
        {
            return _p->channelMute;
        }

        void Player::setChannelMute(const std::vector<bool>& value)
        {
            FEATHER_TK_P();
            if (p.channelMute->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                p.audioMutex.state.channelMute = value;
            }
        }

        double Player::getAudioOffset() const
        {
            return _p->audioOffset->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<double> > Player::observeAudioOffset() const
        {
            return _p->audioOffset;
        }

        void Player::setAudioOffset(double value)
        {
            FEATHER_TK_P();
            if (p.audioOffset->setIfChanged(value))
            {
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    p.mutex.state.audioOffset = value;
                }
                {
                    std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                    p.audioMutex.state.audioOffset = value;
                }
            }
        }

        const std::vector<AudioData>& Player::getCurrentAudio() const
        {
            return _p->currentAudioData->get();
        }

        std::shared_ptr<feather_tk::IObservableList<AudioData> > Player::observeCurrentAudio() const
        {
            return _p->currentAudioData;
        }

        bool Player::Private::hasAudio() const
        {
            bool out = false;
#if defined(TLRENDER_SDL2) || defined(TLRENDER_SDL3)
            out = audioDevices && ioInfo.audio.isValid();
#endif // TLRENDER_SDL2
            return out;
        }

        namespace
        {
#if defined(TLRENDER_SDL2)
            SDL_AudioFormat toSDL(audio::DataType value)
            {
                SDL_AudioFormat out = 0;
                switch (value)
                {
                case audio::DataType::S8: out = AUDIO_S8; break;
                case audio::DataType::S16: out = AUDIO_S16; break;
                case audio::DataType::S32: out = AUDIO_S32; break;
                case audio::DataType::F32: out = AUDIO_F32; break;
                default: break;
                }
                return out;
            }
#elif defined(TLRENDER_SDL3)
            SDL_AudioFormat toSDL(audio::DataType value)
            {
                SDL_AudioFormat out = SDL_AUDIO_UNKNOWN;
                switch (value)
                {
                case audio::DataType::S8: out = SDL_AUDIO_S8; break;
                case audio::DataType::S16: out = SDL_AUDIO_S16; break;
                case audio::DataType::S32: out = SDL_AUDIO_S32; break;
                case audio::DataType::F32: out = SDL_AUDIO_F32; break;
                default: break;
                }
                return out;
            }
#endif // TLRENDER_SDL2

#if defined(TLRENDER_SDL2) || defined(TLRENDER_SDL3)
            //! \todo This is duplicated in AudioSystem.cpp and PlayerAudio.cpp
            audio::DataType fromSDL(SDL_AudioFormat value)
            {
                audio::DataType out = audio::DataType::F32;
                if (SDL_AUDIO_BITSIZE(value) == 8 &&
                    SDL_AUDIO_ISSIGNED(value) &&
                    !SDL_AUDIO_ISFLOAT(value))
                {
                    out = audio::DataType::S8;
                }
                else if (SDL_AUDIO_BITSIZE(value) == 16 &&
                    SDL_AUDIO_ISSIGNED(value) &&
                    !SDL_AUDIO_ISFLOAT(value))
                {
                    out = audio::DataType::S16;
                }
                else if (SDL_AUDIO_BITSIZE(value) == 32 &&
                    SDL_AUDIO_ISSIGNED(value) &&
                    !SDL_AUDIO_ISFLOAT(value))
                {
                    out = audio::DataType::S32;
                }
                else if (SDL_AUDIO_BITSIZE(value) == 32 &&
                    SDL_AUDIO_ISSIGNED(value) &&
                    SDL_AUDIO_ISFLOAT(value))
                {
                    out = audio::DataType::F32;
                }
                return out;
            }
#endif // TLRENDER_SDL2
        }

        void Player::Private::audioInit(const std::shared_ptr<feather_tk::Context>& context)
        {
#if defined(TLRENDER_SDL2) || defined(TLRENDER_SDL3)

#if defined(TLRENDER_SDL2)
            if (sdlID > 0)
            {
                SDL_CloseAudioDevice(sdlID);
                sdlID = 0;
            }
#elif defined(TLRENDER_SDL3)
            if (sdlStream)
            {
                SDL_DestroyAudioStream(sdlStream);
                sdlStream = nullptr;
            }
#endif // TLRENDER_SDL2

            audio::DeviceID id = audioDevice->get();
            auto audioSystem = context->getSystem<audio::System>();
            auto devices = audioSystem->getDevices();
            auto i = std::find_if(
                devices.begin(),
                devices.end(),
                [id](const audio::DeviceInfo& value)
                {
                    return id == value.id;
                });
            audioDevices = !devices.empty();
            audioInfo = i != devices.end() ? i->info : audioSystem->getDefaultDevice().info;
            if (audioInfo.isValid())
            {
                {
                    std::stringstream ss;
                    ss << "Opening audio device: " << id.number << " " << id.name << "\n" <<
                        "    buffer frames: " << playerOptions.audioBufferFrameCount << "\n" <<
                        "    channels: " << audioInfo.channelCount << "\n" <<
                        "    data type: " << audioInfo.dataType << "\n" <<
                        "    sample rate: " << audioInfo.sampleRate;
                    context->log("tl::timeline::Player", ss.str());
                }

                // These are OK to modify since the audio thread is stopped.
                audioMutex.reset = true;
                audioMutex.start = currentTime->get();
                audioMutex.frame = 0;
                audioThread.info = audioInfo;
                audioThread.resample.reset();

                SDL_AudioSpec spec;
                spec.freq = audioInfo.sampleRate;
                spec.format = toSDL(audioInfo.dataType);
                spec.channels = audioInfo.channelCount;
#if defined(TLRENDER_SDL2)
                spec.samples = playerOptions.audioBufferFrameCount;
                spec.padding = 0;
                spec.callback = sdl2Callback;
                spec.userdata = this;
                SDL_AudioSpec outSpec;
                sdlID = SDL_OpenAudioDevice(
                    !id.name.empty() ? id.name.c_str() : nullptr,
                    0,
                    &spec,
                    &outSpec,
                    SDL_AUDIO_ALLOW_ANY_CHANGE);
                if (sdlID > 0)
                {
                    audioInfo.channelCount = outSpec.channels;
                    audioInfo.dataType = fromSDL(outSpec.format);
                    audioInfo.sampleRate = outSpec.freq;
#elif defined(TLRENDER_SDL3)
                sdlStream = SDL_OpenAudioDeviceStream(
                    -1 == id.number ? SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK : id.number,
                    &spec,
                    sdl3Callback,
                    this);
                if (sdlStream)
                {
#endif // TLRENDER_SDL2
                    {
                        std::stringstream ss;
                        ss << "Audio device: " << id.number << " " << id.name << "\n" <<
                        "    channels: " << audioInfo.channelCount << "\n" <<
                        "    data type: " << audioInfo.dataType << "\n" <<
                        "    sample rate: " << audioInfo.sampleRate;
                        context->log("tl::timeline::Player", ss.str());
                    }

#if defined(TLRENDER_SDL2)
                    SDL_PauseAudioDevice(sdlID, 0);
#elif defined(TLRENDER_SDL3)
                    SDL_ResumeAudioStreamDevice(sdlStream);
#endif // TLRENDER_SDL2
                }
                else
                {
                    std::stringstream ss;
                    ss << "Cannot open audio device: " << SDL_GetError();
                    context->log("tl::timeline::Player", ss.str(), feather_tk::LogType::Error);
                }
            }
#endif // TLRENDER_SDL2
        }

        void Player::Private::audioReset(const OTIO_NS::RationalTime& time)
        {
            audioMutex.reset = true;
            audioMutex.start = time;
            audioMutex.frame = 0;
        }

#if defined(TLRENDER_SDL2) || defined(TLRENDER_SDL3)
        void Player::Private::sdlCallback(
            uint8_t* outputBuffer,
            int len)
        {
            // Get mutex protected values.
            AudioState state;
            bool reset = false;
            OTIO_NS::RationalTime start = time::invalidTime;
            {
                std::unique_lock<std::mutex> lock(audioMutex.mutex);
                state = audioMutex.state;
                reset = audioMutex.reset;
                audioMutex.reset = false;
                start = audioMutex.start;
                if (reset)
                {
                    audioMutex.frame = 0;
                }
            }
            //std::cout << "playback: " << playback << std::endl;
            //std::cout << "reset: " << reset << std::endl;
            //std::cout << "start: " << start << std::endl;

            // Zero output audio data.
            const audio::Info& outputInfo = audioThread.info;
            const size_t outputSamples = len / outputInfo.getByteCount();
            std::memset(outputBuffer, 0, outputSamples * outputInfo.getByteCount());

            const audio::Info& inputInfo = ioInfo.audio;
            if (state.playback != Playback::Stop && inputInfo.sampleRate > 0)
            {
                // Initialize on reset.
                if (reset)
                {
                    audioThread.inputFrame = 0;
                    audioThread.outputFrame = 0;
                    if (audioThread.resample)
                    {
                        audioThread.resample->flush();
                    }
                    audioThread.buffer.clear();
                }

                // Create the audio resampler.
                if (!audioThread.resample ||
                    (audioThread.resample && audioThread.resample->getInputInfo() != inputInfo))
                {
                    audioThread.resample = audio::AudioResample::create(inputInfo, outputInfo);
                }

                // Fill the audio buffer.
                int64_t copySize = 0;
                const double speedMult = std::max(timeRange.duration().rate() > 0.0 ? (state.speed / timeRange.duration().rate()) : 1.0, 1.0);
                if (getSampleCount(audioThread.buffer) < outputSamples * 2 * speedMult)
                {
                    // Get audio from the cache.
                    int64_t t =
                        start.rescaled_to(inputInfo.sampleRate).value() -
                        OTIO_NS::RationalTime(state.audioOffset, 1.0).rescaled_to(inputInfo.sampleRate).value();
                    if (Playback::Forward == state.playback)
                    {
                        t += audioThread.inputFrame;
                    }
                    else
                    {
                        t -= audioThread.inputFrame;
                    }
                    std::vector<AudioData> audioDataList;
                    {
                        const int64_t seconds = std::floor(t / static_cast<double>(inputInfo.sampleRate));
                        std::unique_lock<std::mutex> lock(audioMutex.mutex);
                        for (int64_t i = seconds - 1; i < seconds + 1; ++i)
                        {
                            AudioData audioData;
                            if (audioMutex.cache.get(i, audioData))
                            {
                                audioDataList.push_back(audioData);
                            }
                        }
                    }
                    copySize = OTIO_NS::RationalTime(
                        outputSamples * 2 * speedMult - static_cast<double>(getSampleCount(audioThread.buffer)),
                        outputInfo.sampleRate).
                        rescaled_to(inputInfo.sampleRate).value();
                    std::vector<std::shared_ptr<audio::Audio> > audioLayers;
                    if (copySize > 0)
                    {
                        audioLayers = audioCopy(
                            inputInfo,
                            audioDataList,
                            state.playback,
                            t,
                            copySize);
                    }
                    if (!audioLayers.empty())
                    {
                        // Mix the audio layers.
                        const auto now = std::chrono::steady_clock::now();
                        if (state.mute || now < state.muteTimeout)
                        {
                            state.volume = 0.F;
                        }
                        auto audio = audio::mix(audioLayers, state.volume, state.channelMute);

                        // Reverse the audio.
                        if (Playback::Reverse == state.playback)
                        {
                            audio = audio::reverse(audio);
                        }

                        // Change the audio speed.
                        if (state.speed != timeRange.duration().rate() && state.speed > 0.0)
                        {
                            audio = audio::changeSpeed(audio, timeRange.duration().rate() / state.speed);
                        }

                        // Resample the audio and add it to the buffer.
                        audioThread.buffer.push_back(audioThread.resample->process(audio));

                        // Update the frame counters.
                        audioThread.inputFrame += audioLayers[0]->getSampleCount();
                        audioThread.outputFrame += audio->getSampleCount();
                    }
                    else
                    {
                        const int64_t frames = OTIO_NS::RationalTime(outputSamples, outputInfo.sampleRate).
                            rescaled_to(inputInfo.sampleRate).value();
                        audioThread.inputFrame += frames;
                        audioThread.outputFrame += frames;
                    }
                }

                // Send the audio data to the device.
                const size_t bufferSampleCount = getSampleCount(audioThread.buffer);
                if (outputSamples <= bufferSampleCount)
                {
                    audio::move(audioThread.buffer, outputBuffer, outputSamples);
                }

                // Update the frame counter.
                {
                    const double speedMult = timeRange.duration().rate() > 0.0 ?
                        (state.speed / timeRange.duration().rate()) :
                        0.0;
                    std::unique_lock<std::mutex> lock(audioMutex.mutex);
                    audioMutex.frame = audioThread.outputFrame * speedMult;
                }
            }
        }

#if defined(TLRENDER_SDL2)
        void Player::Private::sdl2Callback(
            void* userData,
            Uint8* outputBuffer,
            int len)
        {
            auto p = reinterpret_cast<Player::Private*>(userData);
            if (len > 0)
            {
                p->sdlCallback(outputBuffer, len);
            }
        }
#elif defined(TLRENDER_SDL3)
        void Player::Private::sdl3Callback(
            void* userData,
            SDL_AudioStream *stream,
            int additional_amount,
            int total_amount)
        {
            auto p = reinterpret_cast<Player::Private*>(userData);
            if (additional_amount > 0)
            {
                std::vector<uint8_t> buf(additional_amount * p->audioThread.info.getByteCount());
                p->sdlCallback(buf.data(), buf.size());
                SDL_PutAudioStreamData(stream, buf.data(), buf.size());
            }
        }
#endif // TLRENDER_SDL2
#endif // TLRENDER_SDL2
    }
}
