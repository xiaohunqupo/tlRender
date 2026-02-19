// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include "ViewActions.h"

#include "App.h"
#include "FilesModel.h"

#include <tlRender/UI/Viewport.h>

namespace tl
{
    namespace play
    {
        void ViewActions::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<ui::Viewport>& viewport)
        {
            auto viewportWeak = std::weak_ptr<ui::Viewport>(viewport);
            _actions["Frame"] = ftk::Action::create(
                "Frame",
                "ViewFrame",
                [viewportWeak](bool value)
                {
                    if (auto viewport = viewportWeak.lock())
                    {
                        viewport->setFrameView(value);
                    }
                });
            _actions["Frame"]->setTooltip("Toggle whether to automatically frame the view.");

            _actions["ZoomReset"] = ftk::Action::create(
                "Zoom Reset",
                "ViewZoomReset",
                [viewportWeak]
                {
                    if (auto viewport = viewportWeak.lock())
                    {
                        viewport->viewZoomReset();
                    }
                });
            _actions["ZoomReset"]->setTooltip("Reset the view zoom.");

            _actions["ZoomIn"] = ftk::Action::create(
                "Zoom In",
                "ViewZoomIn",
                [viewportWeak]
                {
                    if (auto viewport = viewportWeak.lock())
                    {
                        viewport->viewZoomIn();
                    }
                });
            _actions["ZoomIn"]->setTooltip("Zoom the view in.");

            _actions["ZoomOut"] = ftk::Action::create(
                "Zoom Out",
                "ViewZoomOut",
                [viewportWeak]
                {
                    if (auto viewport = viewportWeak.lock())
                    {
                        viewport->viewZoomOut();
                    }
                });
            _actions["ZoomOut"]->setTooltip("Zoom the view out.");

            _frameObserver = ftk::Observer<bool>::create(
                viewport->observeFrameView(),
                [this](bool value)
                {
                    _actions["Frame"]->setChecked(value);
                });

            _playerObserver = ftk::Observer<std::shared_ptr<Player> >::create(
                app->getFilesModel()->observePlayer(),
                [this](const std::shared_ptr<Player>& value)
                {
                    _actions["Frame"]->setEnabled(value.get());
                    _actions["ZoomReset"]->setEnabled(value.get());
                    _actions["ZoomIn"]->setEnabled(value.get());
                    _actions["ZoomOut"]->setEnabled(value.get());
                });
        }

        ViewActions::~ViewActions()
        {}

        std::shared_ptr<ViewActions> ViewActions::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<ui::Viewport>& viewport)
        {
            auto out = std::shared_ptr<ViewActions>(new ViewActions);
            out->_init(context, app, viewport);
            return out;
        }

        const std::map<std::string, std::shared_ptr<ftk::Action> >& ViewActions::getActions() const
        {
            return _actions;
        }
    }
}
