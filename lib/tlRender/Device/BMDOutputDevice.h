// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Device/BMDData.h>

#include <tlRender/Timeline/IRender.h>
#include <tlRender/Timeline/Player.h>

namespace tl
{
    namespace bmd
    {
        class System;

        //! Frame rate.
        struct TL_API_TYPE FrameRate
        {
            int num = 0;
            int den = 0;

            TL_API bool operator == (const FrameRate&) const;
            TL_API bool operator != (const FrameRate&) const;
        };

        //! BMD output device.
        class TL_API_TYPE OutputDevice : public std::enable_shared_from_this<OutputDevice>
        {
            FTK_NON_COPYABLE(OutputDevice);

        protected:
            void _init(const std::shared_ptr<ftk::Context>&);

            OutputDevice();

        public:
            TL_API ~OutputDevice();

            //! Create a new output device.
            TL_API static std::shared_ptr<OutputDevice> create(const std::shared_ptr<ftk::Context>&);

            //! Get the device configuration.
            TL_API DeviceConfig getConfig() const;

            //! Observe the device configuration.
            TL_API std::shared_ptr<ftk::IObservable<DeviceConfig> > observeConfig() const;

            //! Set the device configuration.
            TL_API void setConfig(const DeviceConfig&);

            //! Get whether the device is enabled.
            TL_API bool isEnabled() const;

            //! Observe whether the device is enabled.
            TL_API std::shared_ptr<ftk::IObservable<bool> > observeEnabled() const;

            //! Set whether the device is enabled.
            TL_API void setEnabled(bool);

            //! Get whether the device is active.
            TL_API bool isActive() const;

            //! Observe whether the device is active.
            TL_API std::shared_ptr<ftk::IObservable<bool> > observeActive() const;

            //! Get the video size.
            TL_API const ftk::Size2I& getSize() const;

            //! Observe the video size.
            TL_API std::shared_ptr<ftk::IObservable<ftk::Size2I> > observeSize() const;

            //! Get the frame rate.
            TL_API const FrameRate& getFrameRate() const;

            //! Observe the frame rate.
            TL_API std::shared_ptr<ftk::IObservable<FrameRate> > observeFrameRate() const;

            //! Get the video frame delay.
            TL_API int getVideoFrameDelay() const;

            //! Observe the video frame delay.
            TL_API std::shared_ptr<ftk::IObservable<int> > observeVideoFrameDelay() const;

            //! Set the view.
            TL_API void setView(
                const ftk::V2I& position,
                double          zoom,
                bool            frame);

            //! Set the OpenColorIO options.
            TL_API void setOCIOOptions(const timeline::OCIOOptions&);

            //! Set the LUT options.
            TL_API void setLUTOptions(const timeline::LUTOptions&);

            //! Set the image options.
            TL_API void setImageOptions(const std::vector<ftk::ImageOptions>&);

            //! Set the display options.
            TL_API void setDisplayOptions(const std::vector<timeline::DisplayOptions>&);

            //! Set the HDR mode and metadata.
            TL_API void setHDR(HDRMode, const image::HDRData&);

            //! Set the comparison options.
            TL_API void setCompareOptions(const timeline::CompareOptions&);

            //! Set the background options.
            TL_API void setBackgroundOptions(const timeline::BackgroundOptions&);

            //! Set the foreground options.
            TL_API void setForegroundOptions(const timeline::ForegroundOptions&);

            //! Set the overlay.
            TL_API void setOverlay(const std::shared_ptr<ftk::Image>&);

            //! Set the audio volume.
            TL_API void setVolume(float);

            //! Set the audio mute.
            TL_API void setMute(bool);

            //! Set the audio channels mute.
            TL_API void setChannelMute(const std::vector<bool>&);

            //! Set the audio sync offset.
            TL_API void setAudioOffset(double);

            //! Set the timeline player.
            TL_API void setPlayer(const std::shared_ptr<timeline::Player>&);

        private:
            void _tick();
            void _run();
            void _createDevice(
                const DeviceConfig&,
                bool& active,
                ftk::Size2I& size,
                FrameRate& frameRate,
                int videoFrameDelay);
            void _render(
                const DeviceConfig&,
                const timeline::OCIOOptions&,
                const timeline::LUTOptions&,
                const std::vector<ftk::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&,
                const timeline::BackgroundOptions&,
                const timeline::ForegroundOptions&);
            void _read();

            friend class System;

            FTK_PRIVATE();
        };
    }
}
