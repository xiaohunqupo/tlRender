// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        template<typename T>
        inline const T* getParent(const OTIO_NS::Item* value)
        {
            const T* out = nullptr;
            while (value)
            {
                if (auto t = dynamic_cast<const T*>(value))
                {
                    out = t;
                    break;
                }
                value = value->parent();
            }
            return out;
        }
    }
}
