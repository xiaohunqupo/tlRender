// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace CoreTest
    {
        class ImageTest : public Test::ITest
        {
        protected:
            ImageTest(const std::shared_ptr<core::Context>&);

        public:
            static std::shared_ptr<ImageTest> create(const std::shared_ptr<core::Context>&);

            void run() override;
            
        private:
            void _size();
            void _enums();
            void _info();
            void _util();
            void _image();
        };
    }
}