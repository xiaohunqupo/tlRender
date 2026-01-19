// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Timeline/Player.h>

#include <tlRender/Timeline/Util.h>

#include <tlRender/Core/AudioResample.h>

#if defined(TLRENDER_SDL2)
#include <SDL2/SDL.h>
#endif // TLRENDER_SDL2
#if defined(TLRENDER_SDL3)
#include <SDL3/SDL.h>
#endif // TLRENDER_SDL3

#include <atomic>
#include <mutex>
#include <thread>

namespace tl
{
    struct Player::Private
    {
        OTIO_NS::RationalTime loopPlayback(const OTIO_NS::RationalTime&, bool& looped);

        void clearRequests();
        void clearCache();
        size_t getVideoCacheMax() const;
        size_t getAudioCacheMax() const;
        OTIO_NS::TimeRange getVideoCacheRange(size_t max) const;
        ftk::Range<int64_t> getAudioCacheRange(size_t max) const;
        void cacheUpdate();

        bool hasAudio() const;
        void playbackReset(const OTIO_NS::RationalTime&);
        void audioInit(const std::shared_ptr<ftk::Context>&);
        void audioReset(const OTIO_NS::RationalTime&);
#if defined(TLRENDER_SDL2) || defined(TLRENDER_SDL3)
        void sdlCallback(uint8_t* stream, int len);
#if defined(TLRENDER_SDL2)
        static void sdl2Callback(void* user, Uint8* stream, int len);
#elif defined(TLRENDER_SDL3)
        static void sdl3Callback(void* user, SDL_AudioStream *stream, int additional_amount, int total_amount);
#endif // TLRENDER_SDL2
#endif // TLRENDER_SDL2

        void log();

        std::weak_ptr<ftk::LogSystem> logSystem;
        PlayerOptions playerOptions;
        std::shared_ptr<Timeline> timeline;
        OTIO_NS::TimeRange timeRange = invalidTimeRange;
        IOInfo ioInfo;

        std::shared_ptr<ftk::Observable<double> > speed;
        std::shared_ptr<ftk::Observable<double> > speedMult;
        std::shared_ptr<ftk::Observable<Playback> > playback;
        std::shared_ptr<ftk::Observable<Loop> > loop;
        std::shared_ptr<ftk::Observable<OTIO_NS::RationalTime> > currentTime;
        std::shared_ptr<ftk::Observable<OTIO_NS::RationalTime> > seek;
        std::shared_ptr<ftk::Observable<OTIO_NS::TimeRange> > inOutRange;
        std::shared_ptr<ftk::ObservableList<std::shared_ptr<Timeline> > > compare;
        std::shared_ptr<ftk::Observable<CompareTime> > compareTime;
        std::shared_ptr<ftk::Observable<IOOptions> > ioOptions;
        std::shared_ptr<ftk::Observable<int> > videoLayer;
        std::shared_ptr<ftk::ObservableList<int> > compareVideoLayers;
        std::shared_ptr<ftk::ObservableList<VideoFrame> > currentVideoFrame;
        std::shared_ptr<ftk::Observable<AudioDeviceID> > audioDevice;
        std::shared_ptr<ftk::Observable<float> > volume;
        std::shared_ptr<ftk::Observable<bool> > mute;
        std::shared_ptr<ftk::ObservableList<bool> > channelMute;
        std::shared_ptr<ftk::Observable<double> > audioOffset;
        std::shared_ptr<ftk::ObservableList<AudioFrame> > currentAudioFrame;
        std::shared_ptr<ftk::Observable<PlayerCacheOptions> > cacheOptions;
        std::shared_ptr<ftk::Observable<PlayerCacheInfo> > cacheInfo;
        std::shared_ptr<ftk::ListObserver<AudioDeviceInfo> > audioDevicesObserver;
        std::shared_ptr<ftk::Observer<AudioDeviceInfo> > defaultAudioDeviceObserver;

        int accelerate = 0;
        tl::Playback toggle = tl::Playback::Forward;

        bool audioDevices = false;
        AudioInfo audioInfo;
#if defined(TLRENDER_SDL2)
        int sdlID = 0;
#elif defined(TLRENDER_SDL3)
        SDL_AudioStream* sdlStream = nullptr;
#endif // TLRENDER_SDL2

        std::atomic<bool> running;

        struct PlaybackState
        {
            Playback playback = Playback::Stop;
            OTIO_NS::RationalTime currentTime = invalidTime;
            OTIO_NS::TimeRange inOutRange = invalidTimeRange;
            std::vector<std::shared_ptr<Timeline> > compare;
            CompareTime compareTime = CompareTime::Relative;
            IOOptions ioOptions;
            int videoLayer = 0;
            std::vector<int> compareVideoLayers;
            double audioOffset = 0.0;
            PlayerCacheOptions cacheOptions;

            bool operator == (const PlaybackState&) const;
            bool operator != (const PlaybackState&) const;
        };

        struct Mutex
        {
            PlaybackState state;
            bool clearRequests = false;
            bool clearCache = false;
            CacheDir cacheDir = CacheDir::Forward;
            std::vector<VideoFrame> currentVideoFrame;
            std::vector<AudioFrame> currentAudioFrame;
            PlayerCacheInfo cacheInfo;
            std::mutex mutex;
        };
        Mutex mutex;

        struct Thread
        {
            PlaybackState state;
            CacheDir cacheDir = CacheDir::Forward;
            std::map<OTIO_NS::RationalTime, std::vector<VideoRequest> > videoRequests;
            std::map<OTIO_NS::RationalTime, std::vector<VideoFrame> > videoCache;
            std::map<int64_t, AudioRequest> audioRequests;
            std::chrono::steady_clock::time_point cacheTimer;
            std::chrono::steady_clock::time_point logTimer;
            std::thread thread;
        };
        Thread thread;

        struct AudioState
        {
            Playback playback = Playback::Stop;
            double speed = 0.0;
            float volume = 1.F;
            bool mute = false;
            std::vector<bool> channelMute;
            std::chrono::steady_clock::time_point muteTimeout;
            double audioOffset = 0.0;

            bool operator == (const AudioState&) const;
            bool operator != (const AudioState&) const;
        };

        struct AudioMutex
        {
            AudioState state;
            std::map<int64_t, AudioFrame> cache;
            bool reset = false;
            OTIO_NS::RationalTime start = invalidTime;
            int64_t frame = 0;
            std::mutex mutex;
        };
        AudioMutex audioMutex;

        struct AudioThread
        {
            AudioInfo info;
            int64_t inputFrame = 0;
            int64_t outputFrame = 0;
            std::shared_ptr<AudioResample> resample;
            std::list<std::shared_ptr<Audio> > buffer;
            std::shared_ptr<Audio> silence;
        };
        AudioThread audioThread;

        struct NoAudio
        {
            std::chrono::steady_clock::time_point playbackTimer;
            OTIO_NS::RationalTime start = invalidTime;
        };
        NoAudio noAudio;
    };
}
