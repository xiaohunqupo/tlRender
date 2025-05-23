// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQtWidget/Init.h>

#include <tlQt/Init.h>

#include <tlTimelineUI/Init.h>

#include <dtk/core/Context.h>
#include <dtk/core/String.h>
#include <dtk/core/Format.h>

#include <QDir>
#include <QFontDatabase>
#include <QMap>

#include <iostream>

namespace dtk_resource
{
    extern std::vector<uint8_t> NotoSansBold;
    extern std::vector<uint8_t> NotoSansMonoRegular;
    extern std::vector<uint8_t> NotoSansRegular;
}

namespace tl
{
    namespace qtwidget
    {
        void init(
            const std::shared_ptr<dtk::Context>& context,
            qt::DefaultSurfaceFormat defaultSurfaceFormat)
        {
            timelineui::init(context);
            qt::init(context, defaultSurfaceFormat);
        }

        void initFonts(const std::shared_ptr<dtk::Context>& context)
        {
            const std::vector<std::vector<uint8_t> > fonts =
            {
                dtk_resource::NotoSansMonoRegular,
                dtk_resource::NotoSansBold,
                dtk_resource::NotoSansRegular
            };
            std::vector<std::string> fontFamilyList;
            for (const auto& font : fonts)
            {
                const int id = QFontDatabase::addApplicationFontFromData(
                    QByteArray(reinterpret_cast<const char*>(font.data()), font.size()));
                for (const auto& j : QFontDatabase::applicationFontFamilies(id))
                {
                    fontFamilyList.push_back(j.toUtf8().data());
                }
            }
            context->log(
                "tl::qtwidget::initFonts",
                dtk::Format("Added Qt application fonts: {0}").arg(dtk::join(fontFamilyList, ", ")));
        }
    }
}
