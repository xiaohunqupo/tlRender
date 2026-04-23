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
            IOInfo info;

            struct InfoRequest
            {
                std::promise<IOInfo> promise;
            };

            struct VideoRequest
            {
                OTIO_NS::RationalTime time = invalidTime;
                IOOptions options;
                std::promise<VideoData> promise;
            };

            struct AudioRequest
            {
                OTIO_NS::TimeRange timeRange = invalidTimeRange;
                IOOptions options;
                std::promise<AudioData> promise;
            };

            struct Mutex
            {
                std::list<std::shared_ptr<InfoRequest> > infoRequests;
                std::list<std::shared_ptr<VideoRequest> > videoRequests;
                std::mutex mutex;
            };
            Mutex mutex;

            struct AudioMutex
            {
                std::list<std::shared_ptr<AudioRequest> > requests;
                std::mutex mutex;
            };
            AudioMutex audioMutex;

            struct Thread
            {
                OTIO_NS::RationalTime time = invalidTime;
                std::shared_ptr<Pipe> pipe = nullptr;
                std::atomic<bool> running;
                std::condition_variable cv;
                std::thread thread;
            };
            Thread thread;

            struct AudioThread
            {
                OTIO_NS::RationalTime time = invalidTime;
                std::shared_ptr<Pipe> pipe = nullptr;
                std::atomic<bool> running;
                std::condition_variable cv;
                std::thread thread;
            };
            AudioThread audioThread;
        };

        void Read::_init(
            const ftk::Path& path,
            const std::vector<ftk::MemFile>& mem,
            const IOOptions& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            IRead::_init(path, mem, options, logSystem);
            FTK_P();
            p.thread.running = true;
            p.thread.thread = std::thread(
                [this, options]
                {
                    FTK_P();

                    // Get the I/O information.
                    _info(options);

                    // Start the audio thread.
                    p.audioThread.running = true;
                    p.audioThread.thread = std::thread(
                        [this]
                        {
                            _audioThread();
                        });

                    // Start the info/video thread.
                    _thread();
                });
        }

        Read::Read() :
            _p(new Private)
        {}

        Read::~Read()
        {
            FTK_P();

            // Stop the thread.
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }

            // Stop the audio thread.
            p.audioThread.running = false;
            if (p.audioThread.thread.joinable())
            {
                p.audioThread.thread.join();
            }

            // Cancel the requests.
            for (auto& request : p.mutex.infoRequests)
            {
                request->promise.set_value(IOInfo());
            }
            for (auto& request : p.mutex.videoRequests)
            {
                request->promise.set_value(VideoData());
            }
            for (auto& request : p.audioMutex.requests)
            {
                request->promise.set_value(AudioData());
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
            const IOOptions& options)
        {
            FTK_P();
            auto request = std::make_shared<Private::VideoRequest>();
            request->time = time;
            request->options = options;
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
            request->options = options;
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                p.audioMutex.requests.push_back(request);
            }
            p.audioThread.cv.notify_one();
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
            }
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                audioRequests = std::move(p.audioMutex.requests);
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

        void Read::_info(const IOOptions& ioOptions)
        {
            FTK_P();
            try
            {
                // Get audio conversion information.
                AudioInfo audioConvertInfo;
                {
                    auto i = ioOptions.find("FFmpeg/AudioChannelCount");
                    if (i != ioOptions.end())
                    {
                        std::stringstream ss(i->second);
                        ss >> audioConvertInfo.channelCount;
                    }
                    i = ioOptions.find("FFmpeg/AudioType");
                    if (i != ioOptions.end())
                    {
                        from_string(i->second, audioConvertInfo.type);
                    }
                    i = ioOptions.find("FFmpeg/AudioSampleRate");
                    if (i != ioOptions.end())
                    {
                        std::stringstream ss(i->second);
                        ss >> audioConvertInfo.sampleRate;
                    }
                }

                // Read JSON information from ffprobe.
                const Options options(ioOptions);
                std::vector<std::string> cmd;
                cmd.push_back(options.ffprobePath);
                cmd.push_back("-v");
                cmd.push_back("quiet");
                cmd.push_back("-print_format");
                cmd.push_back("json");
                cmd.push_back("-show_streams");
                cmd.push_back("-show_format");
                cmd.push_back(_path.get());
                //std::cout << ftk::join(cmd, ' ') << std::endl;
                nlohmann::json json = nlohmann::json::parse(Pipe(cmd).readAll());

                // Parse format information.
                double duration = 0.0;
                std::string timecode;
                std::string timeReference;
                auto i = json.find("format");
                if (i != json.end())
                {
                    // Check for a duration.
                    auto j = i.value().find("duration");
                    if (j != i.value().end())
                    {
                        duration = atoi(j->get<std::string>().c_str());
                    }

                    // Get the metadata tags.
                    j = i.value().find("tags");
                    if (j != i.value().end())
                    {
                        for (const auto& k : j->items())
                        {
                            p.info.tags[k.key()] = k.value();

                            if (ftk::compare(
                                k.key(),
                                "timecode",
                                ftk::CaseCompare::Insensitive))
                            {
                                timecode = k.value();
                            }
                            else if (ftk::compare(
                                k.key(),
                                "time_reference",
                                ftk::CaseCompare::Insensitive))
                            {
                                timeReference = k.value();
                            }
                        }
                    }
                }

                // Parse stream information.
                i = json.find("streams");
                if (i != json.end())
                {
                    // Find the first data stream.
                    for (const auto& j : i.value())
                    {
                        auto k = j.find("codec_type");
                        if (k != j.end() && "data" == *k)
                        {
                            k = j.find("tags");
                            if (k != j.end())
                            {
                                for (const auto& l : k.value().items())
                                {
                                    if (ftk::compare(
                                        l.key(),
                                        "timecode",
                                        ftk::CaseCompare::Insensitive))
                                    {
                                        timecode = l.value();
                                        break;
                                    }
                                }
                            }
                            break;
                        }
                    }

                    // Find the first video stream.
                    double videoRate = 0;
                    for (const auto& j : i.value())
                    {
                        auto k = j.find("codec_type");
                        if (k != j.end() && "video" == *k)
                        {
                            ftk::ImageInfo info;
                            info.layout.mirror.y = true;

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
                            k = j.find("pix_fmt");
                            if (k != j.end())
                            {
                                info.type = toImageType(*k);
                            }
                            if (ftk::ImageType::None == info.type)
                            {
                                info.type = ftk::ImageType::RGB_U8;
                            }

                            k = j.find("avg_frame_rate");
                            if (k != j.end())
                            {
                                videoRate = toDouble(toRational(*k));
                            }
                            else
                            {
                                k = j.find("r_frame_rate");
                                if (k != j.end())
                                {
                                    videoRate = toDouble(toRational(*k));
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
                                    if (d > 0.0 && videoRate > 0.0)
                                    {
                                        frames = d * videoRate;
                                    }
                                }
                                else
                                {
                                    frames = duration * videoRate;
                                }
                            }

                            OTIO_NS::RationalTime startTime(0.0, videoRate);
                            if (!timecode.empty())
                            {
                                opentime::ErrorStatus errorStatus;
                                const OTIO_NS::RationalTime time = OTIO_NS::RationalTime::from_timecode(
                                    timecode,
                                    videoRate,
                                    &errorStatus);
                                if (!opentime::is_error(errorStatus))
                                {
                                    startTime = time.floor();
                                }
                            }
                            p.info.videoTime = OTIO_NS::TimeRange(
                                startTime,
                                OTIO_NS::RationalTime(frames, videoRate));

                            p.info.video.push_back(info);

                            {
                                std::stringstream ss;
                                ss << info.size.w << " " << info.size.h;
                                p.info.tags["Video Resolution"] = ss.str();
                            }
                            {
                                std::stringstream ss;
                                ss.precision(2);
                                ss << std::fixed;
                                ss << info.pixelAspectRatio;
                                p.info.tags["Video Pixel Aspect Ratio"] = ss.str();
                            }
                            {
                                std::stringstream ss;
                                ss << info.type;
                                p.info.tags["Video Pixel Type"] = ss.str();
                            }
                            k = j.find("codec_name");
                            if (k != j.end())
                            {
                                p.info.tags["Video Codec"] = k->get<std::string>();
                            }
                            {
                                std::stringstream ss;
                                ss << p.info.videoTime.start_time().to_timecode();
                                p.info.tags["Video Start Time"] = ss.str();
                            }
                            {
                                std::stringstream ss;
                                ss << p.info.videoTime.duration().to_timecode();
                                p.info.tags["Video Duration"] = ss.str();
                            }
                            {
                                std::stringstream ss;
                                ss.precision(2);
                                ss << std::fixed;
                                ss << videoRate << " FPS";
                                p.info.tags["Video Speed"] = ss.str();
                            }
                            break;
                        }
                    }

                    // Find the first audio stream.
                    for (const auto& j : i.value())
                    {
                        auto k = j.find("codec_type");
                        if (k != j.end() && "audio" == *k)
                        {
                            int fileChannelCount = 0;
                            k = j.find("channels");
                            if (k != j.end())
                            {
                                fileChannelCount = *k;
                            }
                            AudioType fileAudioType = AudioType::None;
                            k = j.find("sample_fmt");
                            if (k != j.end())
                            {
                                fileAudioType = toAudioType(*k);
                            }
                            if (AudioType::None == fileAudioType)
                            {
                                fileAudioType = AudioType::S16;
                            }
                            int fileSampleRate = 0;
                            k = j.find("sample_rate");
                            if (k != j.end())
                            {
                                fileSampleRate = atoi(k->get<std::string>().c_str());
                            }

                            size_t channelCount = fileChannelCount;
                            AudioType audioType = fileAudioType;
                            size_t sampleRate = fileSampleRate;
                            if (audioConvertInfo.isValid())
                            {
                                channelCount = audioConvertInfo.channelCount;
                                audioType = audioConvertInfo.type;
                                sampleRate = audioConvertInfo.sampleRate;
                            }
                            p.info.audio.channelCount = channelCount;
                            p.info.audio.type = audioType;
                            p.info.audio.sampleRate = sampleRate;

                            k = j.find("duration");
                            if (k != j.end())
                            {
                                duration = atof(k->get<std::string>().c_str());
                            }

                            OTIO_NS::RationalTime startTime(0.0, sampleRate);
                            if (!timecode.empty())
                            {
                                opentime::ErrorStatus errorStatus;
                                const OTIO_NS::RationalTime time = OTIO_NS::RationalTime::from_timecode(
                                    timecode,
                                    videoRate,
                                    &errorStatus);
                                if (!opentime::is_error(errorStatus))
                                {
                                    startTime = time.rescaled_to(sampleRate).floor();
                                }
                            }
                            else if (!timeReference.empty())
                            {
                                startTime = OTIO_NS::RationalTime(
                                    std::atoi(timeReference.c_str()),
                                    sampleRate);
                            }

                            p.info.audioTime = OTIO_NS::TimeRange(
                                startTime,
                                OTIO_NS::RationalTime(duration * sampleRate, sampleRate));

                            {
                                std::stringstream ss;
                                ss << static_cast<int>(fileChannelCount);
                                p.info.tags["Audio Channels"] = ss.str();
                            }
                            {
                                std::stringstream ss;
                                ss << fileAudioType;
                                p.info.tags["Audio Type"] = ss.str();
                            }
                            {
                                std::stringstream ss;
                                ss.precision(1);
                                ss << std::fixed;
                                ss << fileSampleRate / 1000.F << "kHz";
                                p.info.tags["Audio Sample Rate"] = ss.str();
                            }
                            {
                                std::stringstream ss;
                                ss.precision(2);
                                ss << std::fixed;
                                ss << startTime.rescaled_to(1.0).value() << " seconds";
                                p.info.tags["Audio Start Time"] = ss.str();
                            }
                            {
                                std::stringstream ss;
                                ss.precision(2);
                                ss << std::fixed;
                                ss << p.info.audioTime.duration().rescaled_to(1.0).value() << " seconds";
                                p.info.tags["Audio Duration"] = ss.str();
                            }
                            k = j.find("codec_name");
                            if (k != j.end())
                            {
                                p.info.tags["Audio Codec"] = k->get<std::string>();
                            }
                            break;
                        }
                    }
                }
            }
            catch (const std::exception& e)
            {
                _logSystem.lock()->print("tl::Read::ffmpeg_pipe", e.what(), ftk::LogType::Error);
            }
        }

        void Read::_thread()
        {
            FTK_P();
            p.thread.time = p.info.videoTime.start_time();
            IOOptions ioOptions;
            while (p.thread.running)
            {
                std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
                std::shared_ptr<Private::VideoRequest> videoRequest;
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    if (p.thread.cv.wait_for(
                        lock,
                        std::chrono::milliseconds(10),
                        [this]
                        {
                            return
                                !_p->mutex.infoRequests.empty() ||
                                !_p->mutex.videoRequests.empty();
                        }))
                    {
                        infoRequests = std::move(p.mutex.infoRequests);
                        if (!p.mutex.videoRequests.empty())
                        {
                            videoRequest = p.mutex.videoRequests.front();
                            p.mutex.videoRequests.pop_front();
                        }
                    }
                }

                for (auto& request : infoRequests)
                {
                    request->promise.set_value(p.info);
                }

                if (videoRequest &&
                    !videoRequest->time.strictly_equal(p.thread.time))
                {
                    p.thread.time = videoRequest->time;
                    p.thread.pipe.reset();
                }

                if (videoRequest &&
                    !p.info.video.empty() &&
                    (!p.thread.pipe || videoRequest->options != ioOptions))
                {
                    ioOptions = videoRequest->options;
                    try
                    {
                        const Options options(merge(ioOptions, _options));
                        std::vector<std::string> cmd;
                        cmd.push_back(options.ffmpegPath);
                        cmd.push_back("-v");
                        cmd.push_back("quiet");
                        cmd.push_back("-ss");
                        const double s = (p.thread.time - p.info.videoTime.start_time()).to_seconds();
                        cmd.push_back(ftk::Format("{0}").arg(s));
                        cmd.push_back("-i");
                        cmd.push_back(_path.get());
                        cmd.push_back("-f");
                        cmd.push_back("rawvideo");
                        cmd.push_back("-pix_fmt");
                        cmd.push_back(fromImageType(p.info.video.front().type));
                        cmd.push_back("pipe:1");
                        //std::cout << ftk::join(cmd, ' ') << std::endl;
                        p.thread.pipe = std::make_shared<Pipe>(cmd);
                    }
                    catch (const std::exception& e)
                    {
                        _logSystem.lock()->print("tl::Read::ffmpeg_pipe", e.what(), ftk::LogType::Error);
                    }
                }

                if (videoRequest)
                {
                    VideoData video;
                    video.time = videoRequest->time;
                    if (!p.info.video.empty())
                    {
                        video.image = ftk::Image::create(p.info.video.front());
                        if (p.thread.pipe)
                        {
                            try
                            {
                                p.thread.pipe->read(video.image->getData(), video.image->getByteCount());
                            }
                            catch (const std::exception& e)
                            {
                                _logSystem.lock()->print("tl::Read::ffmpeg_pipe", e.what(), ftk::LogType::Error);
                            }
                        }
                        else
                        {
                            video.image->zero();
                        }
                    }
                    videoRequest->promise.set_value(video);
                    p.thread.time += OTIO_NS::RationalTime(1.0, p.info.videoTime.duration().rate());
                }
            }

            p.thread.pipe.reset();
        }

        void Read::_audioThread()
        {
            FTK_P();
            p.audioThread.time = p.info.audioTime.start_time();
            IOOptions ioOptions;
            while (p.audioThread.running)
            {
                std::shared_ptr<Private::AudioRequest> request;
                {
                    std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                    if (p.audioThread.cv.wait_for(
                        lock,
                        std::chrono::milliseconds(10),
                        [this]
                        {
                            return !_p->audioMutex.requests.empty();
                        }))
                    {
                        if (!p.audioMutex.requests.empty())
                        {
                            request = p.audioMutex.requests.front();
                            p.audioMutex.requests.pop_front();
                        }
                    }
                }

                if (request &&
                    !request->timeRange.start_time().strictly_equal(p.audioThread.time))
                {
                    p.audioThread.time = request->timeRange.start_time();
                    p.audioThread.pipe.reset();
                }

                if (request &&
                    (!p.audioThread.pipe || request->options != ioOptions))
                {
                    ioOptions = request->options;
                    try
                    {
                        const Options options(merge(ioOptions, _options));
                        std::vector<std::string> cmd;
                        cmd.push_back(options.ffmpegPath);
                        cmd.push_back("-v");
                        cmd.push_back("quiet");
                        cmd.push_back("-ss");
                        const double s = (p.audioThread.time - p.info.audioTime.start_time()).to_seconds();
                        cmd.push_back(ftk::Format("{0}").arg(s));
                        cmd.push_back("-i");
                        cmd.push_back(_path.get());
                        cmd.push_back("-f");
                        cmd.push_back(fromAudioType(p.info.audio.type));
                        cmd.push_back("pipe:1");
                        //std::cout << ftk::join(cmd, ' ') << std::endl;
                        p.audioThread.pipe = std::make_shared<Pipe>(cmd);
                    }
                    catch (const std::exception& e)
                    {
                        _logSystem.lock()->print("tl::Read::ffmpeg_pipe", e.what(), ftk::LogType::Error);
                    }
                }

                if (request)
                {
                    AudioData audio;
                    audio.time = request->timeRange.start_time();
                    audio.audio = Audio::create(
                        p.info.audio,
                        request->timeRange.duration().rescaled_to(1.0).value() * p.info.audio.sampleRate);
                    if (p.audioThread.pipe)
                    {
                        try
                        {
                            p.audioThread.pipe->read(audio.audio->getData(), audio.audio->getByteCount());
                        }
                        catch (const std::exception& e)
                        {
                            _logSystem.lock()->print("tl::Read::ffmpeg_pipe", e.what(), ftk::LogType::Error);
                        }
                    }
                    else
                    {
                        audio.audio->zero();
                    }
                    request->promise.set_value(audio);
                    p.audioThread.time += request->timeRange.duration();
                }
            }

            p.audioThread.pipe.reset();
        }
    }
}
