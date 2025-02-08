// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <dtk/core/IApp.h>

namespace tl
{
    //! tlresource application
    namespace resource
    {
        //! Application options.
        struct Options
        {
        };

        //! Application.
        class App : public dtk::IApp
        {
            DTK_NON_COPYABLE(App);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                std::vector<std::string>&);

            App();

        public:
            ~App();

            //! Create a new application.
            static std::shared_ptr<App> create(
                const std::shared_ptr<dtk::Context>&,
                std::vector<std::string>&);

            //! Run the application.
            void run() override;

        private:
            std::string _input;
            std::string _output;
            std::string _varName;
            Options _options;

            std::chrono::steady_clock::time_point _startTime;
        };
    }
}
