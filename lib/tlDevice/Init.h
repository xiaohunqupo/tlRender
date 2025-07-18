// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <memory>

namespace feather_tk
{
    class Context;
}

namespace tl
{
    //! Hardware devices
    namespace device
    {
        //! Initialize the library.
        void init(const std::shared_ptr<feather_tk::Context>&);
    }
}
