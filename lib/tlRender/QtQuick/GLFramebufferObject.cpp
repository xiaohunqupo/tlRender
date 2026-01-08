// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/QtQuick/GLFramebufferObject.h>

#include <tlRender/QtQuick/Init.h>

#include <tlRender/GL/Render.h>

#include <ftk/GL/Init.h>
#include <ftk/Core/Context.h>

#include <QOpenGLFramebufferObject>

#include <QQuickWindow>

namespace tl
{
    namespace qtquick
    {
        namespace
        {
            class Renderer : public QQuickFramebufferObject::Renderer
            {
            public:
                Renderer(
                    const GLFramebufferObject* framebufferObject) :
                    _framebufferObject(framebufferObject)
                {}

                virtual ~Renderer()
                {}

                QOpenGLFramebufferObject* createFramebufferObject(const QSize& size) override
                {
                    return QQuickFramebufferObject::Renderer::createFramebufferObject(size);
                }

                void render() override
                {
                    if (!_init)
                    {
                        _init = true;
                        ftk::gl::initGLAD();
                        _render = timeline_gl::Render::create(
                            qtquick::getContext()->getLogSystem(),
                            qtquick::getContext()->getSystem<ftk::FontSystem>());
                    }

                    QOpenGLFramebufferObject* fbo = framebufferObject();
                    const ftk::Size2I size(fbo->width(), fbo->height());
                    _render->begin(size);
                    if (!_video.empty())
                    {
                        _render->drawVideo(
                            { _video.front() },
                            { ftk::Box2I(0, 0, size.w, size.h) });
                    }
                    _render->end();

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
                    _framebufferObject->window()->resetOpenGLState();
#endif // QT_VERSION
                }

                void synchronize(QQuickFramebufferObject*) override
                {
                    _video = _framebufferObject->video();
                }

            private:
                const GLFramebufferObject* _framebufferObject = nullptr;
                bool _init = false;
                std::vector<VideoFrame> _video;
                std::shared_ptr<IRender> _render;
            };
        }

        struct GLFramebufferObject::Private
        {
            std::vector<VideoFrame> video;
        };

        GLFramebufferObject::GLFramebufferObject(QQuickItem* parent) :
            QQuickFramebufferObject(parent),
            _p(new Private)
        {
            setMirrorVertically(true);
        }

        GLFramebufferObject::~GLFramebufferObject()
        {}

        const std::vector<VideoFrame>& GLFramebufferObject::video() const
        {
            return _p->video;
        }

        QQuickFramebufferObject::Renderer* GLFramebufferObject::createRenderer() const
        {
            return new qtquick::Renderer(this);
        }

        void GLFramebufferObject::setVideo(const std::vector<VideoFrame>& value)
        {
            _p->video = value;
            update();
        }
    }
}
