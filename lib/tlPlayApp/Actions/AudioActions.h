// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/Models/SettingsModel.h>

#include <dtk/ui/Action.h>

namespace tl
{
    namespace play
    {
        class App;

        //! Audio actions.
        class AudioActions : public std::enable_shared_from_this<AudioActions>
        {
            DTK_NON_COPYABLE(AudioActions);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&);

            AudioActions();

        public:
            ~AudioActions();

            static std::shared_ptr<AudioActions> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&);

            const std::map<std::string, std::shared_ptr<dtk::Action> >& getActions() const;

        private:
            void _keyShortcutsUpdate(const KeyShortcutsSettings&);

            DTK_PRIVATE();
        };
    }
}
