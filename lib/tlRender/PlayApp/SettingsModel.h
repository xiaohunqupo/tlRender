// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Timeline/Player.h>

#include <ftk/UI/FileBrowser.h>
#include <ftk/UI/Settings.h>

namespace tl
{
    namespace play
    {
        //! This model provides settings that are saved and restored.
        class SettingsModel : public ftk::Settings
        {
            FTK_NON_COPYABLE(SettingsModel);

        protected:
            SettingsModel(
                const std::shared_ptr<ftk::Context>&,
                const std::filesystem::path&);

        public:
            ~SettingsModel();

            //! Create a new model.
            static std::shared_ptr<SettingsModel> create(
                const std::shared_ptr<ftk::Context>&,
                const std::filesystem::path&);

            //! Get the cache settings.
            const timeline::PlayerCacheOptions& getCache() const;

            //! Observe the cache settings.
            std::shared_ptr<ftk::IObservable<timeline::PlayerCacheOptions> > observeCache() const;

            //! Set the cache settings.
            void setCache(const timeline::PlayerCacheOptions&);

        private:
            std::shared_ptr<ftk::FileBrowserSystem> _fileBrowserSystem;
            std::shared_ptr<ftk::Observable<timeline::PlayerCacheOptions> > _cache;
        };
    }
}
