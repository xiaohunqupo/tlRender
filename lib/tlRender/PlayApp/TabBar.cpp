// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include "TabBar.h"

#include "App.h"
#include "FilesModel.h"

namespace tl
{
    namespace play
    {
        void TabBar::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "TabBar", parent);

            _tabBar = ftk::TabBar::create(context, shared_from_this());
            _tabBar->setClosable(true);

            std::weak_ptr<App> appWeak(app);
            _tabBar->setCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->setCurrent(value);
                    }
                });
            _tabBar->setCloseCallback(
                [appWeak](int index)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->close(index);
                    }
                });

            _playersObserver = ftk::ListObserver<std::shared_ptr<Player> >::create(
                app->getFilesModel()->observePlayers(),
                [this](const std::vector<std::shared_ptr<Player> >& value)
                {
                    const int index = _tabBar->getCurrent();
                    _tabBar->clear();
                    for (const auto& player : value)
                    {
                        _tabBar->addTab(
                            player->getPath().getFileName(),
                            player->getPath().get());
                    }
                    _tabBar->setCurrent(index);
                });

            _playerIndexObserver = ftk::Observer<int>::create(
                app->getFilesModel()->observePlayerIndex(),
                [this](int value)
                {
                    _tabBar->setCurrent(value);
                });
        }

        TabBar::~TabBar()
        {}

        std::shared_ptr<TabBar> TabBar::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TabBar>(new TabBar);
            out->_init(context, app, parent);
            return out;
        }
        
        ftk::Size2I TabBar::getSizeHint() const
        {
            return _tabBar->getSizeHint();
        }

        void TabBar::setGeometry(const ftk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _tabBar->setGeometry(value);
        }
    }
}