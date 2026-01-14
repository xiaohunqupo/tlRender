// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <TestLib/ITest.h>

namespace tl
{
    namespace core_tests
    {
        class ColorTest : public tests::ITest
        {
        protected:
            ColorTest(const std::shared_ptr<ftk::Context>&);

        public:
            static std::shared_ptr<ColorTest> create(const std::shared_ptr<ftk::Context>&);

            void run() override;
        };
    }
}
