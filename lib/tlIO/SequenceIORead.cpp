// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/SequenceIOReadPrivate.h>

#include <feather-tk/core/Format.h>
#include <feather-tk/core/LogSystem.h>

#include <cstring>
#include <sstream>

namespace tl
{
    namespace io
    {
        namespace
        {
            const std::chrono::milliseconds requestTimeout(5);
        }

        void ISequenceRead::_init(
            const file::Path& path,
            const std::vector<feather_tk::InMemoryFile>& memory,
            const Options& options,
            const std::shared_ptr<feather_tk::LogSystem>& logSystem)
        {
            IRead::_init(path, memory, options, logSystem);
            FEATHER_TK_P();

            const std::string& number = path.getNumber();
            if (!number.empty())
            {
                if (!_memory.empty())
                {
                    std::stringstream ss(number);
                    ss >> _startFrame;
                    _endFrame = _startFrame + _memory.size() - 1;
                }
                else
                {
                    const feather_tk::RangeI& sequence = path.getSequence();
                    _startFrame = sequence.min();
                    _endFrame = sequence.max();
                }
            }

            auto i = options.find("SequenceIO/ThreadCount");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.threadCount;
            }
            i = options.find("SequenceIO/DefaultSpeed");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> _defaultSpeed;
            }

            p.thread.running = true;
            p.thread.thread = std::thread(
                [this, path]
                {
                    FEATHER_TK_P();
                    try
                    {
                        p.info = _getInfo(
                            path.get(-1, file::PathType::Path),
                            !_memory.empty() ? &_memory[0] : nullptr);
                        p.addTags(p.info);
                        _thread();
                    }
                    catch (const std::exception& e)
                    {
                        //! \todo How should this be handled?
                        if (auto logSystem = _logSystem.lock())
                        {
                            logSystem->print(
                                "tl::io::ISequenceRead",
                                e.what(),
                                feather_tk::LogType::Error);
                        }
                    }
                    _finishRequests();
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        p.mutex.stopped = true;
                    }
                    _cancelRequests();
                });
        }

        ISequenceRead::ISequenceRead() :
            _p(new Private)
        {}

        ISequenceRead::~ISequenceRead()
        {}

        std::future<Info> ISequenceRead::getInfo()
        {
            FEATHER_TK_P();
            auto request = std::make_shared<Private::InfoRequest>();
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.infoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(Info());
            }
            return future;
        }

        std::future<VideoData> ISequenceRead::readVideo(
            const OTIO_NS::RationalTime& time,
            const Options& options)
        {
            FEATHER_TK_P();
            auto request = std::make_shared<Private::VideoRequest>();
            request->time = time;
            request->options = merge(options, _options);
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.videoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(VideoData());
            }
            return future;
        }

        void ISequenceRead::cancelRequests()
        {
            _cancelRequests();
        }

        void ISequenceRead::_finish()
        {
            FEATHER_TK_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
        }

        void ISequenceRead::_thread()
        {
            FEATHER_TK_P();
            p.thread.logTimer = std::chrono::steady_clock::now();
            while (p.thread.running)
            {
                // Check requests.
                std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
                std::list<std::shared_ptr<Private::VideoRequest> > videoRequests;
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    if (p.thread.cv.wait_for(
                        lock,
                        requestTimeout,
                        [this]
                        {
                            return
                                !_p->mutex.infoRequests.empty() ||
                                !_p->mutex.videoRequests.empty() ||
                                !_p->thread.videoRequestsInProgress.empty();
                        }))
                    {
                        infoRequests = std::move(p.mutex.infoRequests);
                        while (!p.mutex.videoRequests.empty() &&
                            (p.thread.videoRequestsInProgress.size() + videoRequests.size()) < p.threadCount)
                        {
                            videoRequests.push_back(p.mutex.videoRequests.front());
                            p.mutex.videoRequests.pop_front();
                        }
                    }
                }

                // Information rquests.
                for (const auto& request : infoRequests)
                {
                    request->promise.set_value(p.info);
                }

                // Initialize video requests.
                while (!videoRequests.empty())
                {
                    auto request = videoRequests.front();
                    videoRequests.pop_front();

                    bool seq = false;
                    std::string fileName;
                    if (!_path.getNumber().empty())
                    {
                        seq = true;
                        fileName = _path.get(
                            static_cast<int>(request->time.value()),
                            file::PathType::Path);
                    }
                    else
                    {
                        fileName = _path.get(-1, file::PathType::Path);
                    }
                    const OTIO_NS::RationalTime time = request->time;
                    const Options options = request->options;
                    request->future = std::async(
                        std::launch::async,
                        [this, seq, fileName, time, options]
                        {
                            VideoData out;
                            try
                            {
                                const int64_t frame = time.value();
                                const int64_t memoryIndex = seq ? (frame - _startFrame) : 0;
                                out = _readVideo(
                                    fileName,
                                    memoryIndex >= 0 && memoryIndex < _memory.size() ? &_memory[memoryIndex] : nullptr,
                                    time,
                                    options);
                            }
                            catch (const std::exception&)
                            {
                                //! \todo How should this be handled?
                            }
                            return out;
                        });
                    p.thread.videoRequestsInProgress.push_back(request);
                }

                // Check for finished video requests.
                //if (!p.thread.videoRequestsInProgress.empty())
                //{
                //    std::cout << "in progress: " << p.thread.videoRequestsInProgress.size() << std::endl;
                //}
                auto requestIt = p.thread.videoRequestsInProgress.begin();
                while (requestIt != p.thread.videoRequestsInProgress.end())
                {
                    if ((*requestIt)->future.valid() &&
                        (*requestIt)->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        //std::cout << "finished: " << requestIt->time << std::endl;
                        auto videoData = (*requestIt)->future.get();
                        (*requestIt)->promise.set_value(videoData);
                        requestIt = p.thread.videoRequestsInProgress.erase(requestIt);
                        continue;
                    }
                    ++requestIt;
                }

                // Logging.
                if (auto logSystem = _logSystem.lock())
                {
                    const auto now = std::chrono::steady_clock::now();
                    const std::chrono::duration<float> diff = now - p.thread.logTimer;
                    if (diff.count() > 10.F)
                    {
                        p.thread.logTimer = now;
                        const std::string id = feather_tk::Format("tl::io::ISequenceRead {0}").arg(this);
                        size_t requestsSize = 0;
                        {
                            std::unique_lock<std::mutex> lock(p.mutex.mutex);
                            requestsSize = p.mutex.videoRequests.size();
                        }
                        logSystem->print(id, feather_tk::Format(
                            "\n"
                            "    Path: {0}\n"
                            "    Requests: {1}, {2} in progress\n"
                            "    Thread count: {3}").
                            arg(_path.get()).
                            arg(requestsSize).
                            arg(p.thread.videoRequestsInProgress.size()).
                            arg(p.threadCount));
                    }
                }
            }
        }

        void ISequenceRead::_finishRequests()
        {
            FEATHER_TK_P();
            for (auto& request : p.thread.videoRequestsInProgress)
            {
                VideoData data;
                data.time = request->time;
                if (request->future.valid())
                {
                    data = request->future.get();
                }
                request->promise.set_value(data);
            }
            p.thread.videoRequestsInProgress.clear();
        }

        void ISequenceRead::_cancelRequests()
        {
            FEATHER_TK_P();
            std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
            std::list<std::shared_ptr<Private::VideoRequest> > videoRequests;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                infoRequests = std::move(p.mutex.infoRequests);
                videoRequests = std::move(p.mutex.videoRequests);
            }
            for (auto& request : infoRequests)
            {
                request->promise.set_value(Info());
            }
            for (auto& request : videoRequests)
            {
                request->promise.set_value(VideoData());
            }
        }

        void ISequenceRead::Private::addTags(Info& info)
        {
            if (!info.video.empty())
            {
                {
                    std::stringstream ss;
                    ss << info.video[0].size.w << " " << info.video[0].size.h;
                    info.tags["Video Resolution"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss.precision(2);
                    ss << std::fixed;
                    ss << info.video[0].pixelAspectRatio;
                    info.tags["Video Pixel Aspect Ratio"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << info.video[0].type;
                    info.tags["Video Pixel Type"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << info.video[0].videoLevels;
                    info.tags["Video Levels"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << info.videoTime.start_time().to_timecode();
                    info.tags["Video Start Time"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << info.videoTime.duration().to_timecode();
                    info.tags["Video Duration"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss.precision(2);
                    ss << std::fixed;
                    ss << info.videoTime.start_time().rate() << " FPS";
                    info.tags["Video Speed"] = ss.str();
                }
            }
        }
    }
}
