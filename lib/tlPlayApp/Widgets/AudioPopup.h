// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/ui/IWidgetPopup.h>

namespace tl
{
    namespace play
    {
        class App;

        //! Audio popup.
        class AudioPopup : public dtk::IWidgetPopup
        {
            DTK_NON_COPYABLE(AudioPopup);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            AudioPopup();

        public:
            virtual ~AudioPopup();

            static std::shared_ptr<AudioPopup> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
            DTK_PRIVATE();
        };
    }
}
