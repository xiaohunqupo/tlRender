// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/TimelineTest/AudioSystemTest.h>

#include <tlRender/Timeline/AudioSystem.h>

#include <ftk/Core/Context.h>

#include <sstream>

namespace tl
{
    namespace timeline_tests
    {
        AudioSystemTest::AudioSystemTest(const std::shared_ptr<ftk::Context>& context) :
            ITest(context, "timeline_tests::AudioSystemTest")
        {}

        std::shared_ptr<AudioSystemTest> AudioSystemTest::create(const std::shared_ptr<ftk::Context>& context)
        {
            return std::shared_ptr<AudioSystemTest>(new AudioSystemTest(context));
        }

        void AudioSystemTest::run()
        {
            auto system = _context->getSystem<AudioSystem>();
            for (const auto& i : system->getDrivers())
            {
                std::stringstream ss;
                ss << "api: " << i;
                _print(ss.str());
            }
            for (const auto& i : system->getDevices())
            {
                std::stringstream ss;
                ss << "device: " << i.id.number << " " << i.id.name;
                _print(ss.str());
            }
            {
                std::stringstream ss;
                const AudioDeviceInfo device = system->getDefaultDevice();
                ss << "default device: " << device.id.number << " " << device.id.name;
                _print(ss.str());
            }
        }
    }
}
