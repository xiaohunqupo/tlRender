// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Core/ISystem.h>

namespace tl
{
    //! Timelines
    namespace timeline
    {
        //! Initialize the library.
        TL_API void init(const std::shared_ptr<ftk::Context>&);
    }
}
