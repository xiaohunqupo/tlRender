// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlBaseApp/BaseApp.h>

#include <tlUI/Event.h>

#include <tlCore/Size.h>

namespace tl
{
    namespace ui
    {
        class Style;
    }

    namespace app
    {
        class Window;

        //! User interface options.
        struct UIOptions
        {
            math::Size2i windowSize = math::Size2i(1920, 1080);
            bool fullscreen = false;
        };

        //! Base class for user interface applications.
        class IApp : public app::BaseApp
        {
            TLRENDER_NON_COPYABLE(IApp);

        protected:
            void _init(
                const std::vector<std::string>&,
                const std::shared_ptr<system::Context>&,
                const std::string& cmdLineName,
                const std::string& cmdLineSummary,
                const std::vector<std::shared_ptr<app::ICmdLineArg> >& = {},
                const std::vector<std::shared_ptr<app::ICmdLineOption> >& = {});

            IApp();

        public:
            ~IApp();

            //! Run the application.
            virtual int run();

            //! Exit the application.
            void exit(int = 0);

            //! Get the style.
            const std::shared_ptr<ui::Style> getStyle() const;

            //! Get the number of screens.
            int getScreenCount() const;

            //! Add a window.
            void addWindow(const std::shared_ptr<Window>&);

            //! Remove a window.
            void removeWindow(const std::shared_ptr<Window>&);

        protected:
            virtual void _tick();

            void _tickRecursive(
                const std::shared_ptr<ui::IWidget>&,
                bool visible,
                bool enabled,
                const ui::TickEvent&);

            UIOptions _uiOptions;

        private:
            void _removeWindow(const std::shared_ptr<Window>&);

            TLRENDER_PRIVATE();
        };
    }
}