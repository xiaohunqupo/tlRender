// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace core_tests
    {
        class HDRTest : public tests::ITest
        {
        protected:
            HDRTest(const std::shared_ptr<feather_tk::Context>&);

        public:
            static std::shared_ptr<HDRTest> create(const std::shared_ptr<feather_tk::Context>&);

            void run() override;

        private:
            void _enums();
            void _operators();
            void _serialize();
        };
    }
}
