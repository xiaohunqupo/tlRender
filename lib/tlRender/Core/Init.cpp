// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/Core/Init.h>

#include <tlRender/Core/AudioSystem.h>

#include <ftk/Core/Context.h>
#include <ftk/Core/Format.h>
#include <ftk/Core/LogSystem.h>

namespace tl
{
    void init(const std::shared_ptr<ftk::Context>& context)
    {
        auto logSystem = context->getLogSystem();
        logSystem->print(
            "tl::init",
            ftk::Format("tlRender version: {0}").arg(TLRENDER_VERSION_FULL));

        AudioSystem::create(context);
    }
}
