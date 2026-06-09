// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#if defined(TLRENDER_QT6)
#include <tlRender/QtTest/TimeObjectTest.h>
#include <tlRender/Qt/Init.h>
#endif // TLRENDER_QT6

#include <tlRender/UITest/ThumbnailSystemTest.h>

#include <tlRender/TimelineTest/ColorOptionsTest.h>
#include <tlRender/TimelineTest/CompareOptionsTest.h>
#include <tlRender/TimelineTest/DisplayOptionsTest.h>
#include <tlRender/TimelineTest/PlayerOptionsTest.h>
#include <tlRender/TimelineTest/PlayerTest.h>
#include <tlRender/TimelineTest/TimelineTest.h>
#include <tlRender/TimelineTest/UtilTest.h>

#include <tlRender/IOTest/IOTest.h>
#if defined(TLRENDER_FFMPEG_PLUGIN)
#include <tlRender/IOTest/FFmpegTest.h>
#endif // TLRENDER_FFMPEG_PLUGIN
#if defined(TLRENDER_EXR)
#include <tlRender/IOTest/EXRTest.h>
#endif // TLRENDER_EXR
#if defined(TLRENDER_OIIO)
#include <tlRender/IOTest/OIIOTest.h>
#endif // TLRENDER_OIIO

#include <tlRender/CoreTest/AudioTest.h>
#include <tlRender/CoreTest/HDRTest.h>
#include <tlRender/CoreTest/TimeTest.h>
#include <tlRender/CoreTest/URLTest.h>

#include <tlRender/UI/Init.h>

#include <ftk/Core/Context.h>

#include <iostream>
#include <vector>

using namespace tl;

void coreTests(
    std::vector<std::shared_ptr<ftk::test::ITest> >& tests,
    const std::shared_ptr<ftk::Context>& context)
{
    tests.push_back(core_tests::AudioTest::create(context));
    tests.push_back(core_tests::HDRTest::create(context));
    tests.push_back(core_tests::TimeTest::create(context));
    tests.push_back(core_tests::URLTest::create(context));
}

void ioTests(
    std::vector<std::shared_ptr<ftk::test::ITest> >& tests,
    const std::shared_ptr<ftk::Context>& context)
{
    tests.push_back(io_tests::IOTest::create(context));
#if defined(TLRENDER_FFMPEG_PLUGIN)
    tests.push_back(io_tests::FFmpegTest::create(context));
#endif // TLRENDER_FFMPEG_PLUGIN
#if defined(TLRENDER_OIIO)
    tests.push_back(io_tests::OIIOTest::create(context));
#endif // TLRENDER_OIIO
#if defined(TLRENDER_EXR)
    tests.push_back(io_tests::EXRTest::create(context));
#endif // TLRENDER_EXR
}

void timelineTests(
    std::vector<std::shared_ptr<ftk::test::ITest> >& tests,
    const std::shared_ptr<ftk::Context>& context)
{
    tests.push_back(timeline_tests::ColorOptionsTest::create(context));
    tests.push_back(timeline_tests::CompareOptionsTest::create(context));
    tests.push_back(timeline_tests::DisplayOptionsTest::create(context));
    tests.push_back(timeline_tests::PlayerOptionsTest::create(context));
    tests.push_back(timeline_tests::PlayerTest::create(context));
    tests.push_back(timeline_tests::TimelineTest::create(context));
    tests.push_back(timeline_tests::UtilTest::create(context));
}

void uiTests(
    std::vector<std::shared_ptr<ftk::test::ITest> >& tests,
    const std::shared_ptr<ftk::Context>& context)
{
    tests.push_back(ui_tests::ThumbnailSystemTest::create(context));
}

void qtTests(
    std::vector<std::shared_ptr<ftk::test::ITest> >& tests,
    const std::shared_ptr<ftk::Context>& context)
{
#if defined(TLRENDER_QT6)
    tests.push_back(qt_tests::TimeObjectTest::create(context));
#endif // TLRENDER_QT6
}

int main(int argc, char* argv[])
{
    try
    {
        auto context = ftk::Context::create();
#if defined(TLRENDER_QT6)
        qt::init(
            context,
            qt::DefaultSurfaceFormat::OpenGL_4_1_CoreProfile);
#else // TLRENDER_QT6
        ui::init(context);
#endif // TLRENDER_QT6

        auto logObserver = ftk::ListObserver<ftk::LogItem>::create(
            context->getSystem<ftk::LogSystem>()->observeLogItems(),
            [](const std::vector<ftk::LogItem>& value)
            {
                for (const auto& i : value)
                {
                    std::cout << "[LOG] " << ftk::getLabel(i) << std::endl;
                }
            },
            ftk::ObserverAction::Suppress);

        context->tick();

        std::vector<std::shared_ptr<ftk::test::ITest> > tests;
        coreTests(tests, context);
        ioTests(tests, context);
        timelineTests(tests, context);
        uiTests(tests, context);
        qtTests(tests, context);

        for (const auto& test : tests)
        {
            std::cout << "Running test: " << test->getName() << std::endl;
            test->run();
            context->tick();
        }

        std::cout << "Finished tests" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
