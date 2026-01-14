// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/Core/Color.h>

#include <OpenColorIO/OpenColorTransforms.h>

namespace tl
{
    std::vector<std::pair<std::string, std::string> > getLUTFormats()
    {
        std::vector<std::pair<std::string, std::string> > out;
        for (int i = 0; i < OCIO_NAMESPACE::FileTransform::GetNumFormats(); ++i)
        {
            out.push_back(std::make_pair(
                OCIO_NAMESPACE::FileTransform::GetFormatNameByIndex(i),
                OCIO_NAMESPACE::FileTransform::GetFormatExtensionByIndex(i)));
        }
        return out;
    }
}
