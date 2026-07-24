// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/TimelineTest/TimelineTest.h>

#include <tlRender/Timeline/Timeline.h>
#include <tlRender/Timeline/Util.h>

#include <tlRender/IO/System.h>

#include <ftk/Core/Assert.h>
#include <ftk/Core/Format.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/timeline.h>

namespace tl
{
    namespace timeline_tests
    {
        TimelineTest::TimelineTest(const std::shared_ptr<ftk::Context>& context) :
            ITest(context, "timeline_tests::TimelineTest")
        {}

        std::shared_ptr<TimelineTest> TimelineTest::create(const std::shared_ptr<ftk::Context>& context)
        {
            return std::shared_ptr<TimelineTest>(new TimelineTest(context));
        }

        void TimelineTest::run()
        {
            _enums();
            _options();
            _util();
            _transitions();
            _videoData();
            _timeline();
            _separateAudio();
            _spatial();
        }

        void TimelineTest::_spatial()
        {
            // The fixtures pair a 1920x1080 clip with a 640x360 one. The
            // expected boxes are what each set of OTIO spatial coordinates
            // works out to in image space, after being scaled from unit-less
            // coordinates to pixels, flipped from Y-up to Y-down, and moved so
            // the canvas starts at the origin.
            const ftk::Box2F full(ftk::V2F(0.F, 0.F), ftk::V2F(1920.F, 1080.F));
            struct Case
            {
                std::string fileName;
                ftk::Size2I canvasSize;
                ftk::Box2F first;
                ftk::Box2F second;
            };
            const std::vector<Case> cases =
            {
                // The same box written three different ways. All three must
                // give the same result, and both clips must fill the canvas
                // whatever resolution they were rendered at.
                { "SpatialCoordinates.otio", ftk::Size2I(1920, 1080), full, full },
                { "SpatialCoordinatesUnits.otio", ftk::Size2I(1920, 1080), full, full },
                { "SpatialCoordinatesCentered.otio", ftk::Size2I(1920, 1080), full, full },

                // Side by side on a wider canvas.
                { "SpatialCoordinatesSideBySide.otio", ftk::Size2I(3840, 1080),
                  full,
                  ftk::Box2F(ftk::V2F(1920.F, 0.F), ftk::V2F(3840.F, 1080.F)) },

                // OTIO is Y-up, so "Upper" ends up at the top of the canvas
                // and "Lower", the first clip, at the bottom.
                { "SpatialCoordinatesOffsetY.otio", ftk::Size2I(1920, 2160),
                  ftk::Box2F(ftk::V2F(0.F, 1080.F), ftk::V2F(1920.F, 2160.F)),
                  full }
            };

            for (const auto& i : cases)
            {
                try
                {
                    const ftk::Path path(TLRENDER_SAMPLE_DATA, i.fileName);
                    _print(ftk::Format("Path: {0}").arg(path.get()));
                    auto timeline = Timeline::create(_context, path);

                    // The clips are 24 frames each, so these land one in each.
                    const std::vector<std::pair<double, ftk::Box2F> > frames =
                    {
                        { 0.0, i.first },
                        { 30.0, i.second }
                    };
                    for (const auto& frame : frames)
                    {
                        auto request = timeline->getVideo(
                            OTIO_NS::RationalTime(frame.first, 24.0));
                        const VideoFrame videoFrame = request.future.get();
                        FTK_ASSERT(i.canvasSize == videoFrame.canvasSize);
                        FTK_ASSERT(!videoFrame.layers.empty());
                        FTK_ASSERT(videoFrame.layers[0].bounds.has_value());
                        FTK_ASSERT(frame.second == videoFrame.layers[0].bounds.value());
                    }
                }
                catch (const std::exception& e)
                {
                    _error(e.what());
                }
            }

            // Timelines without spatial coordinates are laid out from the
            // image sizes as before.
            try
            {
                const ftk::Path path(TLRENDER_SAMPLE_DATA, "MultipleClips.otio");
                _print(ftk::Format("Path: {0}").arg(path.get()));
                auto timeline = Timeline::create(_context, path);
                auto request = timeline->getVideo(OTIO_NS::RationalTime(0.0, 24.0));
                const VideoFrame videoFrame = request.future.get();
                FTK_ASSERT(!videoFrame.canvasSize.isValid());
                for (const auto& layer : videoFrame.layers)
                {
                    FTK_ASSERT(!layer.bounds.has_value());
                }
            }
            catch (const std::exception& e)
            {
                _error(e.what());
            }

            // The pixels per unit must come from the first clip that has
            // spatial coordinates, not simply the first clip. Taking it from a
            // clip without them leaves the scale at 1 and reads the unit-less
            // coordinates as pixels, which gives a canvas of a few pixels.
            try
            {
                const ftk::Path path(TLRENDER_SAMPLE_DATA, "SpatialCoordinatesSecondClip.otio");
                _print(ftk::Format("Path: {0}").arg(path.get()));
                auto timeline = Timeline::create(_context, path);
                auto request = timeline->getVideo(OTIO_NS::RationalTime(30.0, 24.0));
                const VideoFrame videoFrame = request.future.get();
                FTK_ASSERT(ftk::Size2I(1920, 1080) == videoFrame.canvasSize);
                FTK_ASSERT(!videoFrame.layers.empty());
                FTK_ASSERT(videoFrame.layers[0].bounds.has_value());
                FTK_ASSERT(full == videoFrame.layers[0].bounds.value());
            }
            catch (const std::exception& e)
            {
                _error(e.what());
            }

            // Spatial::None ignores the coordinates even when they are there.
            try
            {
                const ftk::Path path(TLRENDER_SAMPLE_DATA, "SpatialCoordinates.otio");
                _print(ftk::Format("Path: {0}").arg(path.get()));
                Options options;
                options.spatial = Spatial::None;
                auto timeline = Timeline::create(_context, path, options);
                auto request = timeline->getVideo(OTIO_NS::RationalTime(0.0, 24.0));
                const VideoFrame videoFrame = request.future.get();
                FTK_ASSERT(!videoFrame.canvasSize.isValid());
                for (const auto& layer : videoFrame.layers)
                {
                    FTK_ASSERT(!layer.bounds.has_value());
                }
            }
            catch (const std::exception& e)
            {
                _error(e.what());
            }

            // Only the first clip of this timeline has spatial coordinates.
            // The default leaves the second clip alone, so it is laid out at
            // its own resolution; Spatial::Normalize gives it the reference
            // size so that both clips are displayed at the same size.
            try
            {
                const ftk::Path path(TLRENDER_SAMPLE_DATA, "SpatialCoordinatesPartial.otio");
                _print(ftk::Format("Path: {0}").arg(path.get()));
                {
                    auto timeline = Timeline::create(_context, path);
                    auto request = timeline->getVideo(OTIO_NS::RationalTime(30.0, 24.0));
                    const VideoFrame videoFrame = request.future.get();
                    FTK_ASSERT(!videoFrame.layers.empty());
                    FTK_ASSERT(!videoFrame.layers[0].bounds.has_value());
                }
                {
                    Options options;
                    options.spatial = Spatial::Normalize;
                    auto timeline = Timeline::create(_context, path, options);
                    auto request = timeline->getVideo(OTIO_NS::RationalTime(30.0, 24.0));
                    const VideoFrame videoFrame = request.future.get();
                    FTK_ASSERT(ftk::Size2I(1920, 1080) == videoFrame.canvasSize);
                    FTK_ASSERT(!videoFrame.layers.empty());
                    FTK_ASSERT(videoFrame.layers[0].bounds.has_value());
                    FTK_ASSERT(full == videoFrame.layers[0].bounds.value());
                }
            }
            catch (const std::exception& e)
            {
                _error(e.what());
            }
        }

        void TimelineTest::_enums()
        {
            FTK_TEST_ENUM(ImageSeqAudio);
            FTK_TEST_ENUM(Transition);
        }

        void TimelineTest::_options()
        {
            Options a;
            a.imageSeqAudio = ImageSeqAudio::FileName;
            FTK_ASSERT(a == a);
            FTK_ASSERT(a != Options());
        }

        void TimelineTest::_util()
        {
        }

        void TimelineTest::_transitions()
        {
            {
                FTK_ASSERT(toTransition(std::string()) == Transition::None);
                FTK_ASSERT(toTransition("SMPTE_Dissolve") == Transition::Dissolve);
            }
        }

        void TimelineTest::_videoData()
        {
            {
                VideoLayer a, b;
                FTK_ASSERT(a == b);
                a.transition = Transition::Dissolve;
                FTK_ASSERT(a != b);
            }
            {
                VideoData a, b;
                FTK_ASSERT(a == b);
                a.time = OTIO_NS::RationalTime(1.0, 24.0);
                FTK_ASSERT(a != b);
            }
        }

        void TimelineTest::_timeline()
        {
            // Test timelines.
            const std::vector<ftk::Path> paths =
            {
                ftk::Path(TLRENDER_SAMPLE_DATA, "Seq/BART_2021-02-07.0001.jpg"),
#if defined(TLRENDER_FFMPEG_PLUGIN) || defined(TLRENDER_FFMPEG_CMD)
                ftk::Path(TLRENDER_SAMPLE_DATA, "BART_2021-02-07.m4v"),
                ftk::Path(TLRENDER_SAMPLE_DATA, "MovieAndSeq.otio"),
                ftk::Path(TLRENDER_SAMPLE_DATA, "TransitionGap.otio"),
#endif // TLRENDER_FFMPEG_PLUGIN or TLRENDER_FFMPEG_CMD
#if defined(TLRENDER_FFMPEG_PLUGIN)
                ftk::Path(TLRENDER_SAMPLE_DATA, "SingleClip.otioz"),
#endif // TLRENDER_FFMPEG_PLUGIN
                ftk::Path(TLRENDER_SAMPLE_DATA, "SingleClipSeq.otioz")
            };
            for (const auto& path : paths)
            {
                try
                {
                    _print(ftk::Format("Timeline: {0}").arg(path.get()));
                    auto timeline = Timeline::create(_context, path);
                    _timeline(timeline);
                }
                catch (const std::exception& e)
                {
                    _error(e.what());
                }
            }
        }

        void TimelineTest::_timeline(const std::shared_ptr<Timeline>& timeline)
        {
            // Get video from the timeline.
            const OTIO_NS::TimeRange& timeRange = timeline->getTimeRange();
            std::vector<VideoFrame> videoFrame;
            std::vector<VideoRequest> videoRequests;
            for (size_t i = 0; i < static_cast<size_t>(timeRange.duration().value()); ++i)
            {
                videoRequests.push_back(timeline->getVideo(OTIO_NS::RationalTime(i, 24.0)));
            }
            IOOptions ioOptions;
            ioOptions["Layer"] = "1";
            for (size_t i = 0; i < static_cast<size_t>(timeRange.duration().value()); ++i)
            {
                videoRequests.push_back(timeline->getVideo(OTIO_NS::RationalTime(i, 24.0), ioOptions));
            }
            while (videoFrame.size() < static_cast<size_t>(timeRange.duration().value()) * 2)
            {
                auto i = videoRequests.begin();
                while (i != videoRequests.end())
                {
                    if (i->future.valid() &&
                        i->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        videoFrame.push_back(i->future.get());
                        i = videoRequests.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
            }
            FTK_ASSERT(videoRequests.empty());

            // Get audio from the timeline.
            std::vector<AudioFrame> audioFrame;
            std::vector<AudioRequest> audioRequests;
            for (size_t i = 0; i < static_cast<size_t>(timeRange.duration().rescaled_to(1.0).value()); ++i)
            {
                audioRequests.push_back(timeline->getAudio(i));
            }
            while (audioFrame.size() < static_cast<size_t>(timeRange.duration().rescaled_to(1.0).value()))
            {
                auto i = audioRequests.begin();
                while (i != audioRequests.end())
                {
                    if (i->future.valid() &&
                        i->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        audioFrame.push_back(i->future.get());
                        i = audioRequests.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
            }
            FTK_ASSERT(audioRequests.empty());

            // Cancel requests.
            videoFrame.clear();
            videoRequests.clear();
            audioFrame.clear();
            audioRequests.clear();
            for (size_t i = 0; i < static_cast<size_t>(timeRange.duration().value()); ++i)
            {
                videoRequests.push_back(timeline->getVideo(OTIO_NS::RationalTime(i, 24.0)));
            }
            for (size_t i = 0; i < static_cast<size_t>(timeRange.duration().value()); ++i)
            {
                videoRequests.push_back(timeline->getVideo(OTIO_NS::RationalTime(i, 24.0), ioOptions));
            }
            for (size_t i = 0; i < static_cast<size_t>(timeRange.duration().rescaled_to(1.0).value()); ++i)
            {
                audioRequests.push_back(timeline->getAudio(i));
            }
            std::vector<uint64_t> ids;
            for (const auto& i : videoRequests)
            {
                ids.push_back(i.id);
            }
            for (const auto& i : audioRequests)
            {
                ids.push_back(i.id);
            }
            timeline->cancelRequests(ids);
        }

        void TimelineTest::_separateAudio()
        {
#if defined(TLRENDER_FFMPEG_PLUGIN) || defined(TLRENDER_FFMPEG_CMD)
            try
            {
                const ftk::Path path(TLRENDER_SAMPLE_DATA, "Seq/BART_2021-02-07.0001.jpg");
                const ftk::Path audioPath(TLRENDER_SAMPLE_DATA, "Seq/BART_2021-02-07.wav");
                auto timeline = Timeline::create(_context, path.get(), audioPath.get());
            }
            catch (const std::exception& e)
            {
                _error(e.what());
            }
            try
            {
                const ftk::Path path(TLRENDER_SAMPLE_DATA, "Seq/BART_2021-02-07.0001.jpg");
                const ftk::Path audioPath(TLRENDER_SAMPLE_DATA, "Seq/BART_2021-02-07.wav");
                auto timeline = Timeline::create(_context, path, audioPath);
            }
            catch (const std::exception& e)
            {
                _error(e.what());
            }
            try
            {
                const ftk::Path path(TLRENDER_SAMPLE_DATA, "Seq/BART_2021-02-07.0001.jpg");
                _print(ftk::Format("Path: {0}").arg(path.get()));
                Options options;
                options.imageSeqAudio = ImageSeqAudio::None;
                auto timeline = Timeline::create(_context, path, options);
                const ftk::Path& audioPath = timeline->getAudioPath();
                FTK_ASSERT(audioPath.isEmpty());
                _print(ftk::Format("Audio path: {0}").arg(audioPath.get()));
            }
            catch (const std::exception& e)
            {
                _error(e.what());
            }
            try
            {
                const ftk::Path path(TLRENDER_SAMPLE_DATA, "Seq/BART_2021-02-07.0001.jpg");
                _print(ftk::Format("Path: {0}").arg(path.get()));
                Options options;
                options.imageSeqAudio = ImageSeqAudio::Ext;
                auto timeline = Timeline::create(_context, path, options);
                const ftk::Path& audioPath = timeline->getAudioPath();
                FTK_ASSERT(!audioPath.isEmpty());
                _print(ftk::Format("Audio path: {0}").arg(audioPath.get()));
            }
            catch (const std::exception& e)
            {
                _error(e.what());
            }
            try
            {
                const ftk::Path path(TLRENDER_SAMPLE_DATA, "Seq/BART_2021-02-07.0001.jpg");
                _print(ftk::Format("Path: {0}").arg(path.get()));
                Options options;
                options.imageSeqAudio = ImageSeqAudio::FileName;
                options.imageSeqAudioFileName = ftk::Path(
                    TLRENDER_SAMPLE_DATA, "AudioToneStereo.wav").get();
                auto timeline = Timeline::create(_context, path, options);
                const ftk::Path& audioPath = timeline->getAudioPath();
                FTK_ASSERT(!audioPath.isEmpty());
                _print(ftk::Format("Audio path: {0}").arg(audioPath.get()));
            }
            catch (const std::exception& e)
            {
                _error(e.what());
            }
#endif // TLRENDER_FFMPEGPLUGIN or TLRENDER_FFMPEG_CMD
        }
    }
}
