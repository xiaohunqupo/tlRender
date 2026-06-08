// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <ftk/TestLib/ITest.h>

namespace tl
{
    namespace timeline_tests
    {
        class PlayerOptionsTest : public ftk::test::ITest
        {
        protected:
            PlayerOptionsTest(const std::shared_ptr<ftk::Context>&);

        public:
            static std::shared_ptr<PlayerOptionsTest> create(const std::shared_ptr<ftk::Context>&);

            void run() override;
        };
    }
}
