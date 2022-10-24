// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 Darby Johnston
// All rights reserved.

#include <tlTimeline/MemoryReference.h>

namespace tl
{
    namespace timeline
    {
        RawMemoryReference::RawMemoryReference(
            const std::string& target_url,
            const uint8_t* memory,
            size_t memory_size,
            const otio::optional<otio::TimeRange>& available_range,
            const otio::AnyDictionary& metadata) :
            otio::MediaReference(std::string(), available_range, metadata),
            _target_url(target_url),
            _memory(memory),
            _memory_size(memory_size)
        {}

        RawMemoryReference::~RawMemoryReference()
        {}

        const std::string& RawMemoryReference::target_url() const noexcept
        {
            return _target_url;
        }

        void RawMemoryReference::set_target_url(const std::string& value)
        {
            _target_url = value;
        }

        const uint8_t* RawMemoryReference::memory() const noexcept
        {
            return _memory;
        }

        size_t RawMemoryReference::memory_size() const noexcept
        {
            return _memory_size;
        }

        void RawMemoryReference::set_memory(const uint8_t* memory, size_t memory_size)
        {
            _memory = memory;
            _memory_size = memory_size;
        }

        SharedMemoryReference::SharedMemoryReference(
            const std::string& target_url,
            const std::shared_ptr<MemoryReferenceData>& memory,
            const otio::optional<otio::TimeRange>& available_range,
            const otio::AnyDictionary& metadata) :
            otio::MediaReference(std::string(), available_range, metadata),
            _target_url(target_url),
            _memory(memory)
        {}

        SharedMemoryReference::~SharedMemoryReference()
        {}

        const std::string& SharedMemoryReference::target_url() const noexcept
        {
            return _target_url;
        }

        void SharedMemoryReference::set_target_url(const std::string& value)
        {
            _target_url = value;
        }

        const std::shared_ptr<MemoryReferenceData>& SharedMemoryReference::memory() const noexcept
        {
            return _memory;
        }

        void SharedMemoryReference::set_memory(const std::shared_ptr<MemoryReferenceData>& memory)
        {
            _memory = memory;
        }

        RawMemorySequenceReference::RawMemorySequenceReference(
            const std::string& target_url,
            const std::vector<const uint8_t*>& memory,
            const std::vector<size_t> memory_sizes,
            const otio::optional<otio::TimeRange>& available_range,
            const otio::AnyDictionary& metadata) :
            otio::MediaReference(std::string(), available_range, metadata),
            _target_url(target_url),
            _memory(memory),
            _memory_sizes(memory_sizes)
        {}

        RawMemorySequenceReference::~RawMemorySequenceReference()
        {}

        const std::string& RawMemorySequenceReference::target_url() const noexcept
        {
            return _target_url;
        }

        void RawMemorySequenceReference::set_target_url(const std::string& value)
        {
            _target_url = value;
        }

        const std::vector<const uint8_t*>& RawMemorySequenceReference::memory() const noexcept
        {
            return _memory;
        }

        const std::vector<size_t>& RawMemorySequenceReference::memory_sizes() const noexcept
        {
            return _memory_sizes;
        }

        void RawMemorySequenceReference::set_memory(
            const std::vector<const uint8_t*>& memory,
            const std::vector<size_t>& memory_sizes)
        {
            _memory = memory;
            _memory_sizes = memory_sizes;
        }

        SharedMemorySequenceReference::SharedMemorySequenceReference(
            const std::string& target_url,
            const std::vector<std::shared_ptr<MemoryReferenceData> >& memory,
            const otio::optional<otio::TimeRange>& available_range,
            const otio::AnyDictionary& metadata) :
            otio::MediaReference(std::string(), available_range, metadata),
            _target_url(target_url),
            _memory(memory)
        {}

        SharedMemorySequenceReference::~SharedMemorySequenceReference()
        {}

        const std::string& SharedMemorySequenceReference::target_url() const noexcept
        {
            return _target_url;
        }

        void SharedMemorySequenceReference::set_target_url(const std::string& value)
        {
            _target_url = value;
        }

        const std::vector<std::shared_ptr<MemoryReferenceData> > & SharedMemorySequenceReference::memory() const noexcept
        {
            return _memory;
        }

        void SharedMemorySequenceReference::set_memory(
            const std::vector<std::shared_ptr<MemoryReferenceData> >& memory)
        {
            _memory = memory;
        }
    }
}