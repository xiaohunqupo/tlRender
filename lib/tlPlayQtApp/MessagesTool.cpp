// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/MessagesTool.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/DockTitleBar.h>

#include <QAction>
#include <QBoxLayout>
#include <QClipboard>
#include <QListWidget>
#include <QToolButton>

namespace tl
{
    namespace play_qt
    {
        namespace
        {
            const int messagesMax = 20;
        }

        struct MessagesTool::Private
        {
            QListWidget* listWidget = nullptr;
            QToolButton* copyButton = nullptr;
            QToolButton* clearButton = nullptr;
            std::shared_ptr<dtk::ListObserver<dtk::LogItem> > logObserver;
        };

        MessagesTool::MessagesTool(App* app, QWidget* parent) :
            IToolWidget(app, parent),
            _p(new Private)
        {
            DTK_P();

            p.listWidget = new QListWidget;
            p.listWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
            p.listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

            p.copyButton = new QToolButton;
            p.copyButton->setText("Copy");
            p.copyButton->setAutoRaise(true);
            p.copyButton->setToolTip(tr("Copy the contents to the clipboard"));

            p.clearButton = new QToolButton;
            p.clearButton->setText("Clear");
            p.clearButton->setAutoRaise(true);
            p.clearButton->setToolTip(tr("Clear the contents"));

            auto layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(p.listWidget);
            auto hLayout = new QHBoxLayout;
            hLayout->setContentsMargins(5, 5, 5, 5);
            hLayout->setSpacing(5);
            hLayout->addWidget(p.copyButton);
            hLayout->addWidget(p.clearButton);
            hLayout->addStretch();
            layout->addLayout(hLayout);
            auto widget = new QWidget;
            widget->setLayout(layout);
            addWidget(widget);

            p.logObserver = dtk::ListObserver<dtk::LogItem>::create(
                app->getContext()->getLogSystem()->observeLogItems(),
                [this](const std::vector<dtk::LogItem>& value)
                {
                    for (const auto& i : value)
                    {
                        switch (i.type)
                        {
                        case dtk::LogType::Warning:
                        case dtk::LogType::Error:
                            _p->listWidget->addItem(QString::fromUtf8(dtk::toString(i).c_str()));
                            break;
                        default: break;
                        }
                        while (_p->listWidget->count() > messagesMax)
                        {
                            delete _p->listWidget->takeItem(0);
                        }
                    }
                });

            connect(
                p.copyButton,
                &QToolButton::clicked,
                [this]
                {
                    auto clipboard = QGuiApplication::clipboard();
                    QStringList text;
                    for (int i = 0; i < _p->listWidget->count(); ++i)
                    {
                        text.append(_p->listWidget->item(i)->text());
                    }
                    clipboard->setText(text.join('\n'));
                });

            connect(
                p.clearButton,
                &QToolButton::clicked,
                p.listWidget,
                &QListWidget::clear);
        }

        MessagesTool::~MessagesTool()
        {}

        MessagesDockWidget::MessagesDockWidget(
            MessagesTool * messagesTool,
            QWidget * parent)
        {
            setObjectName("MessagesTool");
            setWindowTitle(tr("Messages"));
            setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

            auto dockTitleBar = new DockTitleBar;
            dockTitleBar->setText(tr("Messages"));
            dockTitleBar->setIcon(QIcon(":/Icons/Messages.svg"));
            auto dockWidget = new QDockWidget;
            setTitleBarWidget(dockTitleBar);

            setWidget(messagesTool);

            toggleViewAction()->setIcon(QIcon(":/Icons/Messages.svg"));
            toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F8));
            toggleViewAction()->setToolTip(tr("Show messages"));
        }
    }
}
