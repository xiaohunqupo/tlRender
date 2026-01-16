// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/UI/ThumbnailSystem.h>

#include <tlRender/GL/Render.h>

#include <tlRender/Timeline/Timeline.h>

#include <tlRender/IO/System.h>

#include <tlRender/Core/AudioResample.h>

#include <ftk/GL/GL.h>
#include <ftk/GL/Window.h>
#include <ftk/GL/OffscreenBuffer.h>
#include <ftk/Core/Context.h>
#include <ftk/Core/Format.h>
#include <ftk/Core/LRUCache.h>
#include <ftk/Core/String.h>

#include <sstream>

namespace tl
{
    namespace ui
    {
        namespace
        {
            const size_t ioCacheMax = 2;
        }

        struct ThumbnailCache::Private
        {
            size_t max = 16;
            ftk::LRUCache<std::string, IOInfo> info;
            ftk::LRUCache<std::string, std::shared_ptr<ftk::Image> > thumbnails;
            ftk::LRUCache<std::string, std::shared_ptr<ftk::TriMesh2F> > waveforms;
            std::mutex mutex;
        };

        void ThumbnailCache::_init(const std::shared_ptr<ftk::Context>& context)
        {
            _maxUpdate();
        }

        ThumbnailCache::ThumbnailCache() :
            _p(new Private)
        {}

        ThumbnailCache::~ThumbnailCache()
        {}

        std::shared_ptr<ThumbnailCache> ThumbnailCache::create(
            const std::shared_ptr<ftk::Context>& context)
        {
            auto out = std::shared_ptr<ThumbnailCache>(new ThumbnailCache);
            out->_init(context);
            return out;
        }

        size_t ThumbnailCache::getMax() const
        {
            return _p->max;
        }

        void ThumbnailCache::setMax(size_t value)
        {
            FTK_P();
            if (value == p.max)
                return;
            p.max = value;
            _maxUpdate();
        }

        size_t ThumbnailCache::getSize() const
        {
            FTK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.info.getSize() + p.thumbnails.getSize() + p.waveforms.getSize();
        }

        float ThumbnailCache::getPercentage() const
        {
            FTK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return
                (p.info.getSize() + p.thumbnails.getSize() + p.waveforms.getSize()) /
                static_cast<float>(p.info.getMax() + p.thumbnails.getMax() + p.waveforms.getMax()) * 100.F;
        }

        std::string ThumbnailCache::getInfoKey(
            intptr_t id,
            const ftk::Path& path,
            const IOOptions& options)
        {
            std::stringstream ss;
            ss << id << ";" << path.get() << ";";
            for (const auto& i : options)
            {
                ss << i.first << ":" << i.second << ";";
            }
            return ss.str();
        }

        void ThumbnailCache::addInfo(const std::string& key, const IOInfo& info)
        {
            FTK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.info.add(key, info);
        }

        bool ThumbnailCache::containsInfo(const std::string& key)
        {
            FTK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.info.contains(key);
        }

        bool ThumbnailCache::getInfo(const std::string& key, IOInfo& info) const
        {
            FTK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.info.get(key, info);
        }

        std::string ThumbnailCache::getThumbnailKey(
            intptr_t id,
            const ftk::Path& path,
            int height,
            const OTIO_NS::RationalTime& time,
            const IOOptions& options)
        {
            std::stringstream ss;
            ss << id << ";" << path.get() << ";" << height << ";" << time << ";";
            for (const auto& i : options)
            {
                ss << i.first << ":" << i.second << ";";
            }
            return ss.str();
        }

        void ThumbnailCache::addThumbnail(
            const std::string& key,
            const std::shared_ptr<ftk::Image>& thumbnail)
        {
            FTK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.thumbnails.add(key, thumbnail);
        }

        bool ThumbnailCache::containsThumbnail(const std::string& key)
        {
            FTK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.thumbnails.contains(key);
        }

        bool ThumbnailCache::getThumbnail(
            const std::string& key,
            std::shared_ptr<ftk::Image>& thumbnail) const
        {
            FTK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.thumbnails.get(key, thumbnail);
        }

        std::string ThumbnailCache::getWaveformKey(
            intptr_t id,
            const ftk::Path& path,
            const ftk::Size2I& size,
            const OTIO_NS::TimeRange& timeRange,
            const IOOptions& options)
        {
            std::stringstream ss;
            ss << id << ";" << path.get() << ";" << size << ";" << timeRange << ";";
            for (const auto& i : options)
            {
                ss << i.first << ":" << i.second << ";";
            }
            return ss.str();
        }

        void ThumbnailCache::addWaveform(
            const std::string& key,
            const std::shared_ptr<ftk::TriMesh2F>& waveform)
        {
            FTK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.waveforms.add(key, waveform);
        }

        bool ThumbnailCache::containsWaveform(const std::string& key)
        {
            FTK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.waveforms.contains(key);
        }

        bool ThumbnailCache::getWaveform(
            const std::string& key,
            std::shared_ptr<ftk::TriMesh2F>& waveform) const
        {
            FTK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.waveforms.get(key, waveform);
        }

        void ThumbnailCache::clear()
        {
            FTK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.info.clear();
            p.thumbnails.clear();
            p.waveforms.clear();
        }

        void ThumbnailCache::_maxUpdate()
        {
            FTK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.info.setMax(p.max);
            p.thumbnails.setMax(p.max);
            p.waveforms.setMax(p.max);
        }

        struct ThumbnailGenerator::Private
        {
            std::weak_ptr<ftk::Context> context;
            std::shared_ptr<ThumbnailCache> cache;
            std::shared_ptr<ftk::gl::Window> window;
            uint64_t requestId = 0;

            struct InfoRequest
            {
                uint64_t id = 0;
                intptr_t callerId = 0;
                ftk::Path path;
                std::vector<ftk::MemFile> memoryRead;
                IOOptions options;
                std::promise<IOInfo> promise;
            };

            struct ThumbnailRequest
            {
                uint64_t id = 0;
                intptr_t callerId = 0;
                ftk::Path path;
                std::vector<ftk::MemFile> memoryRead;
                int height = 0;
                OTIO_NS::RationalTime time = invalidTime;
                IOOptions options;
                std::promise<std::shared_ptr<ftk::Image> > promise;
            };

            struct WaveformRequest
            {
                uint64_t id = 0;
                intptr_t callerId = 0;
                ftk::Path path;
                std::vector<ftk::MemFile> memoryRead;
                ftk::Size2I size;
                OTIO_NS::TimeRange timeRange = invalidTimeRange;
                IOOptions options;
                std::promise<std::shared_ptr<ftk::TriMesh2F> > promise;
            };

            struct InfoMutex
            {
                std::list<std::shared_ptr<InfoRequest> > requests;
                bool stopped = false;
                std::mutex mutex;
            };
            InfoMutex infoMutex;

            struct ThumbnailMutex
            {
                std::list<std::shared_ptr<ThumbnailRequest> > requests;
                bool stopped = false;
                std::mutex mutex;
            };
            ThumbnailMutex thumbnailMutex;

            struct WaveformMutex
            {
                std::list<std::shared_ptr<WaveformRequest> > requests;
                bool stopped = false;
                std::mutex mutex;
            };
            WaveformMutex waveformMutex;

            struct InfoThread
            {
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            InfoThread infoThread;

            struct ThumbnailThread
            {
                std::shared_ptr<gl::Render> render;
                std::shared_ptr<ftk::gl::OffscreenBuffer> buffer;
                ftk::LRUCache<std::string, std::shared_ptr<IRead> > ioCache;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            ThumbnailThread thumbnailThread;

            struct WaveformThread
            {
                ftk::LRUCache<std::string, std::shared_ptr<IRead> > ioCache;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            WaveformThread waveformThread;
        };

        void ThumbnailGenerator::_init(
            const std::shared_ptr<ThumbnailCache>& cache,
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<ftk::gl::Window>& window)
        {
            FTK_P();
            
            p.context = context;

            p.cache = cache;

            p.window = window;
            if (!p.window)
            {
                p.window = ftk::gl::Window::create(
                    context,
                    "tl::ui::ThumbnailGenerator",
                    ftk::Size2I(1, 1),
                    static_cast<int>(ftk::gl::WindowOptions::None));
            }

            p.infoThread.running = true;
            p.infoThread.thread = std::thread(
                [this]
                {
                    FTK_P();
                    while (p.infoThread.running)
                    {
                        _infoRun();
                    }
                    {
                        std::unique_lock<std::mutex> lock(p.infoMutex.mutex);
                        p.infoMutex.stopped = true;
                    }
                    _infoCancel();
                });

            p.thumbnailThread.ioCache.setMax(ioCacheMax);
            p.thumbnailThread.running = true;
            p.thumbnailThread.thread = std::thread(
                [this]
                {
                    FTK_P();
                    p.window->makeCurrent();
                    if (auto context = p.context.lock())
                    {
                        p.thumbnailThread.render = gl::Render::create(
                            context->getLogSystem(),
                            context->getSystem<ftk::FontSystem>());
                    }
                    if (p.thumbnailThread.render)
                    {
                        while (p.thumbnailThread.running)
                        {
                            _thumbnailRun();
                        }
                    }
                    {
                        std::unique_lock<std::mutex> lock(p.thumbnailMutex.mutex);
                        p.thumbnailMutex.stopped = true;
                    }
                    p.thumbnailThread.buffer.reset();
                    p.thumbnailThread.render.reset();
                    _thumbnailCancel();
                    p.window->clearCurrent();
                });

            p.waveformThread.ioCache.setMax(ioCacheMax);
            p.waveformThread.running = true;
            p.waveformThread.thread = std::thread(
                [this]
                {
                    FTK_P();
                    while (p.waveformThread.running)
                    {
                        _waveformRun();
                    }
                    {
                        std::unique_lock<std::mutex> lock(p.waveformMutex.mutex);
                        p.waveformMutex.stopped = true;
                    }
                    _waveformCancel();
                });
        }

        ThumbnailGenerator::ThumbnailGenerator() :
            _p(new Private)
        {}

        ThumbnailGenerator::~ThumbnailGenerator()
        {
            FTK_P();
            p.infoThread.running = false;
            p.thumbnailThread.running = false;
            p.waveformThread.running = false;
            if (p.infoThread.thread.joinable())
            {
                p.infoThread.thread.join();
            }
            if (p.thumbnailThread.thread.joinable())
            {
                p.thumbnailThread.thread.join();
            }
            if (p.waveformThread.thread.joinable())
            {
                p.waveformThread.thread.join();
            }
        }

        std::shared_ptr<ThumbnailGenerator> ThumbnailGenerator::create(
            const std::shared_ptr<ThumbnailCache>& cache,
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<ftk::gl::Window>& window)
        {
            auto out = std::shared_ptr<ThumbnailGenerator>(new ThumbnailGenerator);
            out->_init(cache, context, window);
            return out;
        }

        InfoRequest ThumbnailGenerator::getInfo(
            intptr_t id,
            const ftk::Path& path,
            const IOOptions& options)
        {
            return getInfo(id, path, {}, options);
        }

        InfoRequest ThumbnailGenerator::getInfo(
            intptr_t id,
            const ftk::Path& path,
            const std::vector<ftk::MemFile>& memoryRead,
            const IOOptions& options)
        {
            FTK_P();
            (p.requestId)++;
            auto request = std::make_shared<Private::InfoRequest>();
            request->id = p.requestId;
            request->callerId = id;
            request->path = path;
            request->memoryRead = memoryRead;
            request->options = options;
            InfoRequest out;
            out.id = p.requestId;
            out.future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.infoMutex.mutex);
                if (!p.infoMutex.stopped)
                {
                    valid = true;
                    p.infoMutex.requests.push_back(request);
                }
            }
            if (valid)
            {
                p.infoThread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(IOInfo());
            }
            return out;
        }

        ThumbnailRequest ThumbnailGenerator::getThumbnail(
            intptr_t id,
            const ftk::Path& path,
            int height,
            const OTIO_NS::RationalTime& time,
            const IOOptions& options)
        {
            return getThumbnail(id, path, {}, height, time, options);
        }

        ThumbnailRequest ThumbnailGenerator::getThumbnail(
            intptr_t id,
            const ftk::Path& path,
            const std::vector<ftk::MemFile>& memoryRead,
            int height,
            const OTIO_NS::RationalTime& time,
            const IOOptions& options)
        {
            FTK_P();
            (p.requestId)++;
            auto request = std::make_shared<Private::ThumbnailRequest>();
            request->id = p.requestId;
            request->callerId = id;
            request->path = path;
            request->memoryRead = memoryRead;
            request->height = height;
            request->time = time;
            request->options = options;
            ThumbnailRequest out;
            out.id = p.requestId;
            out.height = height;
            out.time = time;
            out.future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.thumbnailMutex.mutex);
                if (!p.thumbnailMutex.stopped)
                {
                    valid = true;
                    p.thumbnailMutex.requests.push_back(request);
                }
            }
            if (valid)
            {
                p.thumbnailThread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(nullptr);
            }
            return out;
        }

        WaveformRequest ThumbnailGenerator::getWaveform(
            intptr_t id,
            const ftk::Path& path,
            const ftk::Size2I& size,
            const OTIO_NS::TimeRange& range,
            const IOOptions& options)
        {
            return getWaveform(id, path, {}, size, range, options);
        }

        WaveformRequest ThumbnailGenerator::getWaveform(
            intptr_t id,
            const ftk::Path& path,
            const std::vector<ftk::MemFile>& memoryRead,
            const ftk::Size2I& size,
            const OTIO_NS::TimeRange& timeRange,
            const IOOptions& options)
        {
            FTK_P();
            (p.requestId)++;
            auto request = std::make_shared<Private::WaveformRequest>();
            request->id = p.requestId;
            request->callerId = id;
            request->path = path;
            request->memoryRead = memoryRead;
            request->size = size;
            request->timeRange = timeRange;
            request->options = options;
            WaveformRequest out;
            out.id = p.requestId;
            out.size = size;
            out.timeRange = timeRange;
            out.future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.waveformMutex.mutex);
                if (!p.waveformMutex.stopped)
                {
                    valid = true;
                    p.waveformMutex.requests.push_back(request);
                }
            }
            if (valid)
            {
                p.waveformThread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(nullptr);
            }
            return out;
        }

        void ThumbnailGenerator::cancelRequests(const std::vector<uint64_t>& ids)
        {
            FTK_P();
            {
                std::unique_lock<std::mutex> lock(p.infoMutex.mutex);
                auto i = p.infoMutex.requests.begin();
                while (i != p.infoMutex.requests.end())
                {
                    const auto j = std::find(ids.begin(), ids.end(), (*i)->id);
                    if (j != ids.end())
                    {
                        i = p.infoMutex.requests.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
            }
            {
                std::unique_lock<std::mutex> lock(p.thumbnailMutex.mutex);
                auto i = p.thumbnailMutex.requests.begin();
                while (i != p.thumbnailMutex.requests.end())
                {
                    const auto j = std::find(ids.begin(), ids.end(), (*i)->id);
                    if (j != ids.end())
                    {
                        i = p.thumbnailMutex.requests.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
            }
            {
                std::unique_lock<std::mutex> lock(p.waveformMutex.mutex);
                auto i = p.waveformMutex.requests.begin();
                while (i != p.waveformMutex.requests.end())
                {
                    const auto j = std::find(ids.begin(), ids.end(), (*i)->id);
                    if (j != ids.end())
                    {
                        i = p.waveformMutex.requests.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
            }
        }

        void ThumbnailGenerator::_infoRun()
        {
            FTK_P();
            std::shared_ptr<Private::InfoRequest> request;
            {
                std::unique_lock<std::mutex> lock(p.infoMutex.mutex);
                if (p.infoThread.cv.wait_for(
                    lock,
                    std::chrono::milliseconds(5),
                    [this]
                    {
                        return !_p->infoMutex.requests.empty();
                    }))
                {
                    request = p.infoMutex.requests.front();
                    p.infoMutex.requests.pop_front();
                }
            }
            if (request)
            {
                IOInfo info;
                const std::string key = ThumbnailCache::getInfoKey(
                    request->callerId,
                    request->path,
                    request->options);
                if (!p.cache->getInfo(key, info))
                {
                    if (auto context = p.context.lock())
                    {
                        auto ioSystem = context->getSystem<ReadSystem>();
                        try
                        {
                            const std::string& fileName = request->path.get();
                            //std::cout << "info request: " << request->path.get() << std::endl;
                            std::shared_ptr<IRead> read = ioSystem->read(
                                request->path,
                                request->memoryRead,
                                request->options);
                            if (read)
                            {
                                info = read->getInfo().get();
                            }
                        }
                        catch (const std::exception&)
                        {
                        }
                    }
                }
                request->promise.set_value(info);
                p.cache->addInfo(key, info);
            }
        }

        void ThumbnailGenerator::_thumbnailRun()
        {
            FTK_P();
            std::shared_ptr<Private::ThumbnailRequest> request;
            {
                std::unique_lock<std::mutex> lock(p.thumbnailMutex.mutex);
                if (p.thumbnailThread.cv.wait_for(
                    lock,
                    std::chrono::milliseconds(5),
                    [this]
                    {
                        return !_p->thumbnailMutex.requests.empty();
                    }))
                {
                    request = p.thumbnailMutex.requests.front();
                    p.thumbnailMutex.requests.pop_front();
                }
            }
            if (request)
            {
                std::shared_ptr<ftk::Image> image;
                const std::string key = ThumbnailCache::getThumbnailKey(
                    request->callerId,
                    request->path,
                    request->height,
                    request->time,
                    request->options);
                if (!p.cache->getThumbnail(key, image))
                {
                    if (auto context = p.context.lock())
                    {
                        auto ioSystem = context->getSystem<ReadSystem>();
                        try
                        {
                            const std::string& fileName = request->path.get();
                            //std::cout << "thumbnail request: " << fileName << " " <<
                            //    request->time << std::endl;
                            std::shared_ptr<IRead> read;
                            if (!p.thumbnailThread.ioCache.get(fileName, read))
                            {
                                read = ioSystem->read(
                                    request->path,
                                    request->memoryRead,
                                    request->options);
                                p.thumbnailThread.ioCache.add(fileName, read);
                            }
                            if (read)
                            {
                                const IOInfo info = read->getInfo().get();
                                ftk::Size2I size;
                                if (!info.video.empty())
                                {
                                    size.w = request->height * ftk::aspectRatio(info.video[0].size);
                                    size.h = request->height;
                                }
                                ftk::gl::OffscreenBufferOptions options;
                                options.color = ftk::ImageType::RGBA_U8;
                                if (ftk::gl::doCreate(p.thumbnailThread.buffer, size, options))
                                {
                                    p.thumbnailThread.buffer = ftk::gl::OffscreenBuffer::create(size, options);
                                }
                                const OTIO_NS::RationalTime time =
                                    request->time != invalidTime ?
                                    request->time :
                                    info.videoTime.start_time();
                                const auto videoData = read->readVideo(time, request->options).get();
                                if (p.thumbnailThread.render &&
                                    p.thumbnailThread.buffer &&
                                    videoData.image &&
                                    p.thumbnailThread.running)
                                {
                                    ftk::gl::OffscreenBufferBinding binding(p.thumbnailThread.buffer);
                                    p.thumbnailThread.render->begin(size);
                                    p.thumbnailThread.render->IRender::drawImage(
                                        videoData.image,
                                        ftk::Box2I(0, 0, size.w, size.h));
                                    p.thumbnailThread.render->end();
                                    ftk::ImageInfo info(size.w,
                                        size.h,
                                        ftk::ImageType::RGBA_U8);
                                    image = ftk::Image::create(info);
                                    glPixelStorei(GL_PACK_ALIGNMENT, 1);
                                    glReadPixels(
                                        0,
                                        0,
                                        size.w,
                                        size.h,
                                        GL_RGBA,
                                        GL_UNSIGNED_BYTE,
                                        image->getData());
                                }
                            }
                            else if (
                                ftk::compare(
                                    ".otio",
                                    request->path.getExt(),
                                    ftk::CaseCompare::Insensitive) ||
                                ftk::compare(
                                    ".otioz",
                                    request->path.getExt(),
                                    ftk::CaseCompare::Insensitive))
                            {
                                Options timelineOptions;
                                timelineOptions.ioOptions = request->options;
                                auto timeline = Timeline::create(
                                    context,
                                    request->path,
                                    timelineOptions);
                                const auto info = timeline->getIOInfo();
                                const auto videoData = timeline->getVideo(
                                    timeline->getTimeRange().start_time()).future.get();
                                ftk::Size2I size;
                                if (!info.video.empty())
                                {
                                    size.w = request->height * ftk::aspectRatio(info.video.front().size);
                                    size.h = request->height;
                                }
                                if (size.isValid())
                                {
                                    ftk::gl::OffscreenBufferOptions options;
                                    options.color = ftk::ImageType::RGBA_U8;
                                    if (ftk::gl::doCreate(p.thumbnailThread.buffer, size, options))
                                    {
                                        p.thumbnailThread.buffer = ftk::gl::OffscreenBuffer::create(size, options);
                                    }
                                    if (p.thumbnailThread.render && p.thumbnailThread.buffer)
                                    {
                                        ftk::gl::OffscreenBufferBinding binding(p.thumbnailThread.buffer);
                                        p.thumbnailThread.render->begin(size);
                                        p.thumbnailThread.render->drawVideo(
                                            { videoData },
                                            { ftk::Box2I(0, 0, size.w, size.h) });
                                        p.thumbnailThread.render->end();
                                        ftk::ImageInfo info(size.w,
                                            size.h,
                                            ftk::ImageType::RGBA_U8);
                                        image = ftk::Image::create(info);
                                        glPixelStorei(GL_PACK_ALIGNMENT, 1);
                                        glReadPixels(
                                            0,
                                            0,
                                            size.w,
                                            size.h,
                                            GL_RGBA,
                                            GL_UNSIGNED_BYTE,
                                            image->getData());
                                    }
                                }
                            }
                        }
                        catch (const std::exception&)
                        {}
                    }
                }
                request->promise.set_value(image);
                p.cache->addThumbnail(key, image);
            }
        }

        namespace
        {
            std::shared_ptr<ftk::TriMesh2F> audioMesh(
                const std::shared_ptr<Audio>& audio,
                const ftk::Size2I& size)
            {
                auto out = std::shared_ptr<ftk::TriMesh2F>(new ftk::TriMesh2F);
                const auto& info = audio->getInfo();
                const size_t sampleCount = audio->getSampleCount();
                if (sampleCount > 0)
                {
                    switch (info.type)
                    {
                    case AudioType::F32:
                    {
                        const float* data = reinterpret_cast<const float*>(
                            audio->getData());
                        for (int x = 0; x < size.w; ++x)
                        {
                            const int x0 = std::min(
                                static_cast<size_t>((x + 0) / static_cast<double>(size.w - 1) * (sampleCount - 1)),
                                sampleCount - 1);
                            const int x1 = std::min(
                                static_cast<size_t>((x + 1) / static_cast<double>(size.w - 1) * (sampleCount - 1)),
                                sampleCount - 1);
                            //std::cout << x << ": " << x0 << " " << x1 << std::endl;
                            float min = 0.F;
                            float max = 0.F;
                            if (x0 <= x1)
                            {
                                min = std::numeric_limits<float>::max();
                                max = std::numeric_limits<float>::min();
                                for (int i = x0; i <= x1 && i < sampleCount; ++i)
                                {
                                    const float v = *(data + i * info.channelCount);
                                    min = std::min(min, v);
                                    max = std::max(max, v);
                                }
                            }
                            const int h2 = size.h / 2;
                            const ftk::Box2I box(
                                ftk::V2I(
                                    x,
                                    h2 - h2 * max),
                                ftk::V2I(
                                    x + 1,
                                    h2 - h2 * min));
                            if (box.isValid())
                            {
                                const size_t j = 1 + out->v.size();
                                out->v.push_back(ftk::V2F(box.x(), box.y()));
                                out->v.push_back(ftk::V2F(box.x() + box.w(), box.y()));
                                out->v.push_back(ftk::V2F(box.x() + box.w(), box.y() + box.h()));
                                out->v.push_back(ftk::V2F(box.x(), box.y() + box.h()));
                                out->triangles.push_back(ftk::Triangle2({ j + 0, j + 2, j + 1 }));
                                out->triangles.push_back(ftk::Triangle2({ j + 2, j + 0, j + 3 }));
                            }
                        }
                        break;
                    }
                    default: break;
                    }
                }
                return out;
            }

            std::shared_ptr<ftk::Image> audioImage(
                const std::shared_ptr<Audio>& audio,
                const ftk::Size2I& size)
            {
                auto out = ftk::Image::create(size.w, size.h, ftk::ImageType::L_U8);
                const auto& info = audio->getInfo();
                const size_t sampleCount = audio->getSampleCount();
                if (sampleCount > 0)
                {
                    switch (info.type)
                    {
                    case AudioType::F32:
                    {
                        const float* data = reinterpret_cast<const float*>(
                            audio->getData());
                        for (int x = 0; x < size.w; ++x)
                        {
                            const int x0 = std::min(
                                static_cast<size_t>((x + 0) / static_cast<double>(size.w - 1) * (sampleCount - 1)),
                                sampleCount - 1);
                            const int x1 = std::min(
                                static_cast<size_t>((x + 1) / static_cast<double>(size.w - 1) * (sampleCount - 1)),
                                sampleCount - 1);
                            //std::cout << x << ": " << x0 << " " << x1 << std::endl;
                            float min = 0.F;
                            float max = 0.F;
                            if (x0 < x1)
                            {
                                min = std::numeric_limits<float>::max();
                                max = std::numeric_limits<float>::min();
                                for (int i = x0; i < x1; ++i)
                                {
                                    const float v = *(data + i * info.channelCount);
                                    min = std::min(min, v);
                                    max = std::max(max, v);
                                }
                            }
                            uint8_t* p = out->getData() + x;
                            for (int y = 0; y < size.h; ++y)
                            {
                                const float v = y / static_cast<float>(size.h - 1) * 2.F - 1.F;
                                *p = (v > min && v < max) ? 255 : 0;
                                p += size.w;
                            }
                        }
                        break;
                    }
                    default: break;
                    }
                }
                return out;
            }
        }

        void ThumbnailGenerator::_waveformRun()
        {
            FTK_P();
            std::shared_ptr<Private::WaveformRequest> request;
            {
                std::unique_lock<std::mutex> lock(p.waveformMutex.mutex);
                if (p.waveformThread.cv.wait_for(
                    lock,
                    std::chrono::milliseconds(5),
                    [this]
                    {
                        return !_p->waveformMutex.requests.empty();
                    }))
                {
                    request = p.waveformMutex.requests.front();
                    p.waveformMutex.requests.pop_front();
                }
            }
            if (request)
            {
                std::shared_ptr<ftk::TriMesh2F> mesh;
                const std::string key = ThumbnailCache::getWaveformKey(
                    request->callerId,
                    request->path,
                    request->size,
                    request->timeRange,
                    request->options);
                if (!p.cache->getWaveform(key, mesh))
                {
                    if (auto context = p.context.lock())
                    {
                        try
                        {
                            const std::string& fileName = request->path.get();
                            std::shared_ptr<IRead> read;
                            if (!p.waveformThread.ioCache.get(fileName, read))
                            {
                                auto ioSystem = context->getSystem<ReadSystem>();
                                read = ioSystem->read(
                                    request->path,
                                    request->memoryRead,
                                    request->options);
                                p.waveformThread.ioCache.add(fileName, read);
                            }
                            if (read)
                            {
                                const auto info = read->getInfo().get();
                                const OTIO_NS::TimeRange timeRange =
                                    request->timeRange != invalidTimeRange ?
                                    request->timeRange :
                                    OTIO_NS::TimeRange(
                                        OTIO_NS::RationalTime(0.0, 1.0),
                                        OTIO_NS::RationalTime(1.0, 1.0));
                                const auto audioData = read->readAudio(timeRange, request->options).get();
                                if (audioData.audio && p.waveformThread.running)
                                {
                                    auto resample = AudioResample::create(
                                        audioData.audio->getInfo(),
                                        AudioInfo(1, AudioType::F32, audioData.audio->getSampleRate()));
                                    const auto resampledAudio = resample->process(audioData.audio);
                                    mesh = audioMesh(resampledAudio, request->size);
                                }
                            }
                        }
                        catch (const std::exception&)
                        {}
                    }
                }
                request->promise.set_value(mesh);
                p.cache->addWaveform(key, mesh);
            }
        }

        void ThumbnailGenerator::_infoCancel()
        {
            FTK_P();
            std::list<std::shared_ptr<Private::InfoRequest> > requests;
            {
                std::unique_lock<std::mutex> lock(p.infoMutex.mutex);
                requests = std::move(p.infoMutex.requests);
            }
            for (auto& request : requests)
            {
                request->promise.set_value(IOInfo());
            }
        }

        void ThumbnailGenerator::_thumbnailCancel()
        {
            FTK_P();
            std::list<std::shared_ptr<Private::ThumbnailRequest> > requests;
            {
                std::unique_lock<std::mutex> lock(p.thumbnailMutex.mutex);
                requests = std::move(p.thumbnailMutex.requests);
            }
            for (auto& request : requests)
            {
                request->promise.set_value(nullptr);
            }
        }

        void ThumbnailGenerator::_waveformCancel()
        {
            FTK_P();
            std::list<std::shared_ptr<Private::WaveformRequest> > requests;
            {
                std::unique_lock<std::mutex> lock(p.waveformMutex.mutex);
                requests = std::move(p.waveformMutex.requests);
            }
            for (auto& request : requests)
            {
                request->promise.set_value(nullptr);
            }
        }

        struct ThumbnailSystem::Private
        {
            std::shared_ptr<ThumbnailCache> cache;
            std::shared_ptr<ThumbnailGenerator> generator;
        };

        ThumbnailSystem::ThumbnailSystem(const std::shared_ptr<ftk::Context>& context) :
            ISystem(context, "tl::ui::ThumbnailSystem"),
            _p(new Private)
        {
            FTK_P();
            p.cache = ThumbnailCache::create(context);
            p.generator = ThumbnailGenerator::create(p.cache, context);
        }

        ThumbnailSystem::~ThumbnailSystem()
        {}

        std::shared_ptr<ThumbnailSystem> ThumbnailSystem::create(
            const std::shared_ptr<ftk::Context>& context)
        {
            auto out = context->getSystem<ThumbnailSystem>();
            if (!out)
            {
                out = std::shared_ptr<ThumbnailSystem>(new ThumbnailSystem(context));
                context->addSystem(out);
            }
            return out;
        }

        InfoRequest ThumbnailSystem::getInfo(
            intptr_t id,
            const ftk::Path& path,
            const IOOptions& ioOptions)
        {
            return _p->generator->getInfo(id, path, ioOptions);
        }

        ThumbnailRequest ThumbnailSystem::getThumbnail(
            intptr_t id,
            const ftk::Path& path,
            int height,
            const OTIO_NS::RationalTime& time,
            const IOOptions& ioOptions)
        {
            return _p->generator->getThumbnail(id, path, height, time, ioOptions);
        }

        WaveformRequest ThumbnailSystem::getWaveform(
            intptr_t id,
            const ftk::Path& path,
            const ftk::Size2I& size,
            const OTIO_NS::TimeRange& timeRange,
            const IOOptions& ioOptions)
        {
            return _p->generator->getWaveform(id, path, size, timeRange, ioOptions);
        }

        void ThumbnailSystem::cancelRequests(const std::vector<uint64_t>& ids)
        {
            _p->generator->cancelRequests(ids);
        }
        
        const std::shared_ptr<ThumbnailCache>& ThumbnailSystem::getCache() const
        {
            return _p->cache;
        }
    }
}
