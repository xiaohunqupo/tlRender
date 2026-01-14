// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Core/Util.h>

#include <string>
#include <vector>

namespace tl
{
    //! Get the list of LUT formats supported by OCIO. The pairs consist of
    //! the format name and file extension.
    std::vector<std::pair<std::string, std::string> > getLUTFormats();
}
