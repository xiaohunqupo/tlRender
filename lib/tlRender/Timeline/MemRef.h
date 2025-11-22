// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Core/Time.h>

#include <ftk/Core/FileIO.h>

#include <opentimelineio/mediaReference.h>
#include <opentimelineio/timeline.h>

#include <memory>

namespace tl
{
    namespace timeline
    {
        //! Memory reference data.
        typedef std::vector<uint8_t> MemRefData;

        //! Read references from raw memory pointers.
        class RawMemRef : public OTIO_NS::MediaReference
        {
        public:
            struct Schema
            {
                static auto constexpr name = "RawMemRef";
                static int constexpr version = 1;
            };

            RawMemRef(
                const std::string& target_url = std::string(),
                const uint8_t* mem = nullptr,
                size_t mem_size = 0,
                const std::optional<OTIO_NS::TimeRange>& available_range = std::nullopt,
                const OTIO_NS::AnyDictionary& metadata = OTIO_NS::AnyDictionary());

            const std::string& target_url() const noexcept;

            void set_target_url(const std::string&);

            const uint8_t* mem() const noexcept;

            size_t mem_size() const noexcept;

            void set_mem(const uint8_t* mem, size_t mem_size);

        protected:
            virtual ~RawMemRef();

            std::string _target_url;
            const uint8_t* _mem = nullptr;
            size_t _mem_size = 0;
        };

        //! Read references from a shared memory pointer.
        class SharedMemRef : public OTIO_NS::MediaReference
        {
        public:
            struct Schema
            {
                static auto constexpr name = "SharedMemRef";
                static int constexpr version = 1;
            };

            SharedMemRef(
                const std::string& target_url = std::string(),
                const std::shared_ptr<MemRefData>& mem = nullptr,
                const std::optional<OTIO_NS::TimeRange>& available_range = std::nullopt,
                const OTIO_NS::AnyDictionary& metadata = OTIO_NS::AnyDictionary());

            const std::string& target_url() const noexcept;

            void set_target_url(const std::string&);

            const std::shared_ptr<MemRefData>& mem() const noexcept;

            void set_mem(const std::shared_ptr<MemRefData>&);

        protected:
            virtual ~SharedMemRef();

            std::string _target_url;
            std::shared_ptr<MemRefData> _mem;
        };

        //! Read sequence references from raw memory pointers.
        class SeqRawMemRef : public OTIO_NS::MediaReference
        {
        public:
            struct Schema
            {
                static auto constexpr name = "SeqRawMemRef";
                static int constexpr version = 1;
            };

            SeqRawMemRef(
                const std::string& target_url = std::string(),
                const std::vector<const uint8_t*>& mem = {},
                const std::vector<size_t> mem_sizes = {},
                const std::optional<OTIO_NS::TimeRange>& available_range = std::nullopt,
                const OTIO_NS::AnyDictionary& metadata = OTIO_NS::AnyDictionary());

            const std::string& target_url() const noexcept;

            void set_target_url(const std::string&);

            const std::vector<const uint8_t*>& mem() const noexcept;

            const std::vector<size_t>& mem_sizes() const noexcept;

            void set_mem(
                const std::vector<const uint8_t*>& mem,
                const std::vector<size_t>& mem_sizes);

        protected:
            virtual ~SeqRawMemRef();

            std::string _target_url;
            std::vector<const uint8_t*> _mem;
            std::vector<size_t> _mem_sizes;
        };

        //! Read sequence references from shared memory pointers.
        class SeqSharedMemRef : public OTIO_NS::MediaReference
        {
        public:
            struct Schema
            {
                static auto constexpr name = "SeqSharedMemRef";
                static int constexpr version = 1;
            };

            SeqSharedMemRef(
                const std::string& target_url = std::string(),
                const std::vector<std::shared_ptr<MemRefData> >& mem = {},
                const std::optional<OTIO_NS::TimeRange>& available_range = std::nullopt,
                const OTIO_NS::AnyDictionary& metadata = OTIO_NS::AnyDictionary());

            const std::string& target_url() const noexcept;

            void set_target_url(const std::string&);

            const std::vector<std::shared_ptr<MemRefData> >& mem() const noexcept;

            void set_mem(
                const std::vector<std::shared_ptr<MemRefData> >&);

        protected:
            virtual ~SeqSharedMemRef();

            std::string _target_url;
            std::vector<std::shared_ptr<MemRefData> > _mem;
        };

        //! Zip file memory reference for .otioz support.
        class ZipMemRef : public RawMemRef
        {
        public:
            struct Schema
            {
                static auto constexpr name = "ZipMemRef";
                static int constexpr version = 1;
            };

            ZipMemRef(
                const std::shared_ptr<ftk::FileIO>& file_io = nullptr,
                const std::string& target_url = std::string(),
                const uint8_t* mem = nullptr,
                size_t mem_size = 0,
                const std::optional<OTIO_NS::TimeRange>& available_range = std::nullopt,
                const OTIO_NS::AnyDictionary& metadata = OTIO_NS::AnyDictionary());

            const std::shared_ptr<ftk::FileIO>& file_io() const noexcept;

            void set_file_io(const std::shared_ptr<ftk::FileIO>&);

        protected:
            virtual ~ZipMemRef();

            std::shared_ptr<ftk::FileIO> _file_io;
        };

        //! Zip file memory sequence reference for .otioz support.
        class SeqZipMemRef : public SeqRawMemRef
        {
        public:
            struct Schema
            {
                static auto constexpr name = "SeqZipMemRef";
                static int constexpr version = 1;
            };

            SeqZipMemRef(
                const std::shared_ptr<ftk::FileIO>& file_io = nullptr,
                const std::string& target_url = std::string(),
                const std::vector<const uint8_t*>& mem = {},
                const std::vector<size_t> mem_sizes = {},
                const std::optional<OTIO_NS::TimeRange>& available_range = std::nullopt,
                const OTIO_NS::AnyDictionary& metadata = OTIO_NS::AnyDictionary());

            const std::shared_ptr<ftk::FileIO>& file_io() const noexcept;

            void set_file_io(const std::shared_ptr<ftk::FileIO>&);

        protected:
            virtual ~SeqZipMemRef();

            std::shared_ptr<ftk::FileIO> _file_io;
        };
    }
}
