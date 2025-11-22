// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlTimeline/MemRef.h>

namespace tl
{
    namespace timeline
    {
        RawMemRef::RawMemRef(
            const std::string& target_url,
            const uint8_t* mem,
            size_t mem_size,
            const std::optional<OTIO_NS::TimeRange>& available_range,
            const OTIO_NS::AnyDictionary& metadata) :
            OTIO_NS::MediaReference(std::string(), available_range, metadata),
            _target_url(target_url),
            _mem(mem),
            _mem_size(mem_size)
        {}

        RawMemRef::~RawMemRef()
        {}

        const std::string& RawMemRef::target_url() const noexcept
        {
            return _target_url;
        }

        void RawMemRef::set_target_url(const std::string& value)
        {
            _target_url = value;
        }

        const uint8_t* RawMemRef::mem() const noexcept
        {
            return _mem;
        }

        size_t RawMemRef::mem_size() const noexcept
        {
            return _mem_size;
        }

        void RawMemRef::set_mem(const uint8_t* mem, size_t mem_size)
        {
            _mem = mem;
            _mem_size = mem_size;
        }

        SharedMemRef::SharedMemRef(
            const std::string& target_url,
            const std::shared_ptr<MemRefData>& mem,
            const std::optional<OTIO_NS::TimeRange>& available_range,
            const OTIO_NS::AnyDictionary& metadata) :
            OTIO_NS::MediaReference(std::string(), available_range, metadata),
            _target_url(target_url),
            _mem(mem)
        {}

        SharedMemRef::~SharedMemRef()
        {}

        const std::string& SharedMemRef::target_url() const noexcept
        {
            return _target_url;
        }

        void SharedMemRef::set_target_url(const std::string& value)
        {
            _target_url = value;
        }

        const std::shared_ptr<MemRefData>& SharedMemRef::mem() const noexcept
        {
            return _mem;
        }

        void SharedMemRef::set_mem(const std::shared_ptr<MemRefData>& mem)
        {
            _mem = mem;
        }

        SeqRawMemRef::SeqRawMemRef(
            const std::string& target_url,
            const std::vector<const uint8_t*>& mem,
            const std::vector<size_t> mem_sizes,
            const std::optional<OTIO_NS::TimeRange>& available_range,
            const OTIO_NS::AnyDictionary& metadata) :
            OTIO_NS::MediaReference(std::string(), available_range, metadata),
            _target_url(target_url),
            _mem(mem),
            _mem_sizes(mem_sizes)
        {}

        SeqRawMemRef::~SeqRawMemRef()
        {}

        const std::string& SeqRawMemRef::target_url() const noexcept
        {
            return _target_url;
        }

        void SeqRawMemRef::set_target_url(const std::string& value)
        {
            _target_url = value;
        }

        const std::vector<const uint8_t*>& SeqRawMemRef::mem() const noexcept
        {
            return _mem;
        }

        const std::vector<size_t>& SeqRawMemRef::mem_sizes() const noexcept
        {
            return _mem_sizes;
        }

        void SeqRawMemRef::set_mem(
            const std::vector<const uint8_t*>& mem,
            const std::vector<size_t>& mem_sizes)
        {
            _mem = mem;
            _mem_sizes = mem_sizes;
        }

        SeqSharedMemRef::SeqSharedMemRef(
            const std::string& target_url,
            const std::vector<std::shared_ptr<MemRefData> >& mem,
            const std::optional<OTIO_NS::TimeRange>& available_range,
            const OTIO_NS::AnyDictionary& metadata) :
            OTIO_NS::MediaReference(std::string(), available_range, metadata),
            _target_url(target_url),
            _mem(mem)
        {}

        SeqSharedMemRef::~SeqSharedMemRef()
        {}

        const std::string& SeqSharedMemRef::target_url() const noexcept
        {
            return _target_url;
        }

        void SeqSharedMemRef::set_target_url(const std::string& value)
        {
            _target_url = value;
        }

        const std::vector<std::shared_ptr<MemRefData> >& SeqSharedMemRef::mem() const noexcept
        {
            return _mem;
        }

        void SeqSharedMemRef::set_mem(
            const std::vector<std::shared_ptr<MemRefData> >& mem)
        {
            _mem = mem;
        }

        ZipMemRef::ZipMemRef(
            const std::shared_ptr<ftk::FileIO>& file_io,
            const std::string& target_url,
            const uint8_t* mem,
            size_t mem_size,
            const std::optional<OTIO_NS::TimeRange>& available_range,
            const OTIO_NS::AnyDictionary& metadata) :
            RawMemRef(target_url, mem, mem_size, available_range, metadata),
            _file_io(file_io)
        {}

        ZipMemRef::~ZipMemRef()
        {}

        const std::shared_ptr<ftk::FileIO>& ZipMemRef::file_io() const noexcept
        {
            return _file_io;
        }

        void ZipMemRef::set_file_io(const std::shared_ptr<ftk::FileIO>& file_io)
        {
            _file_io = file_io;
        }

        SeqZipMemRef::SeqZipMemRef(
            const std::shared_ptr<ftk::FileIO>& file_io,
            const std::string& target_url,
            const std::vector<const uint8_t*>& mem,
            const std::vector<size_t> mem_sizes,
            const std::optional<OTIO_NS::TimeRange>& available_range,
            const OTIO_NS::AnyDictionary& metadata) :
            SeqRawMemRef(target_url, mem, mem_sizes, available_range, metadata),
            _file_io(file_io)
        {}

        SeqZipMemRef::~SeqZipMemRef()
        {}

        const std::shared_ptr<ftk::FileIO>& SeqZipMemRef::file_io() const noexcept
        {
            return _file_io;
        }

        void SeqZipMemRef::set_file_io(const std::shared_ptr<ftk::FileIO>&file_io)
        {
            _file_io = file_io;
        }
    }
}
