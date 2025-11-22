// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <TestLib/ITest.h>

namespace tl
{
    namespace timeline_tests
    {
        class MemRefTest : public tests::ITest
        {
        protected:
            MemRefTest(const std::shared_ptr<ftk::Context>&);

        public:
            static std::shared_ptr<MemRefTest> create(const std::shared_ptr<ftk::Context>&);

            void run() override;
        };
    }
}
