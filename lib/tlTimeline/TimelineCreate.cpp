// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlTimeline/TimelinePrivate.h>

#include <tlTimeline/MemRef.h>
#include <tlTimeline/Util.h>

#include <tlIO/System.h>

#include <tlCore/URL.h>

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
    namespace timeline
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
                auto ioSystem = context->getSystem<io::ReadSystem>();
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
                        listOptions.filterExt.insert(
                            imageSeqAudioExts.begin(),
                            imageSeqAudioExts.end());
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

        class ZipReader
        {
        public:
            ZipReader(const std::string& fileName)
            {
                reader = mz_zip_reader_create();
                if (!reader)
                {
                    throw std::runtime_error(ftk::Format(
                        "Cannot create zip reader: \"{0}\"").arg(fileName));
                }
                int32_t err = mz_zip_reader_open_file(reader, fileName.c_str());
                if (err != MZ_OK)
                {
                    throw std::runtime_error(ftk::Format(
                        "Cannot open zip reader: \"{0}\"").arg(fileName));
                }
            }

            ~ZipReader()
            {
                mz_zip_reader_delete(&reader);
            }

            void* reader = nullptr;
        };

        class ZipReaderFile
        {
        public:
            ZipReaderFile(void* reader, const std::string& fileName) :
                reader(reader)
            {
                int32_t err = mz_zip_reader_entry_open(reader);
                if (err != MZ_OK)
                {
                    throw std::runtime_error(ftk::Format(
                        "Cannot open zip entry: \"{0}\"").arg(fileName));
                }
            }

            ~ZipReaderFile()
            {
                mz_zip_reader_entry_close(reader);
            }

            void* reader = nullptr;
        };

        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> readOTIO(
            const ftk::Path& path,
            OTIO_NS::ErrorStatus* errorStatus)
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
                {
                    ZipReader zipReader(fileName);

                    const std::string contentFileName = "content.otio";
                    int32_t err = mz_zip_reader_locate_entry(
                        zipReader.reader,
                        contentFileName.c_str(),
                        0);
                    if (err != MZ_OK)
                    {
                        throw std::runtime_error(ftk::Format(
                            "Cannot find zip entry: \"{0}\"").arg(contentFileName));
                    }
                    mz_zip_file* fileInfo = nullptr;
                    err = mz_zip_reader_entry_get_info(zipReader.reader, &fileInfo);
                    if (err != MZ_OK)
                    {
                        throw std::runtime_error(ftk::Format(
                            "Cannot get zip entry information: \"{0}\"").arg(contentFileName));
                    }
                    ZipReaderFile zipReaderFile(zipReader.reader, contentFileName);
                    std::vector<char> buf;
                    buf.resize(fileInfo->uncompressed_size + 1);
                    err = mz_zip_reader_entry_read(
                        zipReader.reader,
                        buf.data(),
                        fileInfo->uncompressed_size);
                    if (err != fileInfo->uncompressed_size)
                    {
                        throw std::runtime_error(ftk::Format(
                            "Cannot read zip entry: \"{0}\"").arg(contentFileName));
                    }
                    buf[fileInfo->uncompressed_size] = 0;

                    out = dynamic_cast<OTIO_NS::Timeline*>(
                        OTIO_NS::Timeline::from_json_string(buf.data(), errorStatus));

                    auto fileIO = ftk::FileIO::create(fileName, ftk::FileMode::Read);
                    for (auto clip : out->find_children<OTIO_NS::Clip>())
                    {
                        if (auto externalReference =
                            dynamic_cast<OTIO_NS::ExternalReference*>(clip->media_reference()))
                        {
                            const std::string mediaFileName = ftk::Path(
                                url::decode(externalReference->target_url())).get();

                            int32_t err = mz_zip_reader_locate_entry(zipReader.reader, mediaFileName.c_str(), 0);
                            if (err != MZ_OK)
                            {
                                throw std::runtime_error(ftk::Format(
                                    "Cannot find zip entry: \"{0}\"").arg(mediaFileName));
                            }
                            err = mz_zip_reader_entry_get_info(zipReader.reader, &fileInfo);
                            if (err != MZ_OK)
                            {
                                throw std::runtime_error(ftk::Format(
                                    "Cannot get zip entry information: \"{0}\"").arg(mediaFileName));
                            }

                            const size_t headerSize =
                                30 +
                                fileInfo->filename_size +
                                fileInfo->extrafield_size;
                            auto memReference = new ZipMemRef(
                                fileIO,
                                externalReference->target_url(),
                                fileIO->getMemStart() +
                                fileInfo->disk_offset +
                                headerSize,
                                fileInfo->uncompressed_size,
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
                                    url::decode(imageSeqReference->target_url_for_image_number(number))).get();

                                int32_t err = mz_zip_reader_locate_entry(zipReader.reader, mediaFileName.c_str(), 0);
                                if (err != MZ_OK)
                                {
                                    throw std::runtime_error(ftk::Format(
                                        "Cannot find zip entry: \"{0}\"").arg(mediaFileName));
                                }
                                err = mz_zip_reader_entry_get_info(zipReader.reader, &fileInfo);
                                if (err != MZ_OK)
                                {
                                    throw std::runtime_error(ftk::Format(
                                        "Cannot get zip entry information: \"{0}\"").arg(mediaFileName));
                                }

                                const size_t headerSize =
                                    30 +
                                    fileInfo->filename_size +
                                    fileInfo->extrafield_size;
                                memory.push_back(
                                    fileIO->getMemStart() +
                                    fileInfo->disk_offset +
                                    headerSize);
                                memory_sizes.push_back(fileInfo->uncompressed_size);
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
            try
            {
                auto ioSystem = context->getSystem<io::ReadSystem>();

                // Is the input a sequence?
                ftk::expandSeq(
                    std::filesystem::u8path(path.get()),
                    path,
                    options.pathOptions);
                if (!path.getFrames().equal())
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

                // Is the input a video or audio file?
                if (auto read = ioSystem->read(path, options.ioOptions))
                {
                    const auto info = read->getInfo().get();

                    OTIO_NS::RationalTime startTime = time::invalidTime;
                    OTIO_NS::Track* videoTrack = nullptr;
                    OTIO_NS::Track* audioTrack = nullptr;
                    OTIO_NS::ErrorStatus errorStatus;

                    // Read the video.
                    if (!info.video.empty())
                    {
                        startTime = info.videoTime.start_time();
                        auto videoClip = new OTIO_NS::Clip;
                        videoClip->set_source_range(info.videoTime);
                        if (!path.getFrames().equal())
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
                    if (time::isValid(startTime))
                    {
                        out->set_global_start_time(startTime);
                    }
                }
            }
            catch (const std::exception& e)
            {
                error = e.what();
            }

            auto logSystem = context->getLogSystem();
            logSystem->print(
                "tl::timeline::create",
                ftk::Format(
                    "\n"
                    "    Create from path: {0}\n"
                    "    Audio path: {1}").
                arg(path.get()).
                arg(audioPath.get()));

            // Is the input an OTIO file?
            if (!out)
            {
                OTIO_NS::ErrorStatus errorStatus;
                out = readOTIO(path, &errorStatus);
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
            const std::string& fileName,
            const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            auto otioTimeline = timeline::create(
                context,
                ftk::Path(fileName, options.pathOptions),
                options);
            out->_init(context, otioTimeline, options);
            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const std::shared_ptr<ftk::Context>& context,
            const ftk::Path& path,
            const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            auto otioTimeline = timeline::create(context, path, options);
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
            auto otioTimeline = timeline::create(
                context,
                ftk::Path(fileName, options.pathOptions),
                ftk::Path(audioFileName, options.pathOptions),
                options);
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
            auto otioTimeline = timeline::create(
                context,
                path,
                audioPath,
                options);
            out->_init(context, otioTimeline, options);
            return out;
        }
    }
}
