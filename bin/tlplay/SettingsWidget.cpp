// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "SettingsWidget.h"

#include "App.h"
#include "SettingsModel.h"

#include <feather-tk/ui/GroupBox.h>

namespace tl
{
    namespace play
    {
        void CacheSettingsWidget::_init(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "CacheSettingsWidget", parent);

            _videoEdit = feather_tk::DoubleEdit::create(context);
            _videoEdit->setRange(0.0, 128.0);
            _videoEdit->setStep(1.0);
            _videoEdit->setLargeStep(10.0);

            _audioEdit = feather_tk::DoubleEdit::create(context);
            _audioEdit->setRange(0.0, 128.0);
            _audioEdit->setStep(1.0);
            _audioEdit->setLargeStep(10.0);

            _readBehindEdit = feather_tk::DoubleEdit::create(context);
            _readBehindEdit->setRange(0.0, 2.0);

            _layout = feather_tk::FormLayout::create(context, shared_from_this());
            _layout->setSpacingRole(feather_tk::SizeRole::SpacingSmall);
            _layout->addRow("Video cache (GB):", _videoEdit);
            _layout->addRow("Audio cache (GB):", _audioEdit);
            _layout->addRow("Read behind (seconds):", _readBehindEdit);

            std::weak_ptr<App> appWeak(app);
            _videoEdit->setCallback(
                [appWeak](double value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto cache = app->getSettingsModel()->getCache();
                        cache.videoGB = value;
                        app->getSettingsModel()->setCache(cache);
                    }
                });

            _audioEdit->setCallback(
                [appWeak](double value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto cache = app->getSettingsModel()->getCache();
                        cache.audioGB = value;
                        app->getSettingsModel()->setCache(cache);
                    }
                });

            _readBehindEdit->setCallback(
                [appWeak](double value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto cache = app->getSettingsModel()->getCache();
                        cache.readBehind = value;
                        app->getSettingsModel()->setCache(cache);
                    }
                });

            _cacheObserver = feather_tk::ValueObserver<timeline::PlayerCacheOptions>::create(
                app->getSettingsModel()->observeCache(),
                [this](const timeline::PlayerCacheOptions& value)
                {
                    _videoEdit->setValue(value.videoGB);
                    _audioEdit->setValue(value.audioGB);
                    _readBehindEdit->setValue(value.readBehind);
                });
        }

        CacheSettingsWidget::~CacheSettingsWidget()
        {
        }

        std::shared_ptr<CacheSettingsWidget> CacheSettingsWidget::create(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<CacheSettingsWidget>(new CacheSettingsWidget);
            out->_init(context, app, parent);
            return out;
        }

        void CacheSettingsWidget::setGeometry(const feather_tk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _layout->setGeometry(value);
        }

        void CacheSettingsWidget::sizeHintEvent(const feather_tk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_layout->getSizeHint());
        }

        void SettingsWidget::_init(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "SettingsWidget", parent);
            _layout = feather_tk::VerticalLayout::create(context, shared_from_this());
            _layout->setMarginRole(feather_tk::SizeRole::MarginSmall);
            _layout->setSpacingRole(feather_tk::SizeRole::SpacingSmall);
            auto groupBox = feather_tk::GroupBox::create(context, "Cache", _layout);
            CacheSettingsWidget::create(context, app, groupBox);
        }

        SettingsWidget::~SettingsWidget()
        {
        }

        std::shared_ptr<SettingsWidget> SettingsWidget::create(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<SettingsWidget>(new SettingsWidget);
            out->_init(context, app, parent);
            return out;
        }

        void SettingsWidget::setGeometry(const feather_tk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _layout->setGeometry(value);
        }

        void SettingsWidget::sizeHintEvent(const feather_tk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_layout->getSizeHint());
        }
    }
}