// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace core_tests
    {
        class FontSystemTest : public tests::ITest
        {
        protected:
            FontSystemTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<FontSystemTest> create(const std::shared_ptr<system::Context>&);

            void run() override;
        };
    }
}