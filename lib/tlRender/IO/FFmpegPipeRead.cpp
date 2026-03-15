// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/IO/FFmpegPipePrivate.h>

#include <ftk/Core/Format.h>
#include <ftk/Core/LogSystem.h>

#include <atomic>
#include <thread>

namespace tl
{
    namespace ffmpeg_pipe
    {
        struct Read::Private
        {
            Options options;

            struct InfoRequest
            {
                std::promise<IOInfo> promise;
            };

            struct VideoRequest
            {
                OTIO_NS::RationalTime time = invalidTime;
                std::promise<VideoData> promise;
            };

            struct AudioRequest
            {
                OTIO_NS::TimeRange timeRange = invalidTimeRange;
                std::promise<AudioData> promise;
            };

            struct Mutex
            {
                std::list<std::shared_ptr<InfoRequest> > infoRequests;
                std::list<std::shared_ptr<VideoRequest> > videoRequests;
                std::list<std::shared_ptr<AudioRequest> > audioRequests;
                std::mutex mutex;
            };
            Mutex mutex;

            struct Thread
            {
                IOInfo info;
                OTIO_NS::RationalTime videoTime = invalidTime;
                std::shared_ptr<POpen> videoPipe = nullptr;
                std::atomic<bool> running;
                std::condition_variable cv;
                std::thread thread;
            };
            Thread thread;
        };

        void Read::_init(
            const ftk::Path& path,
            const std::vector<ftk::MemFile>& mem,
            const IOOptions& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            IRead::_init(path, mem, options, logSystem);
            FTK_P();

            auto i = options.find("FFmpegPipe/FFmpegPath");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.ffmpegPath;
            }
            i = options.find("FFmpegPipe/FFprobePath");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.ffprobePath;
            }

            p.thread.running = true;
            p.thread.thread = std::thread(
                [this, path]
                {
                    _thread();
                });
        }

        Read::Read() :
            _p(new Private)
        {}

        Read::~Read()
        {
            FTK_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
            for (auto& request : p.mutex.infoRequests)
            {
                request->promise.set_value(IOInfo());
            }
            for (auto& request : p.mutex.videoRequests)
            {
                request->promise.set_value(VideoData());
            }
        }

        std::shared_ptr<Read> Read::create(
            const ftk::Path& path,
            const IOOptions& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, {}, options, logSystem);
            return out;
        }

        std::shared_ptr<Read> Read::create(
            const ftk::Path& path,
            const std::vector<ftk::MemFile>& mem,
            const IOOptions& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, mem, options, logSystem);
            return out;
        }

        std::future<IOInfo> Read::getInfo()
        {
            FTK_P();
            auto request = std::make_shared<Private::InfoRequest>();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.infoRequests.push_back(request);
            }
            p.thread.cv.notify_one();
            return request->promise.get_future();
        }

        std::future<VideoData> Read::readVideo(
            const OTIO_NS::RationalTime& time,
            const IOOptions&)
        {
            FTK_P();
            auto request = std::make_shared<Private::VideoRequest>();
            request->time = time;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.videoRequests.push_back(request);
            }
            p.thread.cv.notify_one();
            return request->promise.get_future();
        }

        std::future<AudioData> Read::readAudio(
            const OTIO_NS::TimeRange& timeRange,
            const IOOptions& options)
        {
            FTK_P();
            auto request = std::make_shared<Private::AudioRequest>();
            request->timeRange = timeRange;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.audioRequests.push_back(request);
            }
            p.thread.cv.notify_one();
            return request->promise.get_future();
        }

        void Read::cancelRequests()
        {
            FTK_P();
            std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
            std::list<std::shared_ptr<Private::VideoRequest> > videoRequests;
            std::list<std::shared_ptr<Private::AudioRequest> > audioRequests;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                infoRequests = std::move(p.mutex.infoRequests);
                videoRequests = std::move(p.mutex.videoRequests);
                audioRequests = std::move(p.mutex.audioRequests);
            }
            for (auto& request : infoRequests)
            {
                request->promise.set_value(IOInfo());
            }
            for (auto& request : videoRequests)
            {
                request->promise.set_value(VideoData());
            }
            for (auto& request : audioRequests)
            {
                request->promise.set_value(AudioData());
            }
        }

        void Read::_thread()
        {
            FTK_P();
            _info();
            p.thread.videoTime = p.thread.info.videoTime.start_time();
            while (p.thread.running)
            {
                std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
                std::shared_ptr<Private::VideoRequest> videoRequest;
                std::shared_ptr<Private::AudioRequest> audioRequest;
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    if (p.thread.cv.wait_for(
                        lock,
                        std::chrono::milliseconds(10),
                        [this]
                        {
                            return
                                !_p->mutex.infoRequests.empty() ||
                                !_p->mutex.videoRequests.empty() ||
                                !_p->mutex.audioRequests.empty();
                        }))
                    {
                        infoRequests = std::move(p.mutex.infoRequests);
                        if (!p.mutex.videoRequests.empty())
                        {
                            videoRequest = p.mutex.videoRequests.front();
                            p.mutex.videoRequests.pop_front();
                        }
                        if (!p.mutex.audioRequests.empty())
                        {
                            audioRequest = p.mutex.audioRequests.front();
                            p.mutex.audioRequests.pop_front();
                        }
                    }
                }

                for (auto& request : infoRequests)
                {
                    request->promise.set_value(p.thread.info);
                }

                if (videoRequest &&
                    !videoRequest->time.strictly_equal(p.thread.videoTime))
                {
                    p.thread.videoTime = videoRequest->time;
                    p.thread.videoPipe.reset();
                }

                if (!p.thread.videoPipe)
                {
                    const std::string cmd = ftk::Format("{0} -v quiet -ss {1} -i \"{2}\" -f rawvideo -pix_fmt rgb24 pipe:1").
                        arg("ffmpeg").
                        arg(p.thread.videoTime.to_seconds()).
                        arg(_path.get());
                    //std::cout << cmd << std::endl;
                    p.thread.videoPipe = std::make_shared<POpen>(cmd, "r");
                }

                if (videoRequest)
                {
                    VideoData video;
                    video.time = videoRequest->time;
                    video.image = ftk::Image::create(p.thread.info.video.front());
                    if (p.thread.videoPipe && p.thread.videoPipe->f())
                    {
                        fread(video.image->getData(), video.image->getByteCount(), 1, p.thread.videoPipe->f());
                    }
                    else
                    {
                        video.image->zero();
                    }
                    videoRequest->promise.set_value(video);                    
                    p.thread.videoTime += OTIO_NS::RationalTime(1.0, p.thread.info.videoTime.duration().rate());
                }

                if (audioRequest)
                {
                    AudioData audio;
                    audio.time = audioRequest->timeRange.start_time();
                    audio.audio = Audio::create(
                        p.thread.info.audio,
                        audioRequest->timeRange.duration().rescaled_to(1.0).value() * p.thread.info.audio.sampleRate);
                    const std::string cmd = ftk::Format("{0} -v quiet -ss {1} -t {2} -i \"{3}\" -f s16le pipe:1").
                        arg("ffmpeg").
                        arg(audioRequest->timeRange.start_time().to_seconds()).
                        arg(audioRequest->timeRange.duration().to_seconds()).
                        arg(_path.get());
                    //std::cout << cmd << std::endl;
                    POpen pipe(cmd, "r");
                    if (pipe.f())
                    {
                        fread(audio.audio->getData(), audio.audio->getByteCount(), 1, pipe.f());
                    }
                    else
                    {
                        audio.audio->zero();
                    }
                    audioRequest->promise.set_value(audio);
                }
            }

            p.thread.videoPipe.reset();
        }

        namespace
        {
            typedef std::pair<int, int> Rational;

            Rational toRational(const std::string& value)
            {
                Rational out;
                const auto s = ftk::split(value, '/');
                if (2 == s.size())
                {
                    out.first = atoi(s[0].c_str());
                    out.second = atoi(s[1].c_str());
                }
                return out;
            }

            double toDouble(const Rational& value)
            {
                return value.second != 0 ?
                    (value.first / static_cast<double>(value.second)) :
                    0.0;
            }
        }

        void Read::_info()
        {
            FTK_P();
            try
            {
                std::string cmd = ftk::Format("{0} -v quiet -print_format json -show_streams -show_format \"{1}\"").
                    arg("ffprobe").
                    arg(_path.get());
                nlohmann::json json = nlohmann::json::parse(POpen(cmd, "r").readAll());
                for (const auto& i : json.items())
                {
                    if ("streams" == i.key())
                    {
                        for (const auto& j : i.value())
                        {
                            auto k = j.find("codec_type");
                            if (k != j.end() && "video" == *k)
                            {
                                ftk::ImageInfo info;
                                info.layout.mirror.y = true;
                                info.type = ftk::ImageType::RGB_U8;

                                k = j.find("width");
                                if (k != j.end())
                                {
                                    info.size.w = *k;
                                }
                                k = j.find("height");
                                if (k != j.end())
                                {
                                    info.size.h = *k;
                                }

                                double rate = 0.0;
                                k = j.find("avg_frame_rate");
                                if (k != j.end())
                                {
                                    rate = toDouble(toRational(*k));
                                }
                                else
                                {
                                    k = j.find("r_frame_rate");
                                    if (k != j.end())
                                    {
                                        rate = toDouble(toRational(*k));
                                    }
                                }

                                size_t frames = 0;
                                k = j.find("nb_frames");
                                if (k != j.end())
                                {
                                    frames = atoi(k->get<std::string>().c_str());
                                }
                                else
                                {
                                    k = j.find("duration");
                                    if (k != j.end())
                                    {
                                        const double d = atof(k->get<std::string>().c_str());
                                        if (d > 0.0 && rate > 0.0)
                                        {
                                            frames = d * rate;
                                        }
                                    }                                    
                                }
                                p.thread.info.videoTime = OTIO_NS::TimeRange(
                                    OTIO_NS::RationalTime(0.0, rate),
                                    OTIO_NS::RationalTime(frames, rate));

                                p.thread.info.video.push_back(info);
                            }
                            else if (k != j.end() && "audio" == *k)
                            {
                                p.thread.info.audio.type = AudioType::S16;

                                k = j.find("sample_rate");
                                if (k != j.end())
                                {
                                    p.thread.info.audio.sampleRate = atoi(k->get<std::string>().c_str());
                                }
                                k = j.find("channels");
                                if (k != j.end())
                                {
                                    p.thread.info.audio.channelCount = *k;
                                }

                                double duration = 0.0;
                                k = j.find("duration");
                                if (k != j.end())
                                {
                                    duration = atof(k->get<std::string>().c_str());
                                }
                                p.thread.info.audioTime = OTIO_NS::TimeRange(
                                    OTIO_NS::RationalTime(0.0, p.thread.info.audio.sampleRate),
                                    OTIO_NS::RationalTime(duration * p.thread.info.audio.sampleRate, p.thread.info.audio.sampleRate));
                            }
                        }
                    }
                }
            }
            catch (const std::exception& e)
            {
                _logSystem.lock()->print("tl::ffmpeg_pipe", e.what(), ftk::LogType::Error);
            }
        }
    }
}
