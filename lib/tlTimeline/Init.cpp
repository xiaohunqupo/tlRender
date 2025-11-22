// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlTimeline/Init.h>

#include <tlTimeline/MemRef.h>

#include <tlIO/Init.h>

#include <ftk/Core/Context.h>
#include <ftk/Core/Format.h>

#include <opentimelineio/typeRegistry.h>

namespace tl
{
    namespace timeline
    {
        void init(const std::shared_ptr<ftk::Context>& context)
        {
            io::init(context);
            System::create(context);
        }

        System::System(const std::shared_ptr<ftk::Context>& context) :
            ISystem(context, "tl::timeline::System")
        {
            const std::vector<std::pair<std::string, bool> > registerTypes
            {
                {
                    "RawMemRef",
                    OTIO_NS::TypeRegistry::instance().register_type<tl::timeline::RawMemRef>()
                },
                {
                    "SharedMemRef",
                    OTIO_NS::TypeRegistry::instance().register_type<tl::timeline::SharedMemRef>()
                },
                {
                    "SeqRawMemRef",
                    OTIO_NS::TypeRegistry::instance().register_type<tl::timeline::SeqRawMemRef>()
                },
                {
                    "SeqSharedMemRef",
                    OTIO_NS::TypeRegistry::instance().register_type<tl::timeline::SeqSharedMemRef>()
                },
                {
                    "ZipMemRef",
                    OTIO_NS::TypeRegistry::instance().register_type<tl::timeline::ZipMemRef>()
                },
                {
                    "SeqZipMemRef",
                    OTIO_NS::TypeRegistry::instance().register_type<tl::timeline::SeqZipMemRef>()
                }
            };
            for (const auto& t : registerTypes)
            {
                _log(ftk::Format("Register type {0}: {1}").
                    arg(t.first).
                    arg(t.second));
            }
        }

        System::~System()
        {}

        std::shared_ptr<System> System::create(const std::shared_ptr<ftk::Context>& context)
        {
            auto out = context->getSystem<System>();
            if (!out)
            {
                out = std::shared_ptr<System>(new System(context));
                context->addSystem(out);
            }
            return out;
        }
    }
}
