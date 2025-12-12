// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Core/Export.h>

#include <ftk/Core/Util.h>

#include <string>

namespace tl
{
    //! URLs
    namespace url
    {
        //! Get the URL scheme.
        TL_API std::string scheme(const std::string&);

        //! Encode a URL.
        TL_API std::string encode(const std::string&);

        //! Decode a URL.
        TL_API std::string decode(const std::string&);
    }
}
