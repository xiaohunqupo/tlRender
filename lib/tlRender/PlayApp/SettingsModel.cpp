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
            _fileBrowserSystem = context->getSystem<ftk::FileBrowserSystem>();
            _fileBrowserSystem->getModel()->setOptions(fileBrowserOptions);
            bool nativeFileDialog = false;
            get("/NativeFileDialog", nativeFileDialog);
            _fileBrowserSystem->setNativeFileDialog(nativeFileDialog);

            // Restore timeline player cache settings.
            timeline::PlayerCacheOptions cache;
            getT("/Cache", cache);
            _cache = ftk::Observable<timeline::PlayerCacheOptions>::create(cache);
        }
        
        SettingsModel::~SettingsModel()
        {
            // Save file browser settings.
            setT("/FileBrowser", _fileBrowserSystem->getModel()->getOptions());
            set("/NativeFileDialog", _fileBrowserSystem->isNativeFileDialog());

            // Save timeline player cache settings.
            setT("/Cache", _cache->get());
        }

        std::shared_ptr<SettingsModel> SettingsModel::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::filesystem::path& path)
        {
            return std::shared_ptr<SettingsModel>(new SettingsModel(context, path));
        }

        const timeline::PlayerCacheOptions& SettingsModel::getCache() const
        {
            return _cache->get();
        }

        std::shared_ptr<ftk::IObservable<timeline::PlayerCacheOptions> > SettingsModel::observeCache() const
        {
            return _cache;
        }

        void SettingsModel::setCache(const timeline::PlayerCacheOptions& value)
        {
            _cache->setIfChanged(value);
        }
    }
}
