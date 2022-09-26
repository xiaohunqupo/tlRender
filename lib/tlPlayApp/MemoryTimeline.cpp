// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlayApp/MemoryTimeline.h>

#include <tlCore/StringFormat.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>

namespace tl
{
    namespace play
    {
        void createMemoryTimeline(
            otio::Timeline* otioTimeline,
            const std::string& directory,
            const file::PathOptions& pathOptions)
        {
            for (auto clip : otioTimeline->children_if<otio::Clip>())
            {
                if (auto externalReference = dynamic_cast<otio::ExternalReference*>(clip->media_reference()))
                {
                    const auto path = timeline::getPath(externalReference->target_url(), directory, pathOptions);
                    auto fileIO = file::FileIO::create(path.get(), file::Mode::Read);
                    const size_t size = fileIO->getSize();
                    auto memory = std::make_shared<timeline::MemoryReferenceData>();
                    memory->resize(size);
                    fileIO->read(memory->data(), size);
                    auto memoryReference = new timeline::MemoryReference(
                        externalReference->target_url(),
                        memory,
                        clip->available_range(),
                        externalReference->metadata());
                    clip->set_media_reference(memoryReference);
                }
                else if (auto imageSequenceRefence = dynamic_cast<otio::ImageSequenceReference*>(clip->media_reference()))
                {
                    int padding = imageSequenceRefence->frame_zero_padding();
                    std::string number;
                    std::stringstream ss;
                    ss << imageSequenceRefence->target_url_base() <<
                        imageSequenceRefence->name_prefix() <<
                        std::setfill('0') << std::setw(padding) << imageSequenceRefence->start_frame() <<
                        imageSequenceRefence->name_suffix();
                    const auto path = timeline::getPath(ss.str(), directory, pathOptions);
                    std::vector<std::shared_ptr<timeline::MemoryReferenceData> > memoryList;
                    const auto range = clip->trimmed_range();
                    for (
                        int64_t frame = imageSequenceRefence->start_frame();
                        frame < imageSequenceRefence->start_frame() + range.duration().value();
                        ++frame)
                    {
                        const auto& fileName = path.get(frame);
                        auto fileIO = file::FileIO::create(fileName, file::Mode::Read);
                        const size_t size = fileIO->getSize();
                        auto memory = std::make_shared<timeline::MemoryReferenceData>();
                        memory->resize(size);
                        fileIO->read(memory->data(), size);
                        memoryList.push_back(memory);
                    }
                    auto memorySequenceReference = new timeline::MemorySequenceReference(
                        path.get(),
                        memoryList,
                        clip->available_range(),
                        imageSequenceRefence->metadata());
                    clip->set_media_reference(memorySequenceReference);
                }
            }
        }
    }
}
