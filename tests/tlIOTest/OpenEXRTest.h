// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace io_tests
    {
        class OpenEXRTest : public tests::ITest
        {
        protected:
            OpenEXRTest(const std::shared_ptr<feather_tk::Context>&);

        public:
            static std::shared_ptr<OpenEXRTest> create(const std::shared_ptr<feather_tk::Context>&);

            void run() override;

        private:
            void _enums();
            void _util();
            void _io();
        };
    }
}
