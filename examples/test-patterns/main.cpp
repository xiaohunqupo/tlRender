// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include "App.h"

#include <tlRender/Timeline/Init.h>

#include <ftk/Core/Context.h>

#include <iostream>

FTK_MAIN()
{
    try
    {
        auto context = ftk::Context::create();
        tl::init(context);
        auto app = tl::examples::test_patterns::App::create(context, ftk::convert(argc, argv));
        if (app->hasCmdLineHelp())
            return 0;
        app->run();
    }
    catch(const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
