// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/ISystem.h>

#include <tlCore/Path.h>

class QWidget;

namespace tl
{
    namespace qtwidget
    {
        //! File browser system.
        class FileBrowserSystem : public system::ISystem
        {
            DTK_NON_COPYABLE(FileBrowserSystem);

        protected:
            FileBrowserSystem(const std::shared_ptr<dtk::Context>&);

        public:
            virtual ~FileBrowserSystem();

            //! Create a new system.
            static std::shared_ptr<FileBrowserSystem> create(
                const std::shared_ptr<dtk::Context>&);

            //! Open the file browser.
            void open(
                QWidget*,
                const std::function<void(const file::Path&)>&);

            //! Get the path.
            const std::string& getPath() const;

            //! Set the path.
            void setPath(const std::string&);

        private:
            DTK_PRIVATE();
        };
    }
}
