// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include "App.h"

#include <tlRender/QtQuick/Init.h>

#include <ftk/Core/Context.h>

#include <iostream>

int main(int argc, char* argv[])
{
    try
    {
        auto context = ftk::Context::create();
        tl::qtquick::init(
            context,
            tl::qt::DefaultSurfaceFormat::OpenGL_4_1_CoreProfile);
#if (QT_VERSION < QT_VERSION_CHECK(6, 5, 0))
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
        tl::examples::player_qtquick::App app(context, argc, argv);
        return app.exec();
    }
    catch(const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
    return 1;
}
