// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/Timeline/Init.h>

#include <tlRender/Timeline/MemRef.h>
#include <tlRender/Timeline/System.h>

#include <tlRender/IO/Init.h>

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
            auto logSystem = context->getLogSystem();
            for (const auto& t : registerTypes)
            {
                logSystem->print(
                    "tl::timeline::init",
                    ftk::Format("Register type {0}: {1}").
                    arg(t.first).
                    arg(t.second));
            }

            System::create(context);
        }
    }
}
