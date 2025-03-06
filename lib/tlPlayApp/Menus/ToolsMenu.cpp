// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Menus/ToolsMenu.h>

#include <tlPlayApp/Actions/ToolsActions.h>
#include <tlPlayApp/Models/ToolsModel.h>
#include <tlPlayApp/App.h>

namespace tl
{
    namespace play
    {
        struct ToolsMenu::Private
        {
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;

            std::shared_ptr<dtk::ValueObserver<Tool> > activeObserver;
        };

        void ToolsMenu::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<ToolsActions>& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            DTK_P();

            p.actions = actions->getActions();

            for (const auto tool : getToolLabels())
            {
                addItem(p.actions[tool]);
            }

            p.activeObserver = dtk::ValueObserver<Tool>::create(
                app->getToolsModel()->observeActiveTool(),
                [this](Tool value)
                {
                    const auto enums = getToolEnums();
                    const auto labels = getToolLabels();
                    for (size_t i = 0; i < enums.size(); ++i)
                    {
                        setItemChecked(_p->actions[labels[i]], enums[i] == value);
                    }
                });
        }

        ToolsMenu::ToolsMenu() :
            _p(new Private)
        {}

        ToolsMenu::~ToolsMenu()
        {}

        std::shared_ptr<ToolsMenu> ToolsMenu::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<ToolsActions>& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ToolsMenu>(new ToolsMenu);
            out->_init(context, app, actions, parent);
            return out;
        }
    }
}
