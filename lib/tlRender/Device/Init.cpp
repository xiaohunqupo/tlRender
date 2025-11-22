// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/Device/Init.h>

#if defined(TLRENDER_BMD)
#include <tlRender/Device/BMDSystem.h>
#endif // TLRENDER_BMD

#include <tlRender/Timeline/Init.h>

#include <ftk/Core/Context.h>

namespace tl
{
    namespace device
    {
        void init(const std::shared_ptr<ftk::Context>& context)
        {
            timeline::init(context);
#if defined(TLRENDER_BMD)
            bmd::System::create(context);
#endif // TLRENDER_BMD
        }
    }
}
