// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Core/Export.h>

#include <ftk/Core/ISystem.h>

#include <filesystem>

namespace tl
{
    namespace file
    {
        //! File logging system.
        class TL_API_TYPE FileLogSystem : public ftk::ISystem
        {
            FTK_NON_COPYABLE(FileLogSystem);

        protected:
            FileLogSystem(
                const std::shared_ptr<ftk::Context>&,
                const std::filesystem::path&);

        public:
            TL_API virtual ~FileLogSystem();

            //! Create a new system.
            TL_API static std::shared_ptr<FileLogSystem> create(
                const std::shared_ptr<ftk::Context>&,
                const std::filesystem::path&);

        private:
            FTK_PRIVATE();
        };
    }
}
