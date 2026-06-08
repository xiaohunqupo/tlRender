// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <ftk/TestLib/ITest.h>

namespace tl
{
    namespace io_tests
    {
        class OIIOTest : public ftk::test::ITest
        {
        protected:
            OIIOTest(const std::shared_ptr<ftk::Context>&);

        public:
            static std::shared_ptr<OIIOTest> create(const std::shared_ptr<ftk::Context>&);

            void run() override;
        };
    }
}
