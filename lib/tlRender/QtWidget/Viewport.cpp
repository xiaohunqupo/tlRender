// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/QtWidget/Viewport.h>

#include <tlRender/UI/Viewport.h>

namespace tl
{
    namespace qtwidget
    {
        struct Viewport::Private
        {
            std::shared_ptr<ui::Viewport> viewport;
        };

        Viewport::Viewport(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<ftk::Style>& style,
            QWidget* parent) :
            ContainerWidget(context, style, parent),
            _p(new Private)
        {
            FTK_P();
            p.viewport = ui::Viewport::create(context);
            setWidget(p.viewport);
        }

        Viewport::~Viewport()
        {}

        ftk::gl::TextureType Viewport::colorBuffer() const
        {
            return _p->viewport->getColorBuffer();
        }

        const ftk::V2I& Viewport::viewPos() const
        {
            return _p->viewport->getViewPos();
        }

        double Viewport::viewZoom() const
        {
            return _p->viewport->getViewZoom();
        }

        bool Viewport::hasFrameView() const
        {
            return _p->viewport->hasFrameView();
        }

        double Viewport::getFPS() const
        {
            return _p->viewport->getFPS();
        }

        size_t Viewport::getDroppedFrames() const
        {
            return _p->viewport->getDroppedFrames();
        }

        void Viewport::setOCIOOptions(const OCIOOptions& value)
        {
            _p->viewport->setOCIOOptions(value);
        }

        void Viewport::setLUTOptions(const LUTOptions& value)
        {
            _p->viewport->setLUTOptions(value);
        }

        void Viewport::setImageOptions(const std::vector<ftk::ImageOptions>& value)
        {
            _p->viewport->setImageOptions(value);
        }

        void Viewport::setDisplayOptions(const std::vector<DisplayOptions>& value)
        {
            _p->viewport->setDisplayOptions(value);
        }

        void Viewport::setCompareOptions(const CompareOptions& value)
        {
            _p->viewport->setCompareOptions(value);
        }

        void Viewport::setBackgroundOptions(const BackgroundOptions& value)
        {
            _p->viewport->setBackgroundOptions(value);
        }

        void Viewport::setForegroundOptions(const ForegroundOptions& value)
        {
            _p->viewport->setForegroundOptions(value);
        }

        void Viewport::setColorBuffer(ftk::gl::TextureType value)
        {
            _p->viewport->setColorBuffer(value);
        }

        void Viewport::setPlayer(const QSharedPointer<qt::PlayerObject>& value)
        {
            _p->viewport->setPlayer(value ? value->player() : nullptr);
        }

        void Viewport::setViewPosAndZoom(const ftk::V2I& pos, double zoom)
        {
            _p->viewport->setViewPosAndZoom(pos, zoom);
        }

        void Viewport::setViewZoom(double zoom, const ftk::V2I& focus)
        {
            _p->viewport->setViewZoom(zoom, focus);
        }

        void Viewport::setFrameView(bool value)
        {
            _p->viewport->setFrameView(value);
        }
        
        void Viewport::viewZoomReset()
        {
            _p->viewport->viewZoomReset();
        }

        void Viewport::viewZoomIn()
        {
            _p->viewport->viewZoomIn();
        }

        void Viewport::viewZoomOut()
        {
            _p->viewport->viewZoomOut();
        }
    }
}
