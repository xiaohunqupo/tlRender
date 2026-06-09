// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/PlayApp/App.h>

#include <tlRender/UI/Init.h>

#include <tlRender/Device/Init.h>

int main(int argc, char* argv[])
{
    try
    {
        auto context = ftk::Context::create();
        tl::ui::init(context);
        tl::device::init(context);
        auto app = tl::play::App::create(context, ftk::convert(argc, argv));
        if (app->hasCmdLineHelp())
            return 0;
        app->run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
