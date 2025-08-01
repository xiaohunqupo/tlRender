// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/MemoryReferenceTest.h>

#include <tlTimeline/MemoryReference.h>

#include <feather-tk/core/Assert.h>
#include <feather-tk/core/String.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        MemoryReferenceTest::MemoryReferenceTest(const std::shared_ptr<feather_tk::Context>& context) :
            ITest(context, "timeline_tests::MemoryReferenceTest")
        {}

        std::shared_ptr<MemoryReferenceTest> MemoryReferenceTest::create(const std::shared_ptr<feather_tk::Context>& context)
        {
            return std::shared_ptr<MemoryReferenceTest>(new MemoryReferenceTest(context));
        }

        void MemoryReferenceTest::run()
        {
            {
                OTIO_NS::SerializableObject::Retainer<RawMemoryReference> v(new RawMemoryReference);
                v->set_target_url("url");
                FEATHER_TK_ASSERT("url" == v->target_url());
                std::vector<uint8_t> memory(100, 0);
                v->set_memory(memory.data(), memory.size());
                FEATHER_TK_ASSERT(v->memory() == memory.data());
                FEATHER_TK_ASSERT(v->memory_size() == memory.size());
            }
            {
                OTIO_NS::SerializableObject::Retainer<SharedMemoryReference> v(new SharedMemoryReference);
                v->set_target_url("url");
                FEATHER_TK_ASSERT("url" == v->target_url());
                std::shared_ptr<std::vector<uint8_t> > memory(new std::vector<uint8_t>(100, 0));
                v->set_memory(memory);
                FEATHER_TK_ASSERT(v->memory() == memory);
            }
            {
                OTIO_NS::SerializableObject::Retainer<RawMemorySequenceReference> v(new RawMemorySequenceReference);
                v->set_target_url("url");
                FEATHER_TK_ASSERT("url" == v->target_url());
                std::vector<std::shared_ptr<std::vector<uint8_t> > > dataList;
                std::vector<const uint8_t*> memory;
                std::vector<size_t> sizes;
                for (size_t i = 0; i < 10; ++i)
                {
                    std::shared_ptr<std::vector<uint8_t> > data(new std::vector<uint8_t>(100, 0));
                    dataList.push_back(data);
                    memory.push_back(data->data());
                    sizes.push_back(100);
                }
                v->set_memory(memory, sizes);
                FEATHER_TK_ASSERT(v->memory() == memory);
                FEATHER_TK_ASSERT(v->memory_sizes() == sizes);
            }
            {
                OTIO_NS::SerializableObject::Retainer<SharedMemorySequenceReference> v(new SharedMemorySequenceReference);
                v->set_target_url("url");
                FEATHER_TK_ASSERT("url" == v->target_url());
                std::vector<std::shared_ptr<std::vector<uint8_t> > > memory;
                std::vector<size_t> sizes;
                for (size_t i = 0; i < 10; ++i)
                {
                    std::shared_ptr<std::vector<uint8_t> > data(new std::vector<uint8_t>(100, 0));
                    memory.push_back(data);
                }
                v->set_memory(memory);
                FEATHER_TK_ASSERT(v->memory() == memory);
            }
            {
                OTIO_NS::SerializableObject::Retainer<ZipMemoryReference> v(new ZipMemoryReference);
            }
            {
                OTIO_NS::SerializableObject::Retainer<ZipMemorySequenceReference> v(new ZipMemorySequenceReference);
            }
        }
    }
}
