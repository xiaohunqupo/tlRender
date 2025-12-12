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

        //! Timeline system.
        class TL_API_TYPE System : public system::ISystem
        {
            FTK_NON_COPYABLE(System);

        protected:
            System(const std::shared_ptr<ftk::Context>&);

        public:
            TL_API virtual ~System();

            //! Create a new system.
            TL_API static std::shared_ptr<System> create(const std::shared_ptr<ftk::Context>&);
        };
    }
}
