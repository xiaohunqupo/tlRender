// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Timeline/Timeline.h>

#include <tlRender/IO/Read.h>

#include <ftk/Core/LRUCache.h>

#include <opentimelineio/clip.h>

#include <atomic>
#include <list>
#include <mutex>
#include <thread>

namespace tl
{
    struct Timeline::Private
    {
        bool getVideoInfo(const OTIO_NS::Composable*);
        bool getAudioInfo(const OTIO_NS::Composable*);

        float transitionValue(double frame, double in, double out) const;

        void tick();
        void requests();
        void finishRequests();

        std::shared_ptr<IRead> getRead(
            const OTIO_NS::Clip*,
            const IOOptions&);
        std::future<VideoData> readVideo(
            const OTIO_NS::Clip*,
            const OTIO_NS::RationalTime&,
            const IOOptions&);
        std::future<AudioData> readAudio(
            const OTIO_NS::Clip*,
            const OTIO_NS::TimeRange&,
            const IOOptions&);

        std::shared_ptr<Audio> padAudioToOneSecond(
            const std::shared_ptr<Audio>&,
            double seconds,
            const OTIO_NS::TimeRange&);

        std::weak_ptr<ftk::Context> context;
        std::weak_ptr<ftk::LogSystem> logSystem;
        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> otioTimeline;
        ftk::Path path;
        ftk::Path audioPath;
        Options options;
        ftk::LRUCache<std::string, std::shared_ptr<IRead> > readCache;
        OTIO_NS::TimeRange timeRange = invalidTimeRange;
        IOInfo ioInfo;
        uint64_t requestId = 0;

        struct VideoLayerData
        {
            VideoLayerData() {};
            VideoLayerData(VideoLayerData&&) = default;

            std::future<VideoData> image;
            std::future<VideoData> imageB;
            Transition transition = Transition::None;
            float transitionValue = 0.F;
        };
        struct VideoRequest
        {
            VideoRequest() {};
            VideoRequest(VideoRequest&&) = default;

            uint64_t id = 0;
            OTIO_NS::RationalTime time = invalidTime;
            IOOptions options;
            std::promise<VideoFrame> promise;

            std::vector<VideoLayerData> layerData;
        };

        struct AudioLayerData
        {
            AudioLayerData() {};
            AudioLayerData(AudioLayerData&&) = default;

            double seconds = -1.0;
            OTIO_NS::TimeRange timeRange;
            std::future<AudioData> audio;
        };
        struct AudioRequest
        {
            AudioRequest() {};
            AudioRequest(AudioRequest&&) = default;

            uint64_t id = 0;
            double seconds = -1.0;
            IOOptions options;
            std::promise<AudioFrame> promise;

            std::vector<AudioLayerData> layerData;
        };

        struct Mutex
        {
            std::list<std::shared_ptr<VideoRequest> > videoRequests;
            std::list<std::shared_ptr<AudioRequest> > audioRequests;
            bool stopped = false;
            std::mutex mutex;
        };
        Mutex mutex;
        struct Thread
        {
            std::list<std::shared_ptr<VideoRequest> > videoRequestsInProgress;
            std::list<std::shared_ptr<AudioRequest> > audioRequestsInProgress;
            std::condition_variable cv;
            std::thread thread;
            std::atomic<bool> running;
            std::chrono::steady_clock::time_point logTimer;
        };
        Thread thread;
    };
}
