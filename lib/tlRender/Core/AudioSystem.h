// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Core/Audio.h>
#include <tlRender/Core/ISystem.h>

#include <ftk/Core/ObservableList.h>
#include <ftk/Core/Observable.h>

namespace tl
{
    namespace audio
    {
        //! Audio device ID.
        struct TL_API_TYPE DeviceID
        {
            int         number = -1;
            std::string name;

            TL_API bool operator == (const DeviceID&) const;
            TL_API bool operator != (const DeviceID&) const;
        };

        //! Audio device information.
        struct TL_API_TYPE  DeviceInfo
        {
            DeviceID    id;
            audio::Info info;

            TL_API bool operator == (const DeviceInfo&) const;
            TL_API bool operator != (const DeviceInfo&) const;
        };

        //! Audio system.
        class TL_API_TYPE System : public system::ISystem
        {
            FTK_NON_COPYABLE(System);

        protected:
            System(const std::shared_ptr<ftk::Context>&);

        public:
            TL_API virtual ~System();

            //! Create a new system.
            TL_API static std::shared_ptr<System> create(const std::shared_ptr<ftk::Context>&);

            //! Get the list of audio drivers.
            TL_API const std::vector<std::string>& getDrivers() const;

            //! Get the list of audio devices.
            TL_API const std::vector<DeviceInfo>& getDevices() const;

            //! Observe the list of audio devices.
            TL_API std::shared_ptr<ftk::IObservableList<DeviceInfo> > observeDevices() const;

            //! Get the default audio device.
            TL_API DeviceInfo getDefaultDevice() const;

            //! Observe the default audio device.
            TL_API std::shared_ptr<ftk::IObservable<DeviceInfo> > observeDefaultDevice() const;

            TL_API void tick() override;
            TL_API std::chrono::milliseconds getTickTime() const override;

        private:
            std::vector<DeviceInfo> _getDevices();
            DeviceInfo _getDefaultDevice();

            void _run();

            FTK_PRIVATE();
        };
    }
}
