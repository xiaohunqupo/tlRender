// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/Timeline/TimelinePrivate.h>

#include <tlRender/Timeline/Util.h>
#include <tlRender/Timeline/ZipPrivate.h>

#include <tlRender/IO/System.h>

#include <tlRender/Core/URL.h>

#include <ftk/Core/Assert.h>
#include <ftk/Core/Context.h>
#include <ftk/Core/Format.h>
#include <ftk/Core/LogSystem.h>
#include <ftk/Core/Time.h>

#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/transition.h>

namespace tl
{
    namespace
    {
        const size_t readCacheMax = 10;
        const std::chrono::milliseconds timeout(5);

        ftk::Path getAssociatedAudio(
            const std::shared_ptr<ftk::Context>& context,
            const ftk::Path& path,
            const ImageSeqAudio& imageSeqAudio,
            const std::vector<std::string>& imageSeqAudioExts,
            const std::string& imageSeqAudioFileName,
            const ftk::PathOptions& pathOptions)
        {
            ftk::Path out;
            auto ioSystem = context->getSystem<ReadSystem>();
            switch (imageSeqAudio)
            {
            case ImageSeqAudio::Ext:
            {
                // Check for an audio file with the same base name.
                std::vector<std::string> baseNames;
                baseNames.push_back(path.getDir() + path.getBase());
                std::string tmp = path.getBase();
                if (!tmp.empty() && '.' == tmp[tmp.size() - 1])
                {
                    tmp.pop_back();
                }
                baseNames.push_back(path.getDir() + tmp);
                for (const auto& baseName : baseNames)
                {
                    for (const auto& ext : imageSeqAudioExts)
                    {
                        const ftk::Path audioPath(baseName + ext, pathOptions);
                        if (std::filesystem::exists(std::filesystem::u8path(audioPath.get())))
                        {
                            out = audioPath;
                            break;
                        }
                    }
                }

                // Or use the first audio file.
                if (out.isEmpty())
                {
                    ftk::DirListOptions listOptions;
                    listOptions.filterExt = imageSeqAudioExts;
                    const auto entries = ftk::dirList(path.getDir(), listOptions);
                    if (!entries.empty())
                    {
                        out = entries.front().path;
                    }
                }

                break;
            }
            case ImageSeqAudio::FileName:
                out = ftk::Path(path.getDir() + imageSeqAudioFileName, pathOptions);
                break;
            default: break;
            }
            return out;
        }
    }

    void Timeline::_init(
        const std::shared_ptr<ftk::Context>& context,
        const ftk::Path& inputPath,
        const ftk::Path& inputAudioPath,
        const Options& options)
    {
        FTK_P();

        ftk::Path path = inputPath;
        ftk::Path audioPath = inputAudioPath;

        auto logSystem = context->getLogSystem();
        logSystem->print(
            "tl::Timeline::_init",
            ftk::Format(
                "\n"
                "    Path: {0}\n"
                "    Audio path: {1}").
            arg(path.get()).
            arg(audioPath.get()));

        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> otioTimeline;

        // Is the input a sequence?
        const std::vector<std::string> seqExts = getExts(
            context,
            static_cast<int>(FileType::Seq));
        const bool hasSeqExt = std::find(
            seqExts.begin(),
            seqExts.end(),
            ftk::toLower(path.getExt())) != seqExts.end();
        if (hasSeqExt)
        {
            path = ftk::expandSeq(path, options.pathOptions);
        }
        if (hasSeqExt && path.isSeq())
        {
            if (audioPath.isEmpty())
            {
                // Check for an associated audio file.
                audioPath = getAssociatedAudio(
                    context,
                    path,
                    options.imageSeqAudio,
                    options.imageSeqAudioExts,
                    options.imageSeqAudioFileName,
                    options.pathOptions);
            }
        }

        // Read the file.
        auto ioSystem = context->getSystem<ReadSystem>();
        if (auto read = ioSystem->read(path, options.ioOptions))
        {
            const auto info = read->getInfo().get();
            OTIO_NS::RationalTime startTime = invalidTime;
            OTIO_NS::Track* videoTrack = nullptr;
            OTIO_NS::Track* audioTrack = nullptr;

            // Read the video.
            if (!info.video.empty())
            {
                startTime = info.videoTime.start_time();
                auto videoClip = new OTIO_NS::Clip;
                videoClip->set_source_range(info.videoTime);
                if (path.isSeq())
                {
                    auto mediaReference = new OTIO_NS::ImageSequenceReference(
                        "",
                        path.getBase(),
                        path.getExt(),
                        info.videoTime.start_time().value(),
                        1,
                        info.videoTime.duration().rate(),
                        path.getPad());
                    mediaReference->set_available_range(info.videoTime);
                    videoClip->set_media_reference(mediaReference);
                }
                else
                {
                    videoClip->set_media_reference(new OTIO_NS::ExternalReference(
                        path.getFileName(),
                        info.videoTime));
                }
                videoTrack = new OTIO_NS::Track("Video", std::nullopt, OTIO_NS::Track::Kind::video);
                videoTrack->append_child(videoClip);
            }

            // Read the separate audio if provided.
            if (!audioPath.isEmpty())
            {
                if (auto audioRead = ioSystem->read(audioPath, options.ioOptions))
                {
                    const auto audioInfo = audioRead->getInfo().get();

                    auto audioClip = new OTIO_NS::Clip;
                    audioClip->set_source_range(audioInfo.audioTime);
                    audioClip->set_media_reference(new OTIO_NS::ExternalReference(
                        audioPath.getFileName(),
                        audioInfo.audioTime));

                    audioTrack = new OTIO_NS::Track("Audio", std::nullopt, OTIO_NS::Track::Kind::audio);
                    audioTrack->append_child(audioClip);
                }
            }
            else if (info.audio.isValid())
            {
                if (startTime.is_invalid_time())
                {
                    startTime = info.audioTime.start_time();
                }

                auto audioClip = new OTIO_NS::Clip;
                audioClip->set_source_range(info.audioTime);
                audioClip->set_media_reference(new OTIO_NS::ExternalReference(
                    path.getFileName(),
                    info.audioTime));

                audioTrack = new OTIO_NS::Track("Audio", std::nullopt, OTIO_NS::Track::Kind::audio);
                audioTrack->append_child(audioClip);
            }

            // Create the stack.
            auto otioStack = new OTIO_NS::Stack;
            if (videoTrack)
            {
                otioStack->append_child(videoTrack);
            }
            if (audioTrack)
            {
                otioStack->append_child(audioTrack);
            }

            // Create the timeline.
            otioTimeline = new OTIO_NS::Timeline(path.get());
            otioTimeline->set_tracks(otioStack);
            if (isValid(startTime))
            {
                otioTimeline->set_global_start_time(startTime);
            }
        }

        // Is the input an OTIO file?
        if (!otioTimeline)
        {
            const std::string fileName = path.get();
            const std::string ext = ftk::toLower(path.getExt());
            OTIO_NS::ErrorStatus otioError;
            if (".otio" == ext)
            {
                otioTimeline = dynamic_cast<OTIO_NS::Timeline*>(
                    OTIO_NS::Timeline::from_json_file(fileName, &otioError));
                if (!otioTimeline)
                {
                    throw std::runtime_error(
                        ftk::Format("Cannot read timeline: \"{0}\"").
                        arg(path.get()));
                }
                else if (OTIO_NS::is_error(otioError))
                {
                    throw std::runtime_error(
                        ftk::Format("Cannot read timeline: \"{0}\": {1}").
                        arg(path.get()).
                        arg(otioError.details));
                }
            }
            else if (".otioz" == ext)
            {
                p.fileIO = ftk::FileIO::create(fileName, ftk::FileMode::Read);

                ZipReader zipReader(logSystem);
                zipReader.open(fileName, p.fileIO->getMemStart(), p.fileIO->getSize());

                std::string json = zipReader.readText("content.otio");
                otioTimeline = dynamic_cast<OTIO_NS::Timeline*>(
                    OTIO_NS::Timeline::from_json_string(json, &otioError));
                if (!otioTimeline)
                {
                    throw std::runtime_error(
                        ftk::Format("Cannot read timeline: \"{0}\"").
                        arg(path.get()));
                }
                else if (OTIO_NS::is_error(otioError))
                {
                    throw std::runtime_error(
                        ftk::Format("Cannot read timeline: \"{0}\": {1}").
                        arg(path.get()).
                        arg(otioError.details));
                }

                for (auto clip : otioTimeline->find_children<OTIO_NS::Clip>())
                {
                    if (auto externalReference =
                        dynamic_cast<OTIO_NS::ExternalReference*>(clip->media_reference()))
                    {
                        const std::string mediaFileName = ftk::Path(
                            decodeURL(externalReference->target_url())).get();

                        auto entry = zipReader.find(mediaFileName);
                        if (!entry.has_value())
                        {
                            throw std::runtime_error(ftk::Format(
                                "Cannot find zip entry: \"{0}\"").arg(mediaFileName));
                        }

                        p.memFiles[externalReference].push_back(ftk::MemFile(
                            p.fileIO->getMemStart() + entry->offset,
                            entry->size));
                    }
                    else if (auto imageSeqReference =
                        dynamic_cast<OTIO_NS::ImageSequenceReference*>(clip->media_reference()))
                    {
                        std::vector<ftk::MemFile> mem;
                        for (int number = 0;
                            number < imageSeqReference->number_of_images_in_sequence();
                            ++number)
                        {
                            const std::string mediaFileName = ftk::Path(
                                decodeURL(imageSeqReference->target_url_for_image_number(number))).get();

                            auto entry = zipReader.find(mediaFileName);
                            if (!entry.has_value())
                            {
                                throw std::runtime_error(ftk::Format(
                                    "Cannot find zip entry: \"{0}\"").arg(mediaFileName));
                            }

                            mem.push_back(ftk::MemFile(
                                p.fileIO->getMemStart() + entry->offset,
                                entry->size));
                        }
                        p.memFiles[imageSeqReference] = mem;
                    }
                }
            }
        }

        OTIO_NS::AnyDictionary dict;
        dict["path"] = path.get();
        dict["audioPath"] = audioPath.get();
        otioTimeline->metadata()["tlRender"] = dict;

        _init(context, otioTimeline, options);
    }

    void Timeline::_init(
        const std::shared_ptr<ftk::Context>& context,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& otioTimeline,
        const Options& options)
    {
        FTK_P();

        p.context = context;
        auto logSystem = context->getLogSystem();
        p.logSystem = logSystem;
        {
            std::vector<std::string> lines;
            lines.push_back(std::string());
            lines.push_back(ftk::Format("    * Image sequence audio: {0}").
                arg(options.imageSeqAudio));
            lines.push_back(ftk::Format("    * Image sequence audio extensions: {0}").
                arg(ftk::join(options.imageSeqAudioExts, ", ")));
            lines.push_back(ftk::Format("    * Image sequence audio file name: {0}").
                arg(options.imageSeqAudioFileName));
            lines.push_back(ftk::Format("    * Compatability: {0}").
                arg(options.compat));
            lines.push_back(ftk::Format("    * Video request max: {0}").
                arg(options.videoRequestMax));
            lines.push_back(ftk::Format("    * Audio request max: {0}").
                arg(options.audioRequestMax));
            lines.push_back(ftk::Format("    * Request timeout: {0}ms").
                arg(options.requestTimeout.count()));
            for (const auto& i : options.ioOptions)
            {
                lines.push_back(ftk::Format("    * AV I/O {0}: {1}").
                    arg(i.first).
                    arg(i.second));
            }
            lines.push_back(ftk::Format("    * Path max number digits: {0}").
                arg(options.pathOptions.seqMaxDigits));
            logSystem->print(
                ftk::Format("tl::Timeline {0}").arg(this),
                ftk::join(lines, "\n"));
        }

        p.otioTimeline = otioTimeline;
        const auto i = otioTimeline->metadata().find("tlRender");
        if (i != otioTimeline->metadata().end())
        {
            try
            {
                const auto dict = std::any_cast<OTIO_NS::AnyDictionary>(i->second);
                auto j = dict.find("path");
                if (j != dict.end())
                {
                    p.path = ftk::Path(std::any_cast<std::string>(j->second));
                }
                j = dict.find("audioPath");
                if (j != dict.end())
                {
                    p.audioPath = ftk::Path(std::any_cast<std::string>(j->second));
                }
            }
            catch (const std::exception&)
            {}
        }
        p.options = options;
        p.readCache.setMax(readCacheMax);

        // Get information about the timeline.
        p.timeRange = tl::getTimeRange(p.otioTimeline.value);
        for (const auto& i : p.otioTimeline.value->tracks()->children())
        {
            if (auto otioTrack = dynamic_cast<const OTIO_NS::Track*>(i.value))
            {
                if (OTIO_NS::Track::Kind::audio == otioTrack->kind())
                {
                    if (_getAudioInfo(otioTrack))
                    {
                        auto j = p.options.ioOptions.find("FFmpeg/AudioChannelCount");
                        if (j == p.options.ioOptions.end())
                        {
                            p.options.ioOptions["FFmpeg/AudioChannelCount"] =
                                ftk::Format("{0}").arg(p.ioInfo.audio.channelCount);
                        }
                        j = p.options.ioOptions.find("FFmpeg/AudioType");
                        if (j == p.options.ioOptions.end())
                        {
                            p.options.ioOptions["FFmpeg/AudioType"] =
                                ftk::Format("{0}").arg(p.ioInfo.audio.type);
                        }
                        j = p.options.ioOptions.find("FFmpeg/AudioSampleRate");
                        if (j == p.options.ioOptions.end())
                        {
                            p.options.ioOptions["FFmpeg/AudioSampleRate"] =
                                ftk::Format("{0}").arg(p.ioInfo.audio.sampleRate);
                        }
                        break;
                    }
                }
            }
        }
        for (const auto& i : p.otioTimeline.value->tracks()->children())
        {
            if (auto otioTrack = dynamic_cast<const OTIO_NS::Track*>(i.value))
            {
                if (OTIO_NS::Track::Kind::video == otioTrack->kind())
                {
                    if (_getVideoInfo(otioTrack))
                    {
                        break;
                    }
                }
            }
        }

        logSystem->print(
            ftk::Format("tl::Timeline {0}").arg(this),
            ftk::Format(
                "\n"
                "    * Time range: {0}\n"
                "    * Video: {1} {2}\n"
                "    * Audio: {3} {4} {5}").
            arg(p.timeRange).
            arg(!p.ioInfo.video.empty() ? p.ioInfo.video[0].size : ftk::Size2I()).
            arg(!p.ioInfo.video.empty() ? p.ioInfo.video[0].type : ftk::ImageType::None).
            arg(p.ioInfo.audio.channelCount).
            arg(p.ioInfo.audio.type).
            arg(p.ioInfo.audio.sampleRate));

        // Create a new thread.
        p.thread.running = true;
        p.thread.thread = std::thread(
            [this]
            {
                FTK_P();
                p.thread.logTimer = std::chrono::steady_clock::now();
                while (p.thread.running)
                {
                    _tick();
                }
                _finishRequests();
            });
    }

    namespace
    {
        std::atomic<size_t> objectCount = 0;
    }

    Timeline::Timeline() :
        _p(new Private)
    {
        ++objectCount;
    }

    Timeline::~Timeline()
    {
        FTK_P();
        if (auto logSystem = p.logSystem.lock())
        {
            logSystem->print(
                ftk::Format("tl::~Timeline {0}").arg(this),
                p.path.get());
        }

        p.thread.running = false;
        if (p.thread.thread.joinable())
        {
            p.thread.thread.join();
        }

        --objectCount;
    }

    std::shared_ptr<Timeline> Timeline::create(
        const std::shared_ptr<ftk::Context>& context,
        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& timeline,
        const Options& options)
    {
        auto out = std::shared_ptr<Timeline>(new Timeline);
        out->_init(context, timeline, options);
        return out;
    }

    std::shared_ptr<Timeline> Timeline::create(
        const std::shared_ptr<ftk::Context>& context,
        const ftk::Path& path,
        const Options& options)
    {
        auto out = std::shared_ptr<Timeline>(new Timeline);
        out->_init(context, path, ftk::Path(), options);
        return out;
    }

    std::shared_ptr<Timeline> Timeline::create(
        const std::shared_ptr<ftk::Context>& context,
        const ftk::Path& path,
        const ftk::Path& audioPath,
        const Options& options)
    {
        auto out = std::shared_ptr<Timeline>(new Timeline);
        out->_init(context, path, audioPath, options);
        return out;
    }

    std::shared_ptr<Timeline> Timeline::create(
        const std::shared_ptr<ftk::Context>& context,
        const std::string& fileName,
        const Options& options)
    {
        auto out = std::shared_ptr<Timeline>(new Timeline);
        out->_init(
            context,
            ftk::Path(fileName, options.pathOptions),
            ftk::Path(),
            options);
        return out;
    }

    std::shared_ptr<Timeline> Timeline::create(
        const std::shared_ptr<ftk::Context>& context,
        const std::string& fileName,
        const std::string& audioFileName,
        const Options& options)
    {
        auto out = std::shared_ptr<Timeline>(new Timeline);
        out->_init(
            context,
            ftk::Path(fileName, options.pathOptions),
            ftk::Path(audioFileName, options.pathOptions),
            options);
        return out;
    }

    std::shared_ptr<ftk::Context> Timeline::getContext() const
    {
        return _p->context.lock();
    }
        
    const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& Timeline::getTimeline() const
    {
        return _p->otioTimeline;
    }

    const ftk::Path& Timeline::getPath() const
    {
        return _p->path;
    }

    const ftk::Path& Timeline::getAudioPath() const
    {
        return _p->audioPath;
    }

    const Options& Timeline::getOptions() const
    {
        return _p->options;
    }
    
    std::vector<ftk::MemFile> Timeline::getMem(const OTIO_NS::MediaReference* otioRef)
    {
        FTK_P();
        const auto i = p.memFiles.find(otioRef);
        return i != p.memFiles.end() ? i->second : std::vector<ftk::MemFile>{};
    }

    const OTIO_NS::TimeRange& Timeline::getTimeRange() const
    {
        return _p->timeRange;
    }

    OTIO_NS::RationalTime Timeline::getDuration() const
    {
        return _p->timeRange.duration();
    }

    const IOInfo& Timeline::getIOInfo() const
    {
        return _p->ioInfo;
    }

    VideoRequest Timeline::getVideo(
        const OTIO_NS::RationalTime& time,
        const IOOptions& options)
    {
        FTK_P();
        (p.requestId)++;
        auto request = std::make_shared<Private::VideoRequest>();
        request->id = p.requestId;
        request->time = time;
        request->options = options;
        VideoRequest out;
        out.id = p.requestId;
        out.future = request->promise.get_future();
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
            request->promise.set_value(VideoFrame());
        }
        return out;
    }

    AudioRequest Timeline::getAudio(
        double seconds,
        const IOOptions& options)
    {
        FTK_P();
        (p.requestId)++;
        auto request = std::make_shared<Private::AudioRequest>();
        request->id = p.requestId;
        request->seconds = seconds;
        request->options = options;
        AudioRequest out;
        out.id = p.requestId;
        out.future = request->promise.get_future();
        bool valid = false;
        {
            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            if (!p.mutex.stopped)
            {
                valid = true;
                p.mutex.audioRequests.push_back(request);
            }
        }
        if (valid)
        {
            p.thread.cv.notify_one();
        }
        else
        {
            request->promise.set_value(AudioFrame());
        }
        return out;
    }

    void Timeline::cancelRequests(const std::vector<uint64_t>& ids)
    {
        FTK_P();
        std::unique_lock<std::mutex> lock(p.mutex.mutex);
        {
            auto i = p.mutex.videoRequests.begin();
            while (i != p.mutex.videoRequests.end())
            {
                const auto j = std::find(ids.begin(), ids.end(), (*i)->id);
                if (j != ids.end())
                {
                    i = p.mutex.videoRequests.erase(i);
                }
                else
                {
                    ++i;
                }
            }
        }
        {
            auto i = p.mutex.audioRequests.begin();
            while (i != p.mutex.audioRequests.end())
            {
                const auto j = std::find(ids.begin(), ids.end(), (*i)->id);
                if (j != ids.end())
                {
                    i = p.mutex.audioRequests.erase(i);
                }
                else
                {
                    ++i;
                }
            }
        }
    }

    size_t Timeline::getObjectCount()
    {
        return objectCount;
    }

    namespace
    {
        std::string getKey(const ftk::Path& path)
        {
            std::vector<std::string> out;
            out.push_back(path.get());
            out.push_back(path.getNum());
            return ftk::join(out, ';');
        }
    }

    std::shared_ptr<IRead> Timeline::_getRead(
        const OTIO_NS::Clip* clip,
        const IOOptions& ioOptions)
    {
        FTK_P();
        std::shared_ptr<IRead> out;
        const auto path = tl::getPath(
            clip->media_reference(),
            p.path.getDir(),
            p.options.pathOptions);
        const std::string key = getKey(path);
        if (!p.readCache.get(key, out))
        {
            if (auto context = p.context.lock())
            {
                const auto mem = getMem(clip->media_reference());
                IOOptions options = ioOptions;
                options["SeqIO/DefaultSpeed"] = ftk::Format("{0}").arg(p.timeRange.duration().rate());
                const auto ioSystem = context->getSystem<ReadSystem>();
                out = ioSystem->read(path, mem, options);
                p.readCache.add(key, out);
            }
        }
        return out;
    }

    std::future<VideoData> Timeline::_readVideo(
        const OTIO_NS::Clip* clip,
        const OTIO_NS::RationalTime& time,
        const IOOptions& options)
    {
        FTK_P();
        std::future<VideoData> out;
        IOOptions optionsMerged = merge(options, p.options.ioOptions);
        optionsMerged["USD/CameraName"] = clip->name();
        auto read = _getRead(clip, optionsMerged);
        const auto timeRangeOpt = clip->trimmed_range_in_parent();
        if (read && timeRangeOpt.has_value())
        {
            const IOInfo& ioInfo = read->getInfo().get();
            OTIO_NS::TimeRange availableRange = clip->available_range();
            OTIO_NS::TimeRange trimmedRange = clip->trimmed_range();
            if (p.options.compat &&
                availableRange.start_time() > ioInfo.videoTime.start_time())
            {
                //! \bug If the available range is greater than the media time,
                //! assume the media time is wrong (e.g., Picchu) and
                //! compensate for it.
                trimmedRange = OTIO_NS::TimeRange(
                    trimmedRange.start_time() - availableRange.start_time(),
                    trimmedRange.duration());
            }
            const auto mediaTime = toVideoMediaTime(
                time,
                timeRangeOpt.value(),
                trimmedRange,
                ioInfo.videoTime.duration().rate());
            out = read->readVideo(mediaTime, optionsMerged);
        }
        return out;
    }

    std::future<AudioData> Timeline::_readAudio(
        const OTIO_NS::Clip* clip,
        const OTIO_NS::TimeRange& timeRange,
        const IOOptions& options)
    {
        FTK_P();
        std::future<AudioData> out;
        IOOptions optionsMerged = merge(options, p.options.ioOptions);
        auto read = _getRead(clip, optionsMerged);
        const auto timeRangeOpt = clip->trimmed_range_in_parent();
        if (read && timeRangeOpt.has_value())
        {
            const IOInfo& ioInfo = read->getInfo().get();
            OTIO_NS::TimeRange trimmedRange = clip->trimmed_range();
            if (p.options.compat &&
                trimmedRange.start_time() < ioInfo.audioTime.start_time())
            {
                //! \bug If the trimmed range is less than the media time,
                //! assume the media time is wrong (e.g., ALab trailer) and
                //! compensate for it.
                trimmedRange = OTIO_NS::TimeRange(
                    ioInfo.audioTime.start_time() + trimmedRange.start_time(),
                    trimmedRange.duration());
            }
            const auto mediaRange = toAudioMediaTime(
                timeRange,
                timeRangeOpt.value(),
                trimmedRange,
                ioInfo.audio.sampleRate);
            out = read->readAudio(mediaRange, optionsMerged);
        }
        return out;
    }

    bool Timeline::_getVideoInfo(const OTIO_NS::Composable* composable)
    {
        FTK_P();
        if (auto clip = dynamic_cast<const OTIO_NS::Clip*>(composable))
        {
            if (auto context = p.context.lock())
            {
                // The first video clip defines the video information for the timeline.
                if (auto read = _getRead(clip, p.options.ioOptions))
                {
                    const IOInfo& ioInfo = read->getInfo().get();
                    p.ioInfo.video = ioInfo.video;
                    p.ioInfo.videoTime = ioInfo.videoTime;
                    p.ioInfo.tags.insert(ioInfo.tags.begin(), ioInfo.tags.end());
                    return true;
                }
            }
        }
        if (auto composition = dynamic_cast<const OTIO_NS::Composition*>(composable))
        {
            for (const auto& child : composition->children())
            {
                if (_getVideoInfo(child))
                {
                    return true;
                }
            }
        }
        return false;
    }

    bool Timeline::_getAudioInfo(const OTIO_NS::Composable* composable)
    {
        FTK_P();
        if (auto clip = dynamic_cast<const OTIO_NS::Clip*>(composable))
        {
            if (auto context = p.context.lock())
            {
                // The first audio clip defines the audio information for the timeline.
                if (auto read = _getRead(clip, p.options.ioOptions))
                {
                    const IOInfo& ioInfo = read->getInfo().get();
                    p.ioInfo.audio = ioInfo.audio;
                    p.ioInfo.audioTime = ioInfo.audioTime;
                    p.ioInfo.tags.insert(ioInfo.tags.begin(), ioInfo.tags.end());
                    return true;
                }
            }
        }
        if (auto composition = dynamic_cast<const OTIO_NS::Composition*>(composable))
        {
            for (const auto& child : composition->children())
            {
                if (_getAudioInfo(child))
                {
                    return true;
                }
            }
        }
        return false;
    }

    float Timeline::_transitionValue(double frame, double in, double out) const
    {
        return (frame - in) / (out - in);
    }

    void Timeline::_tick()
    {
        FTK_P();

        const auto t0 = std::chrono::steady_clock::now();

        _requests();

        // Logging.
        auto t1 = std::chrono::steady_clock::now();
        const std::chrono::duration<float> diff = t1 - p.thread.logTimer;
        if (diff.count() > 10.F)
        {
            p.thread.logTimer = t1;
            if (auto logSystem = p.logSystem.lock())
            {
                size_t videoRequestsSize = 0;
                size_t audioRequestsSize = 0;
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    videoRequestsSize = p.mutex.videoRequests.size();
                    audioRequestsSize = p.mutex.audioRequests.size();
                }
                logSystem->print(
                    ftk::Format("tl::Timeline {0}").arg(this),
                    ftk::Format(
                        "\n"
                        "    * Path: {0}\n"
                        "    * Video requests: {1}, {2} in-progress, {3} max\n"
                        "    * Audio requests: {4}, {5} in-progress, {6} max").
                    arg(p.path.get()).
                    arg(videoRequestsSize).
                    arg(p.thread.videoRequestsInProgress.size()).
                    arg(p.options.videoRequestMax).
                    arg(audioRequestsSize).
                    arg(p.thread.audioRequestsInProgress.size()).
                    arg(p.options.audioRequestMax));
            }
            t1 = std::chrono::steady_clock::now();
        }

        // Sleep for a bit.
        ftk::sleep(timeout, t0, t1);
    }

    void Timeline::_requests()
    {
        FTK_P();

        // Gather requests.
        std::list<std::shared_ptr<Private::VideoRequest> > newVideoRequests;
        std::list<std::shared_ptr<Private::AudioRequest> > newAudioRequests;
        {
            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            p.thread.cv.wait_for(
                lock,
                p.options.requestTimeout,
                [this]
                {
                    return
                        !_p->mutex.videoRequests.empty() ||
                        !_p->thread.videoRequestsInProgress.empty() ||
                        !_p->mutex.audioRequests.empty() ||
                        !_p->thread.audioRequestsInProgress.empty();
                });
            while (!p.mutex.videoRequests.empty() &&
                (p.thread.videoRequestsInProgress.size() + newVideoRequests.size()) < p.options.videoRequestMax)
            {
                newVideoRequests.push_back(p.mutex.videoRequests.front());
                p.mutex.videoRequests.pop_front();
            }
            while (!p.mutex.audioRequests.empty() &&
                (p.thread.audioRequestsInProgress.size() + newAudioRequests.size()) < p.options.audioRequestMax)
            {
                newAudioRequests.push_back(p.mutex.audioRequests.front());
                p.mutex.audioRequests.pop_front();
            }
        }

        // Traverse the timeline for new video requests.
        for (auto& request : newVideoRequests)
        {
            for (const auto& otioTrack : p.otioTimeline->video_tracks())
            {
                if (otioTrack->enabled())
                {
                    for (const auto& otioChild : otioTrack->children())
                    {
                        if (auto otioItem = dynamic_cast<OTIO_NS::Item*>(otioChild.value))
                        {
                            const auto requestTime = request->time - p.timeRange.start_time();
                            OTIO_NS::ErrorStatus errorStatus;
                            const auto range = otioItem->trimmed_range_in_parent(&errorStatus);
                            if (range.has_value() && range.value().contains(requestTime))
                            {
                                Private::VideoLayerData videoLayerData;
                                try
                                {
                                    if (auto otioClip = dynamic_cast<const OTIO_NS::Clip*>(otioItem))
                                    {
                                        videoLayerData.image = _readVideo(otioClip, requestTime, request->options);
                                    }
                                    const auto neighbors = otioTrack->neighbors_of(otioItem, &errorStatus);
                                    if (auto otioTransition = dynamic_cast<OTIO_NS::Transition*>(neighbors.second.value))
                                    {
                                        if (requestTime > range.value().end_time_inclusive() - otioTransition->in_offset())
                                        {
                                            videoLayerData.transition = toTransition(otioTransition->transition_type());
                                            videoLayerData.transitionValue = _transitionValue(
                                                requestTime.value(),
                                                range.value().end_time_inclusive().value() - otioTransition->in_offset().value(),
                                                range.value().end_time_inclusive().value() + otioTransition->out_offset().value() + 1.0);
                                            const auto transitionNeighbors = otioTrack->neighbors_of(otioTransition, &errorStatus);
                                            if (const auto otioClipB = dynamic_cast<OTIO_NS::Clip*>(transitionNeighbors.second.value))
                                            {
                                                videoLayerData.imageB = _readVideo(otioClipB, requestTime, request->options);
                                            }
                                        }
                                    }
                                    if (auto otioTransition = dynamic_cast<OTIO_NS::Transition*>(neighbors.first.value))
                                    {
                                        if (requestTime < range.value().start_time() + otioTransition->out_offset())
                                        {
                                            std::swap(videoLayerData.image, videoLayerData.imageB);
                                            videoLayerData.transition = toTransition(otioTransition->transition_type());
                                            videoLayerData.transitionValue = _transitionValue(
                                                requestTime.value(),
                                                range.value().start_time().value() - otioTransition->in_offset().value() - 1.0,
                                                range.value().start_time().value() + otioTransition->out_offset().value());
                                            const auto transitionNeighbors = otioTrack->neighbors_of(otioTransition, &errorStatus);
                                            if (const auto otioClipB = dynamic_cast<OTIO_NS::Clip*>(transitionNeighbors.first.value))
                                            {
                                                videoLayerData.image = _readVideo(otioClipB, requestTime, request->options);
                                            }
                                        }
                                    }
                                }
                                catch (const std::exception&)
                                {
                                    //! \todo How should this be handled?
                                }
                                request->layerData.push_back(std::move(videoLayerData));
                            }
                        }
                    }
                }
            }

            p.thread.videoRequestsInProgress.push_back(request);
        }

        // Traverse the timeline for new audio requests.
        for (auto& request : newAudioRequests)
        {
            for (const auto& otioTrack : p.otioTimeline->audio_tracks())
            {
                if (otioTrack->enabled())
                {
                    for (const auto& otioChild : otioTrack->children())
                    {
                        if (auto otioClip = dynamic_cast<OTIO_NS::Clip*>(otioChild.value))
                        {
                            const auto rangeOptional = otioClip->trimmed_range_in_parent();
                            if (rangeOptional.has_value())
                            {
                                const OTIO_NS::TimeRange clipTimeRange(
                                    rangeOptional.value().start_time().rescaled_to(1.0),
                                    rangeOptional.value().duration().rescaled_to(1.0));
                                const double start = request->seconds -
                                    p.timeRange.start_time().rescaled_to(1.0).value();
                                const OTIO_NS::TimeRange requestTimeRange = OTIO_NS::TimeRange(
                                    OTIO_NS::RationalTime(start, 1.0),
                                    OTIO_NS::RationalTime(1.0, 1.0));
                                if (requestTimeRange.intersects(clipTimeRange))
                                {
                                    Private::AudioLayerData audioData;
                                    audioData.seconds = request->seconds;
                                    try
                                    {
                                        //! \bug Why is OTIO_NS::TimeRange::clamped() not giving us the
                                        //! result we expect?
                                        //audioData.timeRange = requestTimeRange.clamped(clipTimeRange);
                                        const double start = std::max(
                                            clipTimeRange.start_time().value(),
                                            requestTimeRange.start_time().value());
                                        const double end = std::min(
                                            clipTimeRange.start_time().value() + clipTimeRange.duration().value(),
                                            requestTimeRange.start_time().value() + requestTimeRange.duration().value());
                                        audioData.timeRange = OTIO_NS::TimeRange(
                                            OTIO_NS::RationalTime(start, 1.0),
                                            OTIO_NS::RationalTime(end - start, 1.0));
                                        audioData.audio = _readAudio(otioClip, audioData.timeRange, request->options);
                                    }
                                    catch (const std::exception&)
                                    {
                                        //! \todo How should this be handled?
                                    }
                                    request->layerData.push_back(std::move(audioData));
                                }
                            }
                        }
                    }
                }
            }
            p.thread.audioRequestsInProgress.push_back(request);
        }

        // Check for finished video requests.
        auto videoRequestIt = p.thread.videoRequestsInProgress.begin();
        while (videoRequestIt != p.thread.videoRequestsInProgress.end())
        {
            bool valid = true;
            for (auto& i : (*videoRequestIt)->layerData)
            {
                if (i.image.valid())
                {
                    valid &= i.image.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                }
                if (i.imageB.valid())
                {
                    valid &= i.imageB.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                }
            }
            if (valid)
            {
                VideoFrame frame;
                if (!p.ioInfo.video.empty())
                {
                    frame.size = p.ioInfo.video.front().size;
                }
                frame.time = (*videoRequestIt)->time;
                for (auto& j : (*videoRequestIt)->layerData)
                {
                    VideoLayer layer;
                    if (j.image.valid())
                    {
                        layer.image = j.image.get().image;
                    }
                    if (j.imageB.valid())
                    {
                        layer.imageB = j.imageB.get().image;
                    }
                    layer.transition = j.transition;
                    layer.transitionValue = j.transitionValue;
                    frame.layers.push_back(layer);
                }
                (*videoRequestIt)->promise.set_value(frame);
                videoRequestIt = p.thread.videoRequestsInProgress.erase(videoRequestIt);
                continue;
            }
            ++videoRequestIt;
        }

        // Check for finished audio requests.
        auto audioRequestIt = p.thread.audioRequestsInProgress.begin();
        while (audioRequestIt != p.thread.audioRequestsInProgress.end())
        {
            bool valid = true;
            for (auto& i : (*audioRequestIt)->layerData)
            {
                if (i.audio.valid())
                {
                    valid &= i.audio.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                }
            }
            if (valid)
            {
                AudioFrame frame;
                frame.seconds = (*audioRequestIt)->seconds;
                for (auto& j : (*audioRequestIt)->layerData)
                {
                    AudioLayer layer;
                    if (j.audio.valid())
                    {
                        const auto audioData = j.audio.get();
                        if (audioData.audio)
                        {
                            layer.audio = _padAudioToOneSecond(audioData.audio, j.seconds, j.timeRange);
                        }
                    }
                    frame.layers.push_back(layer);
                }
                if (frame.layers.empty())
                {
                    auto audio = Audio::create(p.ioInfo.audio, p.ioInfo.audio.sampleRate);
                    audio->zero();
                    frame.layers.push_back({ audio });
                }
                (*audioRequestIt)->promise.set_value(frame);
                audioRequestIt = p.thread.audioRequestsInProgress.erase(audioRequestIt);
                continue;
            }
            ++audioRequestIt;
        }
    }

    void Timeline::_finishRequests()
    {
        FTK_P();
        {
            std::list<std::shared_ptr<Private::VideoRequest> > videoRequests;
            std::list<std::shared_ptr<Private::AudioRequest> > audioRequests;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.stopped = true;
                videoRequests = std::move(p.mutex.videoRequests);
                audioRequests = std::move(p.mutex.audioRequests);
            }
            videoRequests.insert(
                videoRequests.begin(),
                p.thread.videoRequestsInProgress.begin(),
                p.thread.videoRequestsInProgress.end());
            p.thread.videoRequestsInProgress.clear();
            audioRequests.insert(
                audioRequests.begin(),
                p.thread.audioRequestsInProgress.begin(),
                p.thread.audioRequestsInProgress.end());
            p.thread.audioRequestsInProgress.clear();
            for (auto& request : videoRequests)
            {
                VideoFrame frame;
                frame.time = request->time;
                for (auto& i : request->layerData)
                {
                    VideoLayer layer;
                    if (i.image.valid())
                    {
                        layer.image = i.image.get().image;
                    }
                    if (i.imageB.valid())
                    {
                        layer.imageB = i.imageB.get().image;
                    }
                    layer.transition = i.transition;
                    layer.transitionValue = i.transitionValue;
                    frame.layers.push_back(layer);
                }
                request->promise.set_value(frame);
            }
            for (auto& request : audioRequests)
            {
                AudioFrame frame;
                frame.seconds = request->seconds;
                for (auto& i : request->layerData)
                {
                    AudioLayer layer;
                    if (i.audio.valid())
                    {
                        layer.audio = i.audio.get().audio;
                    }
                    frame.layers.push_back(layer);
                }
                request->promise.set_value(frame);
            }
        }
    }

    std::shared_ptr<Audio> Timeline::_padAudioToOneSecond(
        const std::shared_ptr<Audio>& audio,
        double seconds,
        const OTIO_NS::TimeRange& timeRange)
    {
        FTK_P();
        std::list<std::shared_ptr<Audio> > list;
        const double s = seconds - p.timeRange.start_time().rescaled_to(1.0).value();
        if (timeRange.start_time().value() > s)
        {
            const OTIO_NS::RationalTime t =
                timeRange.start_time() - OTIO_NS::RationalTime(s, 1.0);
            const OTIO_NS::RationalTime t2 =
                t.rescaled_to(audio->getInfo().sampleRate);
            auto silence = Audio::create(audio->getInfo(), t2.value());
            silence->zero();
            list.push_back(silence);
        }
        list.push_back(audio);
        if (timeRange.end_time_exclusive().value() < s + 1.0)
        {
            const OTIO_NS::RationalTime t =
                OTIO_NS::RationalTime(s + 1.0, 1.0) - timeRange.end_time_exclusive();
            const OTIO_NS::RationalTime t2 =
                t.rescaled_to(audio->getInfo().sampleRate);
            auto silence = Audio::create(audio->getInfo(), t2.value());
            silence->zero();
            list.push_back(silence);
        }
        size_t sampleCount = getSampleCount(list);
        auto out = Audio::create(audio->getInfo(), sampleCount);
        moveAudio(list, out->getData(), sampleCount);
        return out;
    }
}
