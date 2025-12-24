// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Timeline/BackgroundOptions.h>
#include <tlRender/Timeline/ColorOptions.h>
#include <tlRender/Timeline/DisplayOptions.h>
#include <tlRender/Timeline/ForegroundOptions.h>
#include <tlRender/Timeline/Player.h>

#include <ftk/UI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Timeline viewport.
        class TL_API_TYPE Viewport : public ftk::IWidget
        {
            FTK_NON_COPYABLE(Viewport);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            Viewport();

        public:
            TL_API virtual ~Viewport();

            //! Create a new widget.
            TL_API static std::shared_ptr<Viewport> create(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the comparison options.
            TL_API const timeline::CompareOptions& getCompareOptions() const;

            //! Observe the comparison options.
            TL_API std::shared_ptr<ftk::IObservable<timeline::CompareOptions> > observeCompareOptions() const;

            //! Set the comparison options.
            TL_API void setCompareOptions(const timeline::CompareOptions&);

            //! Get the OpenColorIO options.
            TL_API const timeline::OCIOOptions& getOCIOOptions() const;

            //! Observe the OpenColorIO options.
            TL_API std::shared_ptr<ftk::IObservable<timeline::OCIOOptions> > observeOCIOOptions() const;

            //! Set the OpenColorIO options.
            TL_API void setOCIOOptions(const timeline::OCIOOptions&);

            //! Get the LUT options.
            TL_API const timeline::LUTOptions& getLUTOptions() const;

            //! Observe the LUT options.
            TL_API std::shared_ptr<ftk::IObservable<timeline::LUTOptions> > observeLUTOptions() const;

            //! Set the LUT options.
            TL_API void setLUTOptions(const timeline::LUTOptions&);

            //! Get the image options.
            TL_API const std::vector<ftk::ImageOptions>& getImageOptions() const;

            //! Observe the image options.
            TL_API std::shared_ptr<ftk::IObservableList<ftk::ImageOptions> > observeImageOptions() const;

            //! Set the image options.
            TL_API void setImageOptions(const std::vector<ftk::ImageOptions>&);

            //! Get the display options.
            TL_API const std::vector<timeline::DisplayOptions>& getDisplayOptions() const;

            //! Observe the display options.
            TL_API std::shared_ptr<ftk::IObservableList<timeline::DisplayOptions> > observeDisplayOptions() const;

            //! Set the display options.
            TL_API void setDisplayOptions(const std::vector<timeline::DisplayOptions>&);

            //! Get the background options.
            TL_API const timeline::BackgroundOptions& getBackgroundOptions() const;

            //! Observe the background options.
            TL_API std::shared_ptr<ftk::IObservable<timeline::BackgroundOptions> > observeBackgroundOptions() const;

            //! Set the background options.
            TL_API void setBackgroundOptions(const timeline::BackgroundOptions&);

            //! Get the foreground options.
            TL_API const timeline::ForegroundOptions& getForegroundOptions() const;

            //! Observe the foreground options.
            TL_API std::shared_ptr<ftk::IObservable<timeline::ForegroundOptions> > observeForegroundOptions() const;

            //! Set the foreground options.
            TL_API void setForegroundOptions(const timeline::ForegroundOptions&);

            //! Get the color buffer type.
            TL_API ftk::ImageType getColorBuffer() const;

            //! Observe the color buffer type.
            TL_API std::shared_ptr<ftk::IObservable<ftk::ImageType> > observeColorBuffer() const;

            //! Set the color buffer type.
            TL_API void setColorBuffer(ftk::ImageType);

            //! Get the timeline player.
            TL_API const std::shared_ptr<timeline::Player>& getPlayer() const;

            //! Set the timeline player.
            TL_API virtual void setPlayer(const std::shared_ptr<timeline::Player>&);

            //! Get the view position.
            TL_API const ftk::V2I& getViewPos() const;

            //! Observe the view position.
            TL_API std::shared_ptr<ftk::IObservable<ftk::V2I> > observeViewPos() const;

            //! Get the view zoom.
            TL_API double getViewZoom() const;

            //! Observe the view zoom.
            TL_API std::shared_ptr<ftk::IObservable<double> > observeViewZoom() const;

            //! Get the view position and zoom.
            TL_API std::pair<ftk::V2I, double> getViewPosAndZoom() const;

            //! Observe the view position and zoom.
            TL_API std::shared_ptr<ftk::IObservable<std::pair<ftk::V2I, double> > > observeViewPosAndZoom() const;

            //! Set the view position and zoom.
            TL_API void setViewPosAndZoom(const ftk::V2I&, double);

            //! Set the view zoom.
            TL_API void setViewZoom(double, const ftk::V2I& focus = ftk::V2I());

            //! Get whether the view is framed automatically.
            TL_API bool hasFrameView() const;

            //! Observe whether the view is framed automatically.
            TL_API std::shared_ptr<ftk::IObservable<bool> > observeFrameView() const;

            //! Observe when the view is framed.
            TL_API std::shared_ptr<ftk::IObservable<bool> > observeFramed() const;

            //! Set whether the view is framed automatically.
            TL_API void setFrameView(bool);

            //! Reset the view zoom to 1:1.
            TL_API void viewZoomReset();

            //! Zoom the view in.
            TL_API void viewZoomIn();

            //! Zoom the view out.
            TL_API void viewZoomOut();

            //! Get the frames per second.
            TL_API double getFPS() const;

            //! Observe the frames per second.
            TL_API std::shared_ptr<ftk::IObservable<double> > observeFPS() const;

            //! Get the number of dropped frames during playback.
            TL_API size_t getDroppedFrames() const;

            //! Observe the number of dropped frames during playback.
            TL_API std::shared_ptr<ftk::IObservable<size_t> > observeDroppedFrames() const;
            
            //! Sample a color from the viewport.
            TL_API ftk::Color4F getColorSample(const ftk::V2I&);

            //! Set the pan binding.
            TL_API void setPanBinding(ftk::MouseButton, ftk::KeyModifier);

            //! Set the wipe binding.
            TL_API void setWipeBinding(ftk::MouseButton, ftk::KeyModifier);

            //! Set the mouse wheel scale.
            TL_API void setMouseWheelScale(float);

            TL_API void setGeometry(const ftk::Box2I&) override;
            TL_API void sizeHintEvent(const ftk::SizeHintEvent&) override;
            TL_API void drawEvent(const ftk::Box2I&, const ftk::DrawEvent&) override;
            TL_API void mouseEnterEvent(ftk::MouseEnterEvent&) override;
            TL_API void mouseLeaveEvent() override;
            TL_API void mouseMoveEvent(ftk::MouseMoveEvent&) override;
            TL_API void mousePressEvent(ftk::MouseClickEvent&) override;
            TL_API void mouseReleaseEvent(ftk::MouseClickEvent&) override;
            TL_API void scrollEvent(ftk::ScrollEvent&) override;
            TL_API void keyPressEvent(ftk::KeyEvent&) override;
            TL_API void keyReleaseEvent(ftk::KeyEvent&) override;

        protected:
            bool _isMouseInside() const;
            const ftk::V2I& _getMousePressPos() const;

        private:
            ftk::Size2I _getRenderSize() const;
            ftk::V2I _getViewportCenter() const;
            void _frameView();

            void _droppedFramesUpdate(const OTIO_NS::RationalTime&);

            FTK_PRIVATE();
        };
    }
}
