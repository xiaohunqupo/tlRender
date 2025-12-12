// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Device/BMDData.h>

#include <ftk/Core/Image.h>
#include <ftk/Core/Observable.h>

#include <memory>
#include <string>

namespace ftk
{
    class Context;
}

namespace tl
{
    namespace bmd
    {
        //! BMD devices model data.
        struct TL_API_TYPE DevicesModelData
        {
            std::vector<std::string> devices;
            int                      deviceIndex = 0;
            std::vector<std::string> displayModes;
            int                      displayModeIndex = 0;
            std::vector<PixelType>   pixelTypes;
            int                      pixelTypeIndex = 0;
            bool                     deviceEnabled = true;
            BoolOptions              boolOptions;
            ftk::VideoLevels         videoLevels = ftk::VideoLevels::LegalRange;
            HDRMode                  hdrMode = HDRMode::FromFile;
            image::HDRData           hdrData;

            TL_API bool operator == (const DevicesModelData&) const;
        };

        //! BMD devices model.
        class TL_API_TYPE DevicesModel : public std::enable_shared_from_this<DevicesModel>
        {
            FTK_NON_COPYABLE(DevicesModel);

        protected:
            void _init(const std::shared_ptr<ftk::Context>&);

            DevicesModel();

        public:
            TL_API ~DevicesModel();

            //! Create a new device model.
            TL_API static std::shared_ptr<DevicesModel> create(
                const std::shared_ptr<ftk::Context>&);

            //! Observe the model data.
            TL_API std::shared_ptr<ftk::IObservable<DevicesModelData> > observeData() const;

            //! Set the device index.
            TL_API void setDeviceIndex(int);

            //! Set the display mode index.
            TL_API void setDisplayModeIndex(int);

            //! Set the pixel type index.
            TL_API void setPixelTypeIndex(int);

            //! Set whether the device is enabled.
            TL_API void setDeviceEnabled(bool);

            //! Set the boolean options.
            TL_API void setBoolOptions(const BoolOptions&);

            //! Set the video levels.
            TL_API void setVideoLevels(ftk::VideoLevels);

            //! Set the HDR mode.
            TL_API void setHDRMode(HDRMode);

            //! Set the HDR data.
            TL_API void setHDRData(const image::HDRData&);

        private:
            void _update();

            FTK_PRIVATE();
        };

        TL_API void to_json(nlohmann::json&, const DevicesModelData&);

        TL_API void from_json(const nlohmann::json&, DevicesModelData&);
    }
}
