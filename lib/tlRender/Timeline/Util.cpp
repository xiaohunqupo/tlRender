// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/Timeline/Util.h>

#include <tlRender/Timeline/MemRef.h>

#include <tlRender/IO/System.h>

#include <tlRender/Core/URL.h>

#include <ftk/Core/Assert.h>
#include <ftk/Core/Context.h>
#include <ftk/Core/Error.h>
#include <ftk/Core/Format.h>
#include <ftk/Core/Path.h>
#include <ftk/Core/String.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>

#include <ctime>

namespace tl
{
    std::vector<std::string> getExts(
        const std::shared_ptr<ftk::Context>& context,
        int types)
    {
        std::vector<std::string> out;
        if (types & static_cast<int>(FileType::Media))
        {
            out.push_back(".otio");
            out.push_back(".otioz");
        }
        if (auto ioSystem = context->getSystem<ReadSystem>())
        {
            for (const auto& plugin : ioSystem->getPlugins())
            {
                const auto& exts = plugin->getExts(types);
                out.insert(out.end(), exts.begin(), exts.end());
            }
        }
        return out;
    }

    std::vector<OTIO_NS::TimeRange> toRanges(std::vector<OTIO_NS::RationalTime> frames)
    {
        std::vector<OTIO_NS::TimeRange> out;
        if (!frames.empty())
        {
            std::sort(frames.begin(), frames.end());
            auto i = frames.begin();
            auto j = i;
            do
            {
                auto k = j + 1;
                if (k != frames.end() && (*k - *j).value() > 1)
                {
                    out.push_back(OTIO_NS::TimeRange::range_from_start_end_time_inclusive(*i, *j));
                    i = k;
                    j = k;
                }
                else if (k == frames.end())
                {
                    out.push_back(OTIO_NS::TimeRange::range_from_start_end_time_inclusive(*i, *j));
                    i = k;
                    j = k;
                }
                else
                {
                    ++j;
                }
            } while (j != frames.end());
        }
        return out;
    }

    OTIO_NS::RationalTime loop(
        const OTIO_NS::RationalTime& value,
        const OTIO_NS::TimeRange& range,
        bool* looped)
    {
        auto out = value;
        const OTIO_NS::RationalTime start = range.start_time();
        const OTIO_NS::RationalTime end = range.end_time_inclusive();
        const OTIO_NS::RationalTime duration = range.duration();
        if (duration.value() > 0.0)
        {
            while (out < start)
            {
                if (looped)
                {
                    *looped = true;
                }
                out += duration;
            }
            while (out > end)
            {
                if (looped)
                {
                    *looped = true;
                }
                out -= duration;
            }
        }
        return out;
    }

    int64_t loop(
        int64_t value,
        const OTIO_NS::TimeRange& range,
        bool* looped)
    {
        return loop(
            OTIO_NS::RationalTime(value, 1.0),
            OTIO_NS::TimeRange(
                range.start_time().rescaled_to(1.0),
                range.duration().rescaled_to(1.0)),
            looped).value();
    }

    std::vector<OTIO_NS::TimeRange> loop(
        const OTIO_NS::TimeRange& range,
        const OTIO_NS::TimeRange& bounds)
    {
        std::vector<OTIO_NS::TimeRange> out;
        const OTIO_NS::RationalTime& rs = range.start_time();
        const OTIO_NS::RationalTime& bs = bounds.start_time();
        const OTIO_NS::RationalTime re = range.end_time_inclusive();
        const OTIO_NS::RationalTime be = bounds.end_time_inclusive();
        const OTIO_NS::RationalTime one(1.0, range.duration().rate());
        if (rs >= bs && re <= be)
        {
            out.push_back(range);
        }
        else if (rs < bs && re > be)
        {
            out.push_back(bounds);
        }
        else if (rs < bs && re >= bs)
        {
            out.push_back(OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                be - (bs - rs - one),
                be));
            out.push_back(OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                bs,
                re));
        }
        else if (rs <= be && re > be)
        {
            out.push_back(OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                rs,
                be));
            out.push_back(OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                bs,
                bs + (re - be - one)));
        }
        return out;
    }

    std::vector<ftk::Range<int64_t> > loop(
        const ftk::Range<int64_t>& range,
        const ftk::Range<int64_t>& bounds)
    {
        std::vector<ftk::Range<int64_t> > out;
        const int64_t rs = range.min();
        const int64_t bs = bounds.min();
        const int64_t re = range.max();
        const int64_t be = bounds.max();
        if (rs >= bs && re <= be)
        {
            out.push_back(range);
        }
        else if (rs < bs && re > be)
        {
            out.push_back(bounds);
        }
        else if (rs < bs && re >= bs)
        {
            out.push_back(ftk::Range<int64_t>(
                be - (bs - rs - 1),
                be));
            out.push_back(ftk::Range<int64_t>(
                bs,
                re));
        }
        else if (rs <= be && re > be)
        {
            out.push_back(ftk::Range<int64_t>(
                rs,
                be));
            out.push_back(ftk::Range<int64_t>(
                bs,
                bs + (re - be - 1)));
        }
        return out;
    }

    TL_ENUM_IMPL(
        CacheDir,
        "Forward",
        "Reverse");

    const OTIO_NS::Composable* getRoot(const OTIO_NS::Composable* composable)
    {
        const OTIO_NS::Composable* out = composable;
        for (; out->parent(); out = out->parent())
            ;
        return out;
    }

    std::optional<OTIO_NS::RationalTime> getDuration(
        const OTIO_NS::Timeline* otioTimeline,
        const std::string& kind)
    {
        std::optional<OTIO_NS::RationalTime> out;
        OTIO_NS::ErrorStatus errorStatus;
        for (auto track : otioTimeline->find_children<OTIO_NS::Track>(&errorStatus))
        {
            if (kind == track->kind())
            {
                const OTIO_NS::RationalTime duration = track->duration(&errorStatus);
                if (out.has_value())
                {
                    out = std::max(out.value(), duration);
                }
                else
                {
                    out = duration;
                }
            }
        }
        return out;
    }

    OTIO_NS::TimeRange getTimeRange(const OTIO_NS::Timeline* otioTimeline)
    {
        OTIO_NS::TimeRange out = invalidTimeRange;
        auto duration = getDuration(otioTimeline, OTIO_NS::Track::Kind::video);
        if (!duration.has_value())
        {
            duration = getDuration(otioTimeline, OTIO_NS::Track::Kind::audio);
        }
        if (duration.has_value())
        {
            const OTIO_NS::RationalTime startTime = otioTimeline->global_start_time().has_value() ?
                otioTimeline->global_start_time().value().rescaled_to(duration->rate()) :
                OTIO_NS::RationalTime(0, duration->rate());
            out = OTIO_NS::TimeRange(startTime, duration.value());
        }
        return out;
    }

    std::vector<ftk::Path> getPaths(
        const std::shared_ptr<ftk::Context>& context,
        const ftk::Path& path,
        const ftk::DirListOptions& options)
    {
        std::vector<ftk::Path> out;
        if (std::filesystem::is_directory(std::filesystem::u8path(path.get())))
        {
            auto ioSystem = context->getSystem<ReadSystem>();
            const auto entries = ftk::dirList(path.getFileName(true), options);
            for (const auto& entry : entries)
            {
                const ftk::Path& path = entry.path;
                const std::string ext = ftk::toLower(path.getExt());
                switch (ioSystem->getFileType(ext))
                {
                case FileType::Media:
                case FileType::Seq:
                    out.push_back(path);
                    break;
                default:
                    if (".otio" == ext || ".otioz" == ext)
                    {
                        out.push_back(path);
                    }
                    break;
                }
            }
        }
        else
        {
            out.push_back(path);
        }
        return out;
    }

    ftk::Path getPath(
        const std::string& url,
        const std::string& directory,
        const ftk::PathOptions& pathOptions)
    {
        ftk::Path out(decodeURL(url), pathOptions);
        if (!out.hasProtocol() && !out.isAbs())
        {
            out.setDir(ftk::appendSeparator(directory) + out.getDir());
        }
        return out;
    }

    ftk::Path getPath(
        const OTIO_NS::MediaReference* ref,
        const std::string& directory,
        ftk::PathOptions pathOptions)
    {
        std::string url;
        ftk::RangeI64 frames;
        if (auto externalRef = dynamic_cast<const OTIO_NS::ExternalReference*>(ref))
        {
            url = externalRef->target_url();
            pathOptions.seqMaxDigits = 0;
        }
        else if (auto imageSeqRef = dynamic_cast<const OTIO_NS::ImageSequenceReference*>(ref))
        {
            std::stringstream ss;
            ss << imageSeqRef->target_url_base() <<
                imageSeqRef->name_prefix() <<
                std::setfill('0') << std::setw(imageSeqRef->frame_zero_padding()) <<
                imageSeqRef->start_frame() <<
                imageSeqRef->name_suffix();
            url = ss.str();
            frames = ftk::RangeI64(
                imageSeqRef->start_frame(),
                imageSeqRef->end_frame());
        }
        else if (auto rawMemRef = dynamic_cast<const RawMemRef*>(ref))
        {
            url = rawMemRef->target_url();
            pathOptions.seqMaxDigits = 0;
        }
        else if (auto sharedMemRef = dynamic_cast<const SharedMemRef*>(ref))
        {
            url = sharedMemRef->target_url();
            pathOptions.seqMaxDigits = 0;
        }
        else if (auto seqRawMemRef = dynamic_cast<const SeqRawMemRef*>(ref))
        {
            url = seqRawMemRef->target_url();
        }
        else if (auto seqSharedMemRef = dynamic_cast<const SeqSharedMemRef*>(ref))
        {
            url = seqSharedMemRef->target_url();
        }
        ftk::Path out = getPath(url, directory, pathOptions);
        if (!frames.equal())
        {
            out.setFrames(frames);
        }
        return out;
    }

    std::vector<ftk::MemFile> getMemRead(const OTIO_NS::MediaReference* ref)
    {
        std::vector<ftk::MemFile> out;
        if (auto rawMemRef = dynamic_cast<const RawMemRef*>(ref))
        {
            out.push_back(ftk::MemFile(rawMemRef->mem(), rawMemRef->mem_size()));
        }
        else if (auto sharedMemRef = dynamic_cast<const SharedMemRef*>(ref))
        {
            if (const auto& mem = sharedMemRef->mem())
            {
                out.push_back(ftk::MemFile(mem->data(), mem->size()));
            }
        }
        else if (auto seqRawMemRef = dynamic_cast<const SeqRawMemRef*>(ref))
        {
            const auto& mem = seqRawMemRef->mem();
            const size_t mem_size = mem.size();
            const auto& mem_sizes = seqRawMemRef->mem_sizes();
            const size_t mem_sizes_size = mem_sizes.size();
            for (size_t i = 0; i < mem_size && i < mem_sizes_size; ++i)
            {
                out.push_back(ftk::MemFile(mem[i], mem_sizes[i]));
            }
        }
        else if (auto seqSharedMemRef = dynamic_cast<const SeqSharedMemRef*>(ref))
        {
            for (const auto& mem : seqSharedMemRef->mem())
            {
                if (mem)
                {
                    out.push_back(ftk::MemFile(mem->data(), mem->size()));
                }
            }
        }
        return out;
    }

    TL_ENUM_IMPL(
        ToMemRef,
        "Shared",
        "Raw");

    void toMemRefs(
        OTIO_NS::Timeline* otioTimeline,
        const std::string& directory,
        ToMemRef toMemRef,
        const ftk::PathOptions& pathOptions)
    {
        // Recursively iterate over all clips in the timeline.
        for (auto clip : otioTimeline->find_children<OTIO_NS::Clip>())
        {
            if (auto ref = dynamic_cast<OTIO_NS::ExternalReference*>(clip->media_reference()))
            {
                // Get the external reference path.
                const auto path = getPath(ref->target_url(), directory, pathOptions);

                // Read the external reference into memory.
                auto fileIO = ftk::FileIO::create(path.get(), ftk::FileMode::Read);
                const size_t size = fileIO->getSize();

                // Replace the external reference with a memory reference.
                switch (toMemRef)
                {
                case ToMemRef::Shared:
                {
                    auto mem = std::make_shared<MemRefData>();
                    mem->resize(size);
                    fileIO->read(mem->data(), size);
                    clip->set_media_reference(new SharedMemRef(
                        ref->target_url(),
                        mem,
                        clip->available_range(),
                        ref->metadata()));
                    break;
                }
                case ToMemRef::Raw:
                {
                    uint8_t* mem = new uint8_t [size];
                    fileIO->read(mem, size);
                    clip->set_media_reference(new RawMemRef(
                        ref->target_url(),
                        mem,
                        size,
                        clip->available_range(),
                        ref->metadata()));
                    break;
                }
                default: break;
                }
            }
            else if (auto ref = dynamic_cast<OTIO_NS::ImageSequenceReference*>(
                clip->media_reference()))
            {
                // Get the image sequence reference path.
                const int padding = ref->frame_zero_padding();
                std::stringstream ss;
                ss << ref->target_url_base() <<
                    ref->name_prefix() <<
                    std::setfill('0') << std::setw(padding) << ref->start_frame() <<
                    ref->name_suffix();
                const auto path = getPath(ss.str(), directory, pathOptions);

                // Read the image sequence reference into memory.
                std::vector<std::shared_ptr<MemRefData> > sharedMemList;
                std::vector<const uint8_t*> rawMemList;
                std::vector<size_t> rawMemSizeList;
                const auto range = clip->trimmed_range();
                for (
                    int64_t frame = ref->start_frame();
                    frame < ref->start_frame() + range.duration().value();
                    ++frame)
                {
                    const auto fileName = path.getFrame(frame, true);
                    auto fileIO = ftk::FileIO::create(fileName, ftk::FileMode::Read);
                    const size_t size = fileIO->getSize();
                    switch (toMemRef)
                    {
                    case ToMemRef::Shared:
                    {
                        auto mem = std::make_shared<MemRefData>();
                        mem->resize(size);
                        fileIO->read(mem->data(), size);
                        sharedMemList.push_back(mem);
                        break;
                    }
                    case ToMemRef::Raw:
                    {
                        auto mem = new uint8_t [size];
                        fileIO->read(mem, size);
                        rawMemList.push_back(mem);
                        rawMemSizeList.push_back(size);
                        break;
                    }
                    default: break;
                    }
                }

                // Replace the image sequence reference with a memory
                // sequence reference.
                switch (toMemRef)
                {
                case ToMemRef::Shared:
                    clip->set_media_reference(new SeqSharedMemRef(
                        path.get(),
                        sharedMemList,
                        clip->available_range(),
                        ref->metadata()));
                    break;
                case ToMemRef::Raw:
                    clip->set_media_reference(new SeqRawMemRef(
                        path.get(),
                        rawMemList,
                        rawMemSizeList,
                        clip->available_range(),
                        ref->metadata()));
                    break;
                default: break;
                }
            }
        }
    }

    OTIO_NS::RationalTime toVideoMediaTime(
        const OTIO_NS::RationalTime& time,
        const OTIO_NS::TimeRange& trimmedRangeInParent,
        const OTIO_NS::TimeRange& trimmedRange,
        double rate)
    {
        OTIO_NS::RationalTime out =
            time - trimmedRangeInParent.start_time() + trimmedRange.start_time();
        out = out.rescaled_to(rate).round();
        return out;
    }

    OTIO_NS::TimeRange toAudioMediaTime(
        const OTIO_NS::TimeRange& timeRange,
        const OTIO_NS::TimeRange& trimmedRangeInParent,
        const OTIO_NS::TimeRange& trimmedRange,
        double sampleRate)
    {
        OTIO_NS::TimeRange out = OTIO_NS::TimeRange(
            timeRange.start_time() - trimmedRangeInParent.start_time() + trimmedRange.start_time(),
            timeRange.duration());
        out = OTIO_NS::TimeRange(
            out.start_time().rescaled_to(sampleRate).round(),
            out.duration().rescaled_to(sampleRate).round());
        return out;
    }

    std::vector<std::shared_ptr<Audio> > audioCopy(
        const AudioInfo& info,
        const std::vector<AudioFrame>& audioFrame,
        Playback playback,
        int64_t frame,
        int64_t size)
    {
        std::vector<std::shared_ptr<Audio> > out;

        // Adjust the frame for reverse playback.
        if (Playback::Reverse == playback)
        {
            frame -= size;
        }

        // Find the first chunk of audio data.
        const int64_t seconds = std::floor(frame / static_cast<double>(info.sampleRate));
        auto secondsIt = std::find_if(
            audioFrame.begin(),
            audioFrame.end(),
            [seconds](const AudioFrame& audioFrame)
            {
                return seconds == audioFrame.seconds;
            });

        // Find the second chunk of audio data.
        const int64_t secondsPlusOne = seconds + 1;
        auto secondsPlusOneIt = std::find_if(
            audioFrame.begin(),
            audioFrame.end(),
            [secondsPlusOne](const AudioFrame& audioFrame)
            {
                return secondsPlusOne == audioFrame.seconds;
            });

        if (secondsIt != audioFrame.end())
        {
            // Adjust the size if necessary.
            const int64_t offset = frame - seconds * info.sampleRate;
            int64_t outSize = size;
            if ((offset + outSize) > info.sampleRate && secondsPlusOneIt == audioFrame.end())
            {
                outSize = info.sampleRate - offset;
            }

            // Create the output audio.
            for (size_t i = 0; i < secondsIt->layers.size(); ++i)
            {
                auto audio = Audio::create(info, outSize);
                audio->zero();
                out.push_back(audio);
            }

            // Copy audio from the first chunk.
            const int64_t sizeTmp = std::min(outSize, static_cast<int64_t>(info.sampleRate) - offset);
            for (size_t i = 0; i < secondsIt->layers.size(); ++i)
            {
                if (secondsIt->layers[i].audio &&
                    secondsIt->layers[i].audio->getInfo() == info)
                {
                    memcpy(
                        out[i]->getData(),
                        secondsIt->layers[i].audio->getData() + offset * info.getByteCount(),
                        sizeTmp * info.getByteCount());
                }
            }

            if (sizeTmp < outSize && secondsPlusOneIt != audioFrame.end())
            {
                // Copy audio from the second chunk.
                for (size_t i = 0; i < secondsIt->layers.size() && i < secondsPlusOneIt->layers.size(); ++i)
                {
                    if (secondsPlusOneIt->layers[i].audio &&
                        secondsPlusOneIt->layers[i].audio->getInfo() == info)
                    {
                        memcpy(
                            out[i]->getData() + sizeTmp * info.getByteCount(),
                            secondsPlusOneIt->layers[i].audio->getData(),
                            (outSize - sizeTmp) * info.getByteCount());
                    }
                }
            }
        }

        return out;
    }
}
