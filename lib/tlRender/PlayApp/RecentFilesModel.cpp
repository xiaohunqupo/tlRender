// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include "RecentFilesModel.h"

namespace tl
{
    namespace play
    {
        void RecentFilesModel::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<ftk::Settings>& settings)
        {
            ftk::RecentFilesModel::_init(context);

            _settings = settings;

            nlohmann::json json;
            std::vector<std::string> recent;
            _settings->get("/Files/Recent", recent);
            std::vector<std::filesystem::path> recentPaths;
            for (const auto& i : recent)
            {
                recentPaths.push_back(std::filesystem::u8path(i));
            }
            setRecent(recentPaths);
            size_t max = 10;
            _settings->get("/Files/RecentMax", max);
            setRecentMax(max);
        }

        RecentFilesModel::~RecentFilesModel()
        {
            std::vector<std::string> recent;
            for (const auto& path : getRecent())
            {
                recent.push_back(path.u8string());
            }
            _settings->set("/Files/Recent", recent);
            _settings->set("/Files/RecentMax", getRecentMax());
        }

        std::shared_ptr<RecentFilesModel> RecentFilesModel::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<ftk::Settings>& settings)
        {
            auto out = std::shared_ptr<RecentFilesModel>(new RecentFilesModel);
            out->_init(context, settings);
            return out;
        }
    }
}
