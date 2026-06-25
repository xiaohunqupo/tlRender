// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Timeline/Timeline.h>

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
        std::weak_ptr<ftk::Context> context;
        std::weak_ptr<ftk::LogSystem> logSystem;
        std::shared_ptr<ftk::FileIO> fileIO;
        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> otioTimeline;
        std::map<const OTIO_NS::MediaReference*, std::vector<ftk::MemFile> > memFiles;
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
        // The internal, in-flight record for a request: the promise the caller
        // is waiting on plus the per-layer IO futures still being assembled.
        // Distinct from the public tl::VideoRequest (id + future) handed back
        // by getVideo(); this is the worker side of that handle.
        struct PendingVideoRequest
        {
            PendingVideoRequest() {};
            PendingVideoRequest(PendingVideoRequest&&) = default;

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
        struct PendingAudioRequest
        {
            PendingAudioRequest() {};
            PendingAudioRequest(PendingAudioRequest&&) = default;

            uint64_t id = 0;
            double seconds = -1.0;
            IOOptions options;
            std::promise<AudioFrame> promise;

            std::vector<AudioLayerData> layerData;
        };

        // Shared between the main thread and the request thread; every field
        // is guarded by mutex. The request queues are filled by the main
        // thread (getVideo/getAudio, cancelRequests) and drained by the
        // request thread (_requests). stopped is set by the request thread at
        // shutdown and read by the main thread to reject late requests.
        struct Mutex
        {
            std::list<std::shared_ptr<PendingVideoRequest> > videoRequests;
            std::list<std::shared_ptr<PendingAudioRequest> > audioRequests;
            bool stopped = false;
            std::mutex mutex;
        };
        Mutex mutex;
        // Owned by the request thread; no locking. The in-progress lists hold
        // requests whose IO futures are outstanding. thread and running are the
        // exceptions: the main thread starts the thread (in _init) and clears
        // running (in ~Timeline) to ask it to stop; running is atomic for that
        // handoff.
        struct Thread
        {
            std::list<std::shared_ptr<PendingVideoRequest> > videoRequestsInProgress;
            std::list<std::shared_ptr<PendingAudioRequest> > audioRequestsInProgress;
            std::condition_variable cv;
            std::thread thread;
            std::atomic<bool> running;
            std::chrono::steady_clock::time_point logTimer;
        };
        Thread thread;

        // Build a finished frame from a request whose futures are ready.
        // Calling these blocks on the layer futures via get(), so callers
        // must ensure readiness (poll with wait_for, or accept the block at
        // shutdown).
        VideoFrame videoFrame(PendingVideoRequest&);
        AudioFrame audioFrame(PendingAudioRequest&);
        std::shared_ptr<Audio> padAudioToOneSecond(
            const std::shared_ptr<Audio>&,
            double seconds,
            const OTIO_NS::TimeRange&);
    };
}
