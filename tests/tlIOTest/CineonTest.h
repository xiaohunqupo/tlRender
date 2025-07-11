// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace io_tests
    {
        class CineonTest : public tests::ITest
        {
        protected:
            CineonTest(const std::shared_ptr<feather_tk::Context>&);

        public:
            static std::shared_ptr<CineonTest> create(const std::shared_ptr<feather_tk::Context>&);

            void run() override;

        private:
            void _enums();
            void _io();
        };
    }
}
