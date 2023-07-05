// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/CompareMenu.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
    {
        struct CompareMenu::Private
        {
            std::weak_ptr<App> app;

            std::map<std::string, std::shared_ptr<ui::MenuItem> > items;
            std::map<timeline::CompareMode, std::shared_ptr<ui::MenuItem> > compareItems;
            std::shared_ptr<Menu> currentMenu;
            std::vector<std::shared_ptr<ui::MenuItem> > currentItems;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
            std::shared_ptr<observer::ListObserver<int> > bIndexesObserver;
            std::shared_ptr<observer::ValueObserver<timeline::CompareOptions> > compareOptionsObserver;
        };

        void CompareMenu::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            Menu::_init(context);
            TLRENDER_P();

            p.app = app;

            p.currentMenu = addSubMenu("Current");

            p.items["Next"] = std::make_shared<ui::MenuItem>(
                "Next",
                "Next",
                ui::Key::PageDown,
                static_cast<int>(ui::KeyModifier::Shift),
                [this]
                {
                    close();
                if (auto app = _p->app.lock())
                {
                    app->getFilesModel()->nextB();
                }
                });
            addItem(p.items["Next"]);

            p.items["Prev"] = std::make_shared<ui::MenuItem>(
                "Previous",
                "Prev",
                ui::Key::PageUp,
                static_cast<int>(ui::KeyModifier::Shift),
                [this]
                {
                    close();
                if (auto app = _p->app.lock())
                {
                    app->getFilesModel()->prevB();
                }
                });
            addItem(p.items["Prev"]);

            addDivider();

            const std::array<std::string, static_cast<size_t>(timeline::CompareMode::Count)> icons =
            {
                "CompareA",
                "CompareB",
                "CompareWipe",
                "CompareOverlay",
                "CompareDifference",
                "CompareHorizontal",
                "CompareVertical",
                "CompareTile"
            };
            const std::array<ui::Key, static_cast<size_t>(timeline::CompareMode::Count)> shortcuts =
            {
                ui::Key::A,
                ui::Key::B,
                ui::Key::W,
                ui::Key::Unknown,
                ui::Key::Unknown,
                ui::Key::Unknown,
                ui::Key::Unknown,
                ui::Key::T
            };
            const auto enums = timeline::getCompareModeEnums();
            for (size_t i = 0; i < enums.size(); ++i)
            {
                const auto mode = enums[i];
                p.compareItems[mode] = std::make_shared<ui::MenuItem>(
                    timeline::getLabel(mode),
                    icons[i],
                    shortcuts[i],
                    static_cast<int>(ui::KeyModifier::Control),
                    [this, mode]
                    {
                        close();
                        if (auto app = _p->app.lock())
                        {
                            auto options = app->getFilesModel()->getCompareOptions();
                            options.mode = mode;
                            app->getFilesModel()->setCompareOptions(options);
                        }
                    });
                addItem(p.compareItems[mode]);
            }

            p.filesObserver = observer::ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                app->getFilesModel()->observeFiles(),
                [this](const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
                {
                    _filesUpdate(value);
                });

            p.bIndexesObserver = observer::ListObserver<int>::create(
                app->getFilesModel()->observeBIndexes(),
                [this](const std::vector<int>& value)
                {
                    _currentUpdate(value);
                });

            p.compareOptionsObserver = observer::ValueObserver<timeline::CompareOptions>::create(
                app->getFilesModel()->observeCompareOptions(),
                [this](const timeline::CompareOptions& value)
                {
                    _compareUpdate(value);
                });
        }

        CompareMenu::CompareMenu() :
            _p(new Private)
        {}

        CompareMenu::~CompareMenu()
        {}

        std::shared_ptr<CompareMenu> CompareMenu::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<CompareMenu>(new CompareMenu);
            out->_init(app, context);
            return out;
        }

        void CompareMenu::close()
        {
            Menu::close();
            TLRENDER_P();
            p.currentMenu->close();
        }

        void CompareMenu::_filesUpdate(
            const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
        {
            TLRENDER_P();

            setItemEnabled(p.items["Next"], value.size() > 1);
            setItemEnabled(p.items["Prev"], value.size() > 1);

            p.currentMenu->clear();
            p.currentItems.clear();
            if (auto app = p.app.lock())
            {
                const auto bIndexes = app->getFilesModel()->getBIndexes();
                for (size_t i = 0; i < value.size(); ++i)
                {
                    auto item = std::make_shared<ui::MenuItem>(
                        value[i]->path.get(-1, false),
                        [this, i]
                        {
                            close();
                        if (auto app = _p->app.lock())
                        {
                            app->getFilesModel()->toggleB(i);
                        }
                        });
                    const auto j = std::find(bIndexes.begin(), bIndexes.end(), i);
                    item->checked = j != bIndexes.end();
                    p.currentMenu->addItem(item);
                    p.currentItems.push_back(item);
                }
            }
        }

        void CompareMenu::_currentUpdate(const std::vector<int>& value)
        {
            TLRENDER_P();
            for (int i = 0; i < p.currentItems.size(); ++i)
            {
                const auto j = std::find(value.begin(), value.end(), i);
                p.currentMenu->setItemChecked(
                    p.currentItems[i],
                    j != value.end());
            }
        }

        void CompareMenu::_compareUpdate(const timeline::CompareOptions& value)
        {
            TLRENDER_P();
            for (const auto& item : p.compareItems)
            {
                setItemChecked(item.second, item.first == value.mode);
            }
        }
    }
}