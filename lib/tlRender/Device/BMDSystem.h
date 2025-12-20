// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Device/BMDData.h>

#include <ftk/Core/ISystem.h>
#include <ftk/Core/ObservableList.h>

namespace tl
{
    namespace bmd
    {
        class OutputDevice;

        //! BMD system.
        class System : public ftk::ISystem
        {
            FTK_NON_COPYABLE(System);

        protected:
            System(const std::shared_ptr<ftk::Context>&);

        public:
            TL_API ~System() override;

            //! Create a new system.
            TL_API static std::shared_ptr<System> create(const std::shared_ptr<ftk::Context>&);

            //! Observe the device information.
            TL_API std::shared_ptr<ftk::IObservableList<DeviceInfo> > observeDeviceInfo() const;

            TL_API void tick() override;
            TL_API std::chrono::milliseconds getTickTime() const override;

        private:
            void _addDevice(const std::shared_ptr<OutputDevice>&);

            friend class OutputDevice;

            FTK_PRIVATE();
        };
    }
}
