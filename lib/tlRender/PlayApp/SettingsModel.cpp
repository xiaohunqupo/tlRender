// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include "SettingsModel.h"

namespace tl
{
    namespace play
    {
        SettingsModel::SettingsModel(
            const std::shared_ptr<ftk::Context>& context,
            const std::filesystem::path& path) :
            Settings(context, path, false)
        {
            // Restore file browser settings.
            ftk::FileBrowserOptions fileBrowserOptions;
            getT("/FileBrowser", fileBrowserOptions);
            auto fileBrowserSystem = context->getSystem<ftk::FileBrowserSystem>();
            fileBrowserSystem->getModel()->setOptions(fileBrowserOptions);
            bool nativeFileDialog = false;
            get("/NativeFileDialog", nativeFileDialog);
            fileBrowserSystem->setNativeFileDialog(nativeFileDialog);
            _fileBrowserSystem = fileBrowserSystem;

            // Restore timeline player cache settings.
            PlayerCacheOptions cache;
            getT("/Cache", cache);
            _cache = ftk::Observable<PlayerCacheOptions>::create(cache);
        }
        
        SettingsModel::~SettingsModel()
        {
            // Save file browser settings.
            if (auto fileBrowserSystem = _fileBrowserSystem.lock())
            {
                setT("/FileBrowser", fileBrowserSystem->getModel()->getOptions());
                set("/NativeFileDialog", fileBrowserSystem->isNativeFileDialog());
            }

            // Save timeline player cache settings.
            setT("/Cache", _cache->get());
        }

        std::shared_ptr<SettingsModel> SettingsModel::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::filesystem::path& path)
        {
            return std::shared_ptr<SettingsModel>(new SettingsModel(context, path));
        }

        const PlayerCacheOptions& SettingsModel::getCache() const
        {
            return _cache->get();
        }

        std::shared_ptr<ftk::IObservable<PlayerCacheOptions> > SettingsModel::observeCache() const
        {
            return _cache;
        }

        void SettingsModel::setCache(const PlayerCacheOptions& value)
        {
            _cache->setIfChanged(value);
        }
    }
}
