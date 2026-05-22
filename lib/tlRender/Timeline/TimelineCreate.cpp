// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/Timeline/TimelinePrivate.h>

#include <tlRender/Timeline/MemRef.h>
#include <tlRender/Timeline/Util.h>

#include <tlRender/IO/System.h>

#include <tlRender/Core/URL.h>

#include <ftk/Core/Context.h>
#include <ftk/Core/Format.h>
#include <ftk/Core/String.h>

#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>

#include <mz.h>
#include <mz_strm.h>
#include <mz_zip.h>
#include <mz_zip_rw.h>

#include <filesystem>

namespace tl
{
    namespace
    {
        ftk::Path getAudioPath(
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

    uint16_t readLE16(const uint8_t* p)
    {
        return
            static_cast<uint16_t>(p[0]) |
            (static_cast<uint16_t>(p[1]) << 8);
    }

    uint32_t readLE32(const uint8_t* p)
    {
        return
            static_cast<uint32_t>(p[0]) |
            (static_cast<uint32_t>(p[1]) << 8) |
            (static_cast<uint32_t>(p[2]) << 16) |
            (static_cast<uint32_t>(p[3]) << 24);
    }

    constexpr uint32_t zipHeaderMagic = 0x04034b50u;
    constexpr size_t zipHeaderNameOffset = 26;
    constexpr size_t zipHeaderExtraLenOffset = 28;
    constexpr size_t zipHeaderSize = 30;

    class ZipReader
    {
        FTK_NON_COPYABLE(ZipReader);

        struct MZReaderDeleter
        {
            void operator()(void* p) const { if (p) mz_zip_reader_delete(&p); }
        };
        using MZReaderPtr = std::unique_ptr<void, MZReaderDeleter>;

        struct MZEntryScope
        {
            MZEntryScope(void* p) : p(p) {}
            ~MZEntryScope() { if (p) mz_zip_reader_entry_close(p); }
            void* p = nullptr;
        };

    public:
        ZipReader(const std::shared_ptr<ftk::LogSystem>& logSystem) :
            _logSystem(logSystem)
        {}

        void open(
            const std::string& fileName,
            const uint8_t* fileMMap,
            size_t fileSize);

        struct Entry { int64_t offset; int64_t size; };

        std::optional<Entry> find(const std::string& name) const;

        std::string readText(const std::string& name);

    private:
        std::shared_ptr<ftk::LogSystem> _logSystem;
        std::string _fileName;
        const uint8_t* _fileMMap = nullptr;
        size_t _fileSize = 0;
        MZReaderPtr _reader;
        std::map<std::string, Entry> _entries;
    };

    void ZipReader::open(
        const std::string& fileName,
        const uint8_t* fileMMap,
        size_t fileSize)
    {
        if (_reader)
        {
            _reader.reset();
            _entries.clear();
        }

        _fileName = fileName;
        _fileMMap = fileMMap;
        _fileSize = fileSize;

        _reader.reset(mz_zip_reader_create());
        if (!_reader.get())
        {
            throw std::runtime_error(ftk::Format(
                "Cannot create zip reader: \"{0}\"").arg(fileName));
        }
        int32_t err = mz_zip_reader_open_file(_reader.get(), fileName.c_str());
        if (err != MZ_OK)
        {
            throw std::runtime_error(ftk::Format(
                "Cannot open zip reader: \"{0}\"").arg(fileName));
        }

        err = mz_zip_reader_goto_first_entry(_reader.get());
        if (err != MZ_OK)
        {
            throw std::runtime_error(ftk::Format(
                "Cannot goto first zip entry: \"{0}\"").arg(fileName));
        }
        while (MZ_OK == err)
        {
            mz_zip_file* fileInfo = nullptr;
            err = mz_zip_reader_entry_get_info(_reader.get(), &fileInfo);
            if (err != MZ_OK || !fileInfo)
            {
                throw std::runtime_error(ftk::Format(
                    "Cannot get zip entry information: \"{0}\"").arg(fileName));
            }
            if (mz_zip_reader_entry_is_dir(_reader.get()) != MZ_OK &&
                0 == fileInfo->compression_method)
            {
                if (fileInfo->disk_offset < 0 ||
                    static_cast<size_t>(fileInfo->disk_offset) + zipHeaderSize > _fileSize)
                {
                    throw std::runtime_error(ftk::Format(
                        "Local zip header entry out of bounds: \"{0}\"").arg(fileName));
                }
                const uint8_t* hdr = _fileMMap + fileInfo->disk_offset;
                if (readLE32(hdr) != zipHeaderMagic)
                {
                    throw std::runtime_error(ftk::Format(
                        "Bad local zip header: \"{0}\"").arg(fileName));
                }
                const uint16_t nameLen  = readLE16(hdr + zipHeaderNameOffset);
                const uint16_t extraLen = readLE16(hdr + zipHeaderExtraLenOffset);
                const int64_t dataOffset =
                    fileInfo->disk_offset + zipHeaderSize + nameLen + extraLen;

                if (dataOffset < 0 ||
                    static_cast<size_t>(dataOffset) > _fileSize ||
                    fileInfo->uncompressed_size < 0 ||
                    static_cast<size_t>(fileInfo->uncompressed_size) > _fileSize - dataOffset)
                {
                    throw std::runtime_error(ftk::Format(
                        "Local zip entry out of bounds: \"{0}\"").arg(fileName));
                }
                Entry entry{ dataOffset, fileInfo->uncompressed_size };
                if (!_entries.emplace(fileInfo->filename, entry).second)
                {
                    _logSystem->print("tl::ZipReader", ftk::Format(
                        "Duplicate zip entry, ignoring subsequent: \"{0}\"").arg(fileInfo->filename),
                        ftk::LogType::Warning);
                }
            }
            err = mz_zip_reader_goto_next_entry(_reader.get());
            if (err != MZ_OK && err != MZ_END_OF_LIST)
            {
                throw std::runtime_error(ftk::Format(
                    "Cannot goto next zip entry: \"{0}\"").arg(fileName));
            }
        }
    }

    std::optional<ZipReader::Entry> ZipReader::find(const std::string& name) const
    {
        const auto i = _entries.find(name);
        return i != _entries.end() ? std::optional<Entry>(i->second) : std::nullopt;
    }

    std::string ZipReader::readText(const std::string& name)
    {
        int32_t err = mz_zip_reader_locate_entry(
            _reader.get(),
            name.c_str(),
            0);
        if (err != MZ_OK)
        {
            throw std::runtime_error(ftk::Format(
                "Cannot find zip entry: \"{0}\"").arg(name));
        }
        err = mz_zip_reader_entry_open(_reader.get());
        if (err != MZ_OK)
        {
            throw std::runtime_error(ftk::Format(
                "Cannot open zip entry: \"{0}\"").arg(name));
        }
        MZEntryScope entry(_reader.get());
        mz_zip_file* fileInfo = nullptr;
        err = mz_zip_reader_entry_get_info(_reader.get(), &fileInfo);
        if (err != MZ_OK || !fileInfo)
        {
            throw std::runtime_error(ftk::Format(
                "Cannot get zip entry information: \"{0}\"").arg(name));
        }
        if (fileInfo->uncompressed_size > INT32_MAX)
        {
            throw std::runtime_error(ftk::Format(
                "Text zip entry exceeds max size: \"{0}\"").arg(name));
        }
        std::string out(fileInfo->uncompressed_size, 0);
        err = mz_zip_reader_entry_read(
            _reader.get(),
            out.data(),
            fileInfo->uncompressed_size);
        if (err != fileInfo->uncompressed_size)
        {
            throw std::runtime_error(ftk::Format(
                "Cannot read zip entry: \"{0}\"").arg(name));
        }
        return out;
    }

    OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> readOTIO(
        const ftk::Path& path,
        OTIO_NS::ErrorStatus* errorStatus,
        const std::shared_ptr<ftk::LogSystem>& logSystem)
    {
        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> out;
        const std::string fileName = path.get();
        const std::string ext = ftk::toLower(path.getExt());
        if (".otio" == ext)
        {
            out = dynamic_cast<OTIO_NS::Timeline*>(
                OTIO_NS::Timeline::from_json_file(fileName, errorStatus));
        }
        else if (".otioz" == ext)
        {
            auto fileIO = ftk::FileIO::create(fileName, ftk::FileMode::Read);

            ZipReader zipReader(logSystem);
            zipReader.open(fileName, fileIO->getMemStart(), fileIO->getSize());

            std::string json = zipReader.readText("content.otio");
            out = dynamic_cast<OTIO_NS::Timeline*>(
                OTIO_NS::Timeline::from_json_string(json, errorStatus));

            for (auto clip : out->find_children<OTIO_NS::Clip>())
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

                    auto memReference = new ZipMemRef(
                        fileIO,
                        externalReference->target_url(),
                        fileIO->getMemStart() + entry->offset,
                        entry->size,
                        externalReference->available_range(),
                        externalReference->metadata());
                    clip->set_media_reference(memReference);
                }
                else if (auto imageSeqReference =
                    dynamic_cast<OTIO_NS::ImageSequenceReference*>(clip->media_reference()))
                {
                    std::vector<const uint8_t*> memory;
                    std::vector<size_t> memory_sizes;
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

                        memory.push_back(fileIO->getMemStart() + entry->offset);
                        memory_sizes.push_back(entry->size);
                    }
                    auto memoryReference = new SeqZipMemRef(
                        fileIO,
                        imageSeqReference->target_url_for_image_number(0),
                        memory,
                        memory_sizes,
                        imageSeqReference->available_range(),
                        imageSeqReference->metadata());
                    clip->set_media_reference(memoryReference);
                }
            }
        }
        return out;
    }

    OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> create(
        const std::shared_ptr<ftk::Context>& context,
        const ftk::Path& path,
        const Options& options)
    {
        return create(context, path, ftk::Path(), options);
    }

    OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> create(
        const std::shared_ptr<ftk::Context>& context,
        const ftk::Path& inputPath,
        const ftk::Path& inputAudioPath,
        const Options& options)
    {
        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> out;

        std::string error;
        ftk::Path path = inputPath;
        ftk::Path audioPath = inputAudioPath;

        auto logSystem = context->getLogSystem();
        logSystem->print(
            "tl::create",
            ftk::Format(
                "\n"
                "    Path: {0}\n"
                "    Audio path: {1}").
            arg(path.get()).
            arg(audioPath.get()));

        try
        {
            auto ioSystem = context->getSystem<ReadSystem>();

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
                    audioPath = getAudioPath(
                        context,
                        path,
                        options.imageSeqAudio,
                        options.imageSeqAudioExts,
                        options.imageSeqAudioFileName,
                        options.pathOptions);
                }
            }

            // Read the file.
            if (auto read = ioSystem->read(path, options.ioOptions))
            {
                const auto info = read->getInfo().get();
                OTIO_NS::RationalTime startTime = invalidTime;
                OTIO_NS::Track* videoTrack = nullptr;
                OTIO_NS::Track* audioTrack = nullptr;
                OTIO_NS::ErrorStatus errorStatus;

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
                    videoTrack->append_child(videoClip, &errorStatus);
                    if (OTIO_NS::is_error(errorStatus))
                    {
                        throw std::runtime_error("Cannot append child");
                    }
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
                        audioTrack->append_child(audioClip, &errorStatus);
                        if (OTIO_NS::is_error(errorStatus))
                        {
                            throw std::runtime_error("Cannot append child");
                        }
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
                    audioTrack->append_child(audioClip, &errorStatus);
                    if (OTIO_NS::is_error(errorStatus))
                    {
                        throw std::runtime_error("Cannot append child");
                    }
                }

                // Create the stack.
                auto otioStack = new OTIO_NS::Stack;
                if (videoTrack)
                {
                    otioStack->append_child(videoTrack, &errorStatus);
                    if (OTIO_NS::is_error(errorStatus))
                    {
                        throw std::runtime_error("Cannot append child");
                    }
                }
                if (audioTrack)
                {
                    otioStack->append_child(audioTrack, &errorStatus);
                    if (OTIO_NS::is_error(errorStatus))
                    {
                        throw std::runtime_error("Cannot append child");
                    }
                }

                // Create the timeline.
                out = new OTIO_NS::Timeline(path.get());
                out->set_tracks(otioStack);
                if (isValid(startTime))
                {
                    out->set_global_start_time(startTime);
                }
            }
        }
        catch (const std::exception& e)
        {
            error = e.what();
        }

        // Is the input an OTIO file?
        if (!out)
        {
            OTIO_NS::ErrorStatus errorStatus;
            out = readOTIO(path, &errorStatus, logSystem);
            if (OTIO_NS::is_error(errorStatus))
            {
                out = nullptr;
                error = errorStatus.full_description;
            }
            else if (!out)
            {
                error = ftk::Format("Cannot read timeline: \"{0}\"").arg(path.get());
            }
        }
        if (!out)
        {
            throw std::runtime_error(error);
        }

        OTIO_NS::AnyDictionary dict;
        dict["path"] = path.get();
        dict["audioPath"] = audioPath.get();
        out->metadata()["tlRender"] = dict;

        return out;
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
        auto otioTimeline = tl::create(context, path, options);
        out->_init(context, otioTimeline, options);
        return out;
    }

    std::shared_ptr<Timeline> Timeline::create(
        const std::shared_ptr<ftk::Context>& context,
        const ftk::Path& path,
        const ftk::Path& audioPath,
        const Options& options)
    {
        auto out = std::shared_ptr<Timeline>(new Timeline);
        auto otioTimeline = tl::create(
            context,
            path,
            audioPath,
            options);
        out->_init(context, otioTimeline, options);
        return out;
    }

    std::shared_ptr<Timeline> Timeline::create(
        const std::shared_ptr<ftk::Context>& context,
        const std::string& fileName,
        const Options& options)
    {
        auto out = std::shared_ptr<Timeline>(new Timeline);
        auto otioTimeline = tl::create(
            context,
            ftk::Path(fileName, options.pathOptions),
            options);
        out->_init(context, otioTimeline, options);
        return out;
    }

    std::shared_ptr<Timeline> Timeline::create(
        const std::shared_ptr<ftk::Context>& context,
        const std::string& fileName,
        const std::string& audioFileName,
        const Options& options)
    {
        auto out = std::shared_ptr<Timeline>(new Timeline);
        auto otioTimeline = tl::create(
            context,
            ftk::Path(fileName, options.pathOptions),
            ftk::Path(audioFileName, options.pathOptions),
            options);
        out->_init(context, otioTimeline, options);
        return out;
    }
}
