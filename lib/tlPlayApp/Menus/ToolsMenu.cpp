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
        void ToolsMenu::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<ToolsActions>& toolsActions,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);

            const auto labels = getToolLabels();
            auto actions = toolsActions->getActions();
            for (size_t i = 1; i < labels.size(); ++i)
            {
                addItem(actions[labels[i]]);
            }
        }

        ToolsMenu::~ToolsMenu()
        {}

        std::shared_ptr<ToolsMenu> ToolsMenu::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<ToolsActions>& toolsActions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ToolsMenu>(new ToolsMenu);
            out->_init(context, toolsActions, parent);
            return out;
        }
    }
}
