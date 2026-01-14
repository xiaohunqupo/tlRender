// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#if defined(TLRENDER_QT5) || defined(TLRENDER_QT6)
#include <QtTest/TimeObjectTest.h>
#include <tlRender/Qt/Init.h>
#endif // TLRENDER_QT5 || TLRENDER_QT6

#include <TimelineTest/ColorOptionsTest.h>
#include <TimelineTest/CompareOptionsTest.h>
#include <TimelineTest/DisplayOptionsTest.h>
#include <TimelineTest/MemRefTest.h>
#include <TimelineTest/PlayerOptionsTest.h>
#include <TimelineTest/PlayerTest.h>
#include <TimelineTest/TimelineTest.h>
#include <TimelineTest/UtilTest.h>

#include <IOTest/IOTest.h>
#if defined(TLRENDER_FFMPEG)
#include <IOTest/FFmpegTest.h>
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_EXR)
#include <IOTest/EXRTest.h>
#endif // TLRENDER_EXR
#if defined(TLRENDER_OIIO)
#include <IOTest/OIIOTest.h>
#endif // TLRENDER_OIIO

#include <CoreTest/AudioTest.h>
#include <CoreTest/HDRTest.h>
#include <CoreTest/TimeTest.h>
#include <CoreTest/URLTest.h>

#include <tlRender/Timeline/Init.h>

#include <ftk/Core/Context.h>

#include <iostream>
#include <vector>

using namespace tl;
using namespace tl::tests;

void coreTests(
    std::vector<std::shared_ptr<tests::ITest> >& tests,
    const std::shared_ptr<ftk::Context>& context)
{
    tests.push_back(core_tests::AudioTest::create(context));
    tests.push_back(core_tests::HDRTest::create(context));
    tests.push_back(core_tests::TimeTest::create(context));
    tests.push_back(core_tests::URLTest::create(context));
}

void ioTests(
    std::vector<std::shared_ptr<tests::ITest> >& tests,
    const std::shared_ptr<ftk::Context>& context)
{
    tests.push_back(io_tests::IOTest::create(context));
#if defined(TLRENDER_FFMPEG)
    tests.push_back(io_tests::FFmpegTest::create(context));
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_OIIO)
    tests.push_back(io_tests::OIIOTest::create(context));
#endif // TLRENDER_OIIO
#if defined(TLRENDER_EXR)
    tests.push_back(io_tests::EXRTest::create(context));
#endif // TLRENDER_EXR
}

void timelineTests(
    std::vector<std::shared_ptr<tests::ITest> >& tests,
    const std::shared_ptr<ftk::Context>& context)
{
    tests.push_back(timeline_tests::ColorOptionsTest::create(context));
    tests.push_back(timeline_tests::CompareOptionsTest::create(context));
    tests.push_back(timeline_tests::DisplayOptionsTest::create(context));
    tests.push_back(timeline_tests::MemRefTest::create(context));
    tests.push_back(timeline_tests::PlayerOptionsTest::create(context));
    tests.push_back(timeline_tests::PlayerTest::create(context));
    tests.push_back(timeline_tests::TimelineTest::create(context));
    tests.push_back(timeline_tests::UtilTest::create(context));
}

void qtTests(
    std::vector<std::shared_ptr<tests::ITest> >& tests,
    const std::shared_ptr<ftk::Context>& context)
{
#if defined(TLRENDER_QT5) || defined(TLRENDER_QT6)
    tests.push_back(qt_tests::TimeObjectTest::create(context));
#endif // TLRENDER_QT5 || TLRENDER_QT6
}

int main(int argc, char* argv[])
{
    auto context = ftk::Context::create();
#if defined(TLRENDER_QT5) || defined(TLRENDER_QT6)
    qt::init(
        context,
        qt::DefaultSurfaceFormat::OpenGL_4_1_CoreProfile);
#else // TLRENDER_QT5 || TLRENDER_QT6
    tl::init(context);
#endif // TLRENDER_QT5 || TLRENDER_QT6

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

    std::vector<std::shared_ptr<tests::ITest> > tests;
    coreTests(tests, context);
    ioTests(tests, context);
    timelineTests(tests, context);
    qtTests(tests, context);

    for (const auto& test : tests)
    {
        std::cout << "Running test: " << test->getName() << std::endl;
        test->run();
        context->tick();
    }

    std::cout << "Finished tests" << std::endl;
    return 0;
}
