// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayGLApp/IToolWidget.h>

namespace tl
{
    namespace play_gl
    {
        class App;

        //! View tool.
        class ViewTool : public IToolWidget
        {
            TLRENDER_NON_COPYABLE(ViewTool);

        protected:
            void _init(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            ViewTool();

        public:
            virtual ~ViewTool();

            static std::shared_ptr<ViewTool> create(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
            TLRENDER_PRIVATE();
        };
    }
}