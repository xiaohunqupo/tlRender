// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <CoreTest/ColorTest.h>

#include <tlRender/Core/Color.h>

#include <ftk/Core/Format.h>
#include <ftk/Core/String.h>

namespace tl
{
    namespace core_tests
    {
        ColorTest::ColorTest(const std::shared_ptr<ftk::Context>& context) :
            ITest(context, "core_tests::ColorTest")
        {}

        std::shared_ptr<ColorTest> ColorTest::create(const std::shared_ptr<ftk::Context>& context)
        {
            return std::shared_ptr<ColorTest>(new ColorTest(context));
        }

        void ColorTest::run()
        {
            std::vector<std::string> s;
            for (const auto& i : getLUTFormats())
            {
                s.push_back(ftk::Format("    * {0}: {1}").arg(i.first).arg(i.second));
            }
            _print(ftk::Format("LUT formats:\n{0}").arg(ftk::join(s, '\n')));
        }
    }
}
