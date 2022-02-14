// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlay/FilesTool.h>

#include <tlPlay/App.h>
#include <tlPlay/FilesModel.h>
#include <tlPlay/FilesView.h>

#include <tlQt/Util.h>

#include <QBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QSignalBlocker>
#include <QSettings>
#include <QToolBar>
#include <QTreeView>

namespace tl
{
    namespace play
    {
        struct FilesTool::Private
        {
            App* app = nullptr;
            FilesAModel* filesAModel = nullptr;
            QTreeView* treeView = nullptr;
        };

        FilesTool::FilesTool(
            const QMap<QString, QAction*>& actions,
            App* app,
            QWidget* parent) :
            ToolWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;
            p.filesAModel = new FilesAModel(app->filesModel(), app->getContext(), this);

            p.treeView = new QTreeView;
            p.treeView->setAllColumnsShowFocus(true);
            p.treeView->setAlternatingRowColors(true);
            p.treeView->setSelectionMode(QAbstractItemView::NoSelection);
            p.treeView->setItemDelegateForColumn(1, new FilesLayersItemDelegate);
            p.treeView->setEditTriggers(QAbstractItemView::CurrentChanged);
            p.treeView->setIndentation(0);
            //! \bug Setting the model causes this output to be printed on exit:
            //! QBasicTimer::start: QBasicTimer can only be used with threads started with QThread
            p.treeView->setModel(p.filesAModel);

            auto toolBar = new QToolBar;
            toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
            toolBar->setIconSize(QSize(20, 20));
            toolBar->addAction(actions["File/Open"]);
            toolBar->addAction(actions["File/OpenWithAudio"]);
            toolBar->addAction(actions["File/Close"]);
            toolBar->addAction(actions["File/CloseAll"]);
            toolBar->addAction(actions["File/Prev"]);
            toolBar->addAction(actions["File/Next"]);
            toolBar->addAction(actions["File/PrevLayer"]);
            toolBar->addAction(actions["File/NextLayer"]);

            auto vLayout = new QVBoxLayout;
            vLayout->setContentsMargins(0, 0, 0, 0);
            vLayout->setSpacing(0);
            vLayout->addWidget(p.treeView);
            vLayout->addWidget(toolBar);
            auto widget = new QWidget;
            widget->setLayout(vLayout);
            addWidget(widget, 1);

            QSettings settings;
            auto ba = settings.value(qt::versionedSettingsKey("FilesTool/Header")).toByteArray();
            if (!ba.isEmpty())
            {
                p.treeView->header()->restoreState(ba);
            }

            connect(
                p.treeView,
                SIGNAL(activated(const QModelIndex&)),
                SLOT(_activatedCallback(const QModelIndex&)));
        }

        FilesTool::~FilesTool()
        {
            TLRENDER_P();
            QSettings settings;
            settings.setValue(qt::versionedSettingsKey("FilesTool/Header"), p.treeView->header()->saveState());
        }

        void FilesTool::_activatedCallback(const QModelIndex& index)
        {
            TLRENDER_P();
            p.app->filesModel()->setA(index.row());
        }
    }
}