// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <ftk/TestLib/ITest.h>

namespace tl
{
    namespace qt_tests
    {
        class TimeObjectTest : public ftk::test::ITest
        {
        protected:
            TimeObjectTest(const std::shared_ptr<ftk::Context>&);

        public:
            static std::shared_ptr<TimeObjectTest> create(const std::shared_ptr<ftk::Context>&);

            void run() override;
        };
    }
}
