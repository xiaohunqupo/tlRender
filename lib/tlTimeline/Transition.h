// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/core/Util.h>

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace tl
{
    namespace timeline
    {
        //! Transitions.
        enum class Transition
        {
            None,
            Dissolve,

            Count,
            First = None
        };
        DTK_ENUM(Transition);

        //! Convert to a transition.
        Transition toTransition(const std::string&);
    }
}
