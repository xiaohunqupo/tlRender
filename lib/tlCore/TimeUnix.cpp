// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCore/Time.h>

#include <thread>

namespace tl
{
    namespace time
	{
        void sleep(const std::chrono::microseconds& value)
		{
            std::this_thread::sleep_for(value);
        }
    }
}
