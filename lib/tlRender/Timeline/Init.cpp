// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/Timeline/Init.h>

#include <tlRender/Timeline/MemRef.h>
#include <tlRender/Timeline/Player.h>
#include <tlRender/Timeline/System.h>

#include <tlRender/IO/System.h>

#include <tlRender/Core/AudioSystem.h>

#include <ftk/GL/Init.h>
#include <ftk/Core/Context.h>
#include <ftk/Core/DiagSystem.h>
#include <ftk/Core/Format.h>

#include <opentimelineio/typeRegistry.h>

namespace tl
{
    void init(const std::shared_ptr<ftk::Context>& context)
    {
        auto logSystem = context->getLogSystem();
        logSystem->print(
            "tl::init",
            ftk::Format("tlRender version: {0}").arg(TLRENDER_VERSION_FULL));

        auto diagSystem = context->getSystem<ftk::DiagSystem>();
        diagSystem->addSampler(
            "tlRender Memory/Audio: {0}MB",
            [] { return tl::Audio::getTotalByteCount() / ftk::megabyte; });
        diagSystem->addSampler(
            "tlRender Objects/Audio: {0}",
            [] { return tl::Audio::getObjectCount(); });
        diagSystem->addSampler(
            "tlRender Objects/I/O: {0}",
            [] { return tl::IIO::getObjectCount(); });
        diagSystem->addSampler(
            "tlRender Objects/Players: {0}",
            [] { return tl::Player::getObjectCount(); });
        diagSystem->addSampler(
            "tlRender Objects/Timelines: {0}",
            [] { return tl::Timeline::getObjectCount(); });

        ftk::gl::init(context);

        AudioSystem::create(context);
        ReadSystem::create(context);
        WriteSystem::create(context);

        const std::vector<std::pair<std::string, bool> > registerTypes
        {
            {
                "RawMemRef",
                OTIO_NS::TypeRegistry::instance().register_type<tl::RawMemRef>()
            },
            {
                "SharedMemRef",
                OTIO_NS::TypeRegistry::instance().register_type<tl::SharedMemRef>()
            },
            {
                "SeqRawMemRef",
                OTIO_NS::TypeRegistry::instance().register_type<tl::SeqRawMemRef>()
            },
            {
                "SeqSharedMemRef",
                OTIO_NS::TypeRegistry::instance().register_type<tl::SeqSharedMemRef>()
            },
            {
                "ZipMemRef",
                OTIO_NS::TypeRegistry::instance().register_type<tl::ZipMemRef>()
            },
            {
                "SeqZipMemRef",
                OTIO_NS::TypeRegistry::instance().register_type<tl::SeqZipMemRef>()
            }
        };
        for (const auto& t : registerTypes)
        {
            logSystem->print(
                "tl::init",
                ftk::Format("Register type {0}: {1}").
                arg(t.first).
                arg(t.second));
        }

        System::create(context);
    }
}
