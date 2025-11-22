// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlTimelineTest/MemRefTest.h>

#include <tlTimeline/MemRef.h>

#include <ftk/Core/Assert.h>
#include <ftk/Core/String.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        MemRefTest::MemRefTest(const std::shared_ptr<ftk::Context>& context) :
            ITest(context, "timeline_tests::MemRefTest")
        {}

        std::shared_ptr<MemRefTest> MemRefTest::create(const std::shared_ptr<ftk::Context>& context)
        {
            return std::shared_ptr<MemRefTest>(new MemRefTest(context));
        }

        void MemRefTest::run()
        {
            {
                OTIO_NS::SerializableObject::Retainer<RawMemRef> v(new RawMemRef);
                v->set_target_url("url");
                FTK_ASSERT("url" == v->target_url());
                std::vector<uint8_t> mem(100, 0);
                v->set_mem(mem.data(), mem.size());
                FTK_ASSERT(v->mem() == mem.data());
                FTK_ASSERT(v->mem_size() == mem.size());
            }
            {
                OTIO_NS::SerializableObject::Retainer<SharedMemRef> v(new SharedMemRef);
                v->set_target_url("url");
                FTK_ASSERT("url" == v->target_url());
                std::shared_ptr<std::vector<uint8_t> > mem(new std::vector<uint8_t>(100, 0));
                v->set_mem(mem);
                FTK_ASSERT(v->mem() == mem);
            }
            {
                OTIO_NS::SerializableObject::Retainer<SeqRawMemRef> v(new SeqRawMemRef);
                v->set_target_url("url");
                FTK_ASSERT("url" == v->target_url());
                std::vector<std::shared_ptr<std::vector<uint8_t> > > dataList;
                std::vector<const uint8_t*> mem;
                std::vector<size_t> sizes;
                for (size_t i = 0; i < 10; ++i)
                {
                    std::shared_ptr<std::vector<uint8_t> > data(new std::vector<uint8_t>(100, 0));
                    dataList.push_back(data);
                    mem.push_back(data->data());
                    sizes.push_back(100);
                }
                v->set_mem(mem, sizes);
                FTK_ASSERT(v->mem() == mem);
                FTK_ASSERT(v->mem_sizes() == sizes);
            }
            {
                OTIO_NS::SerializableObject::Retainer<SeqSharedMemRef> v(new SeqSharedMemRef);
                v->set_target_url("url");
                FTK_ASSERT("url" == v->target_url());
                std::vector<std::shared_ptr<std::vector<uint8_t> > > mem;
                std::vector<size_t> sizes;
                for (size_t i = 0; i < 10; ++i)
                {
                    std::shared_ptr<std::vector<uint8_t> > data(new std::vector<uint8_t>(100, 0));
                    mem.push_back(data);
                }
                v->set_mem(mem);
                FTK_ASSERT(v->mem() == mem);
            }
            {
                OTIO_NS::SerializableObject::Retainer<ZipMemRef> v(new ZipMemRef);
            }
            {
                OTIO_NS::SerializableObject::Retainer<SeqZipMemRef> v(new SeqZipMemRef);
            }
        }
    }
}
