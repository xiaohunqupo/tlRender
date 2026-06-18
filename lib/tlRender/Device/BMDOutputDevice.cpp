// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/Device/BMDOutputPrivate.h>

#include <tlRender/Device/BMDSystem.h>
#include <tlRender/Device/BMDUtil.h>

#include <tlRender/GL/Render.h>

#include <ftk/GL/OffscreenBuffer.h>
#include <ftk/GL/Texture.h>
#include <ftk/GL/Util.h>
#include <ftk/GL/Window.h>

#include <tlRender/Core/AudioResample.h>

#include <ftk/Core/Context.h>
#include <ftk/Core/Format.h>

#include <algorithm>
#include <atomic>
#include <iostream>
#include <list>
#include <mutex>
#include <tuple>

#if defined(_WINDOWS)
#include <atlbase.h>
#endif // _WINDOWS

namespace tl
{
    namespace bmd
    {
        namespace
        {
            const int videoFrameDelay = 3;
            const std::chrono::milliseconds timeout(5);

            //! Flags for which inputs have changed since the last update.
            enum class Update : uint32_t
            {
                None        = 0,
                Config      = 1 << 0,
                Enabled     = 1 << 1,
                FrameDelay  = 1 << 2,
                OCIO        = 1 << 3,
                LUT         = 1 << 4,
                Image       = 1 << 5,
                Display     = 1 << 6,
                HDR         = 1 << 7,
                Compare     = 1 << 8,
                Background  = 1 << 9,
                Foreground  = 1 << 10,
                View        = 1 << 11,
                TimeRange   = 1 << 12,
                Playback    = 1 << 13,
                Seek        = 1 << 14,
                Video       = 1 << 15,
                Overlay     = 1 << 16,
                Audio       = 1 << 17,
                AudioCtl    = 1 << 18
            };

            constexpr Update operator | (Update a, Update b)
            {
                return static_cast<Update>(
                    static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
            }
            constexpr Update operator & (Update a, Update b)
            {
                return static_cast<Update>(
                    static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
            }
            inline Update& operator |= (Update& a, Update b)
            {
                a = a | b;
                return a;
            }
            constexpr bool any(Update a)
            {
                return a != Update::None;
            }

            //! Inputs that require re-creating the output device.
            constexpr Update deviceMask =
                Update::Config | Update::Enabled | Update::FrameDelay;

            //! Inputs that require re-rendering the video.
            constexpr Update renderMask =
                Update::OCIO | Update::LUT | Update::Image | Update::Display |
                Update::HDR | Update::Compare | Update::Background |
                Update::Foreground | Update::View | Update::Video |
                Update::Overlay;

            //! The output device input state: a single snapshot of everything
            //! the public setters and the player observers write.
            struct State
            {
                DeviceConfig config;
                bool enabled = false;
                int videoFrameDelay = bmd::videoFrameDelay;
                OCIOOptions ocioOptions;
                LUTOptions lutOptions;
                std::vector<ftk::ImageOptions> imageOptions;
                std::vector<DisplayOptions> displayOptions;
                std::vector<AspectRatioOptions> aspectRatioOptions;
                HDRMode hdrMode = HDRMode::FromFile;
                HDRData hdrData;
                CompareOptions compareOptions;
                BackgroundOptions bgOptions;
                ForegroundOptions fgOptions;
                ftk::V2I viewPos;
                double viewZoom = 1.0;
                bool frameView = true;
                OTIO_NS::TimeRange timeRange = invalidTimeRange;
                Playback playback = Playback::Stop;
                double speed = 0.0;
                OTIO_NS::RationalTime currentTime = invalidTime;
                std::vector<VideoFrame> videoFrames;
                std::shared_ptr<ftk::Image> overlay;
                float volume = 1.F;
                bool mute = false;
                std::vector<bool> channelMute;
                double audioOffset = 0.0;
                std::vector<AudioFrame> audioFrames;
            };
        }

        bool FrameRate::operator == (const FrameRate& other) const
        {
            return num == other.num && den == other.den;
        }

        bool FrameRate::operator != (const FrameRate& other) const
        {
            return !(*this == other);
        }

        struct OutputDevice::Private
        {
            std::weak_ptr<ftk::Context> context;

            std::shared_ptr<ftk::Observable<DeviceConfig> > config;
            std::shared_ptr<ftk::Observable<bool> > enabled;
            std::shared_ptr<ftk::Observable<bool> > active;
            std::shared_ptr<ftk::Observable<ftk::Size2I> > size;
            std::shared_ptr<ftk::Observable<FrameRate> > frameRate;
            std::shared_ptr<ftk::Observable<int> > videoFrameDelay;

            std::shared_ptr<Player> player;
            std::shared_ptr<ftk::Observer<Playback> > playbackObserver;
            std::shared_ptr<ftk::Observer<double> > speedObserver;
            std::shared_ptr<ftk::Observer<OTIO_NS::RationalTime> > currentTimeObserver;
            std::shared_ptr<ftk::Observer<OTIO_NS::RationalTime> > seekObserver;
            std::shared_ptr<ftk::ListObserver<VideoFrame> > videoObserver;
            std::shared_ptr<ftk::ListObserver<AudioFrame> > audioObserver;

            std::shared_ptr<ftk::gl::Window> window;

            struct Mutex
            {
                State state;
                Update pending = Update::None;

                // Outputs written by the thread and read by _tick().
                bool active = false;
                ftk::Size2I size;
                FrameRate frameRate;

                std::mutex mutex;
            };
            Mutex mutex;

            struct Thread
            {
                std::unique_ptr<DLWrapper> dl;

                State state;
                ftk::Size2I size;
                PixelType outputPixelType = PixelType::None;

                std::shared_ptr<IRender> render;
                std::shared_ptr<ftk::gl::OffscreenBuffer> offscreenBuffer;
                GLuint pbo = 0;

                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            Thread thread;
        };

        void OutputDevice::_init(const std::shared_ptr<ftk::Context>& context)
        {
            FTK_P();

            if (auto system = context->getSystem<System>())
            {
                system->_addDevice(shared_from_this());
            }

            p.context = context;

            p.config = ftk::Observable<DeviceConfig>::create();
            p.enabled = ftk::Observable<bool>::create(false);
            p.active = ftk::Observable<bool>::create(false);
            p.size = ftk::Observable<ftk::Size2I>::create();
            p.frameRate = ftk::Observable<FrameRate>::create();
            p.videoFrameDelay = ftk::Observable<int>::create(bmd::videoFrameDelay);

            p.window = ftk::gl::Window::create(
                context,
                "tl::bmd::OutputDevice",
                ftk::Size2I(1, 1),
                static_cast<int>(ftk::gl::WindowOptions::None));
            p.thread.running = true;
            p.thread.thread = std::thread(
                [this]
                {
                    FTK_P();
#if defined(_WINDOWS)
                    CoInitialize(NULL);
#endif // _WINDOWS
                    p.window->makeCurrent();
                    _run();
                    p.window->clearCurrent();
#if defined(_WINDOWS)
                    CoUninitialize();
#endif // _WINDOWS
                });
        }

        OutputDevice::OutputDevice() :
            _p(new Private)
        {}

        OutputDevice::~OutputDevice()
        {
            FTK_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
        }

        std::shared_ptr<OutputDevice> OutputDevice::create(
            const std::shared_ptr<ftk::Context>& context)
        {
            auto out = std::shared_ptr<OutputDevice>(new OutputDevice);
            out->_init(context);
            return out;
        }

        DeviceConfig OutputDevice::getConfig() const
        {
            return _p->config->get();
        }

        std::shared_ptr<ftk::IObservable<DeviceConfig> > OutputDevice::observeConfig() const
        {
            return _p->config;
        }

        void OutputDevice::setConfig(const DeviceConfig& value)
        {
            FTK_P();
            if (p.config->setIfChanged(value))
            {
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    p.mutex.state.config = value;
                    p.mutex.pending |= Update::Config;
                }
                p.thread.cv.notify_one();
            }
        }

        bool OutputDevice::isEnabled() const
        {
            return _p->enabled->get();
        }

        std::shared_ptr<ftk::IObservable<bool> > OutputDevice::observeEnabled() const
        {
            return _p->enabled;
        }

        void OutputDevice::setEnabled(bool value)
        {
            FTK_P();
            if (p.enabled->setIfChanged(value))
            {
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    p.mutex.state.enabled = value;
                    p.mutex.pending |= Update::Enabled;
                }
                p.thread.cv.notify_one();
            }
        }

        bool OutputDevice::isActive() const
        {
            return _p->active->get();
        }

        std::shared_ptr<ftk::IObservable<bool> > OutputDevice::observeActive() const
        {
            return _p->active;
        }

        const ftk::Size2I& OutputDevice::getSize() const
        {
            return _p->size->get();
        }

        std::shared_ptr<ftk::IObservable<ftk::Size2I> > OutputDevice::observeSize() const
        {
            return _p->size;
        }

        const FrameRate& OutputDevice::getFrameRate() const
        {
            return _p->frameRate->get();
        }

        std::shared_ptr<ftk::IObservable<FrameRate> > OutputDevice::observeFrameRate() const
        {
            return _p->frameRate;
        }

        int OutputDevice::getVideoFrameDelay() const
        {
            return _p->videoFrameDelay->get();
        }

        std::shared_ptr<ftk::IObservable<int> > OutputDevice::observeVideoFrameDelay() const
        {
            return _p->videoFrameDelay;
        }

        void OutputDevice::setVideoFrameDelay(int value)
        {
            FTK_P();
            // The pre-roll loop in DLOutputCallback schedules this many frames
            // before starting playback; a value below 1 leaves nothing
            // pre-rolled and underruns immediately.
            const int delay = std::max(value, 1);
            if (p.videoFrameDelay->setIfChanged(delay))
            {
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    p.mutex.state.videoFrameDelay = delay;
                    p.mutex.pending |= Update::FrameDelay;
                }
                p.thread.cv.notify_one();
            }
        }

        void OutputDevice::setView(
            const ftk::V2I& position,
            double          zoom,
            bool            frame)
        {
            FTK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.state.viewPos = position;
                p.mutex.state.viewZoom = zoom;
                p.mutex.state.frameView = frame;
                p.mutex.pending |= Update::View;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setOCIOOptions(const OCIOOptions& value)
        {
            FTK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.state.ocioOptions = value;
                p.mutex.pending |= Update::OCIO;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setLUTOptions(const LUTOptions& value)
        {
            FTK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.state.lutOptions = value;
                p.mutex.pending |= Update::LUT;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setImageOptions(const std::vector<ftk::ImageOptions>& value)
        {
            FTK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.state.imageOptions = value;
                p.mutex.pending |= Update::Image;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setDisplayOptions(const std::vector<DisplayOptions>& value)
        {
            FTK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.state.displayOptions = value;
                p.mutex.pending |= Update::Display;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setHDR(HDRMode hdrMode, const HDRData& hdrData)
        {
            FTK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.state.hdrMode = hdrMode;
                p.mutex.state.hdrData = hdrData;
                p.mutex.pending |= Update::HDR;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setCompareOptions(const CompareOptions& value)
        {
            FTK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.state.compareOptions = value;
                p.mutex.pending |= Update::Compare;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setBackgroundOptions(const BackgroundOptions& value)
        {
            FTK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.state.bgOptions = value;
                p.mutex.pending |= Update::Background;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setForegroundOptions(const ForegroundOptions& value)
        {
            FTK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.state.fgOptions = value;
                p.mutex.pending |= Update::Foreground;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setOverlay(const std::shared_ptr<ftk::Image>& value)
        {
            FTK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.state.overlay = value;
                p.mutex.pending |= Update::Overlay;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setVolume(float value)
        {
            FTK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.state.volume = value;
                p.mutex.pending |= Update::AudioCtl;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setMute(bool value)
        {
            FTK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.state.mute = value;
                p.mutex.pending |= Update::AudioCtl;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setChannelMute(const std::vector<bool>& value)
        {
            FTK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.state.channelMute = value;
                p.mutex.pending |= Update::AudioCtl;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setAudioOffset(double value)
        {
            FTK_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.state.audioOffset = value;
                p.mutex.pending |= Update::AudioCtl;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::setPlayer(const std::shared_ptr<Player>& value)
        {
            FTK_P();
            if (value == p.player)
                return;

            p.playbackObserver.reset();
            p.speedObserver.reset();
            p.currentTimeObserver.reset();
            p.seekObserver.reset();
            p.videoObserver.reset();
            p.audioObserver.reset();

            p.player = value;

            if (p.player)
            {
                auto weak = std::weak_ptr<OutputDevice>(shared_from_this());
                p.playbackObserver = ftk::Observer<Playback>::create(
                    p.player->observePlayback(),
                    [weak](Playback value)
                    {
                        if (auto device = weak.lock())
                        {
                            {
                                std::unique_lock<std::mutex> lock(device->_p->mutex.mutex);
                                device->_p->mutex.state.playback = value;
                                device->_p->mutex.pending |= Update::Playback;
                            }
                            device->_p->thread.cv.notify_one();
                        }
                    },
                    ftk::ObserverAction::Suppress);
                p.speedObserver = ftk::Observer<double>::create(
                    p.player->observeSpeed(),
                    [weak](double value)
                    {
                        if (auto device = weak.lock())
                        {
                            {
                                std::unique_lock<std::mutex> lock(device->_p->mutex.mutex);
                                device->_p->mutex.state.speed = value;
                                device->_p->mutex.pending |= Update::Playback;
                            }
                            device->_p->thread.cv.notify_one();
                        }
                    },
                    ftk::ObserverAction::Suppress);
                p.currentTimeObserver = ftk::Observer<OTIO_NS::RationalTime>::create(
                    p.player->observeCurrentTime(),
                    [weak](const OTIO_NS::RationalTime& value)
                    {
                        if (auto device = weak.lock())
                        {
                            {
                                std::unique_lock<std::mutex> lock(device->_p->mutex.mutex);
                                device->_p->mutex.state.currentTime = value;
                                device->_p->mutex.pending |= Update::Playback;
                            }
                            device->_p->thread.cv.notify_one();
                        }
                    },
                    ftk::ObserverAction::Suppress);
                p.seekObserver = ftk::Observer<OTIO_NS::RationalTime>::create(
                    p.player->observeSeek(),
                    [weak](const OTIO_NS::RationalTime&)
                    {
                        if (auto device = weak.lock())
                        {
                            {
                                std::unique_lock<std::mutex> lock(device->_p->mutex.mutex);
                                device->_p->mutex.pending |= Update::Seek;
                            }
                            device->_p->thread.cv.notify_one();
                        }
                    },
                    ftk::ObserverAction::Suppress);
                p.videoObserver = ftk::ListObserver<VideoFrame>::create(
                    p.player->observeCurrentVideo(),
                    [weak](const std::vector<VideoFrame>& value)
                    {
                        if (auto device = weak.lock())
                        {
                            {
                                std::unique_lock<std::mutex> lock(device->_p->mutex.mutex);
                                device->_p->mutex.state.videoFrames = value;
                                device->_p->mutex.pending |= Update::Video;
                            }
                            device->_p->thread.cv.notify_one();
                        }
                    },
                    ftk::ObserverAction::Suppress);
                p.audioObserver = ftk::ListObserver<AudioFrame>::create(
                    p.player->observeCurrentAudio(),
                    [weak](const std::vector<AudioFrame>& value)
                    {
                        if (auto device = weak.lock())
                        {
                            {
                                std::unique_lock<std::mutex> lock(device->_p->mutex.mutex);
                                device->_p->mutex.state.audioFrames = value;
                                device->_p->mutex.pending |= Update::Audio;
                            }
                            device->_p->thread.cv.notify_one();
                        }
                    },
                    ftk::ObserverAction::Suppress);
            }

            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (p.player)
                {
                    p.mutex.state.timeRange = p.player->getTimeRange();
                    p.mutex.state.playback = p.player->getPlayback();
                    p.mutex.state.speed = p.player->getSpeed();
                    p.mutex.state.currentTime = p.player->getCurrentTime();
                }
                else
                {
                    p.mutex.state.timeRange = invalidTimeRange;
                    p.mutex.state.playback = Playback::Stop;
                    p.mutex.state.speed = 0.0;
                    p.mutex.state.currentTime = invalidTime;
                }
                p.mutex.state.videoFrames.clear();
                p.mutex.state.audioFrames.clear();
                if (p.player)
                {
                    p.mutex.state.videoFrames = p.player->getCurrentVideo();
                    p.mutex.state.audioFrames = p.player->getCurrentAudio();
                }
                p.mutex.pending |=
                    Update::TimeRange | Update::Playback |
                    Update::Video | Update::Audio;
            }
            p.thread.cv.notify_one();
        }

        void OutputDevice::_tick()
        {
            FTK_P();
            bool active = false;
            ftk::Size2I size = p.size->get();
            FrameRate frameRate = p.frameRate->get();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                active = p.mutex.active;
                size = p.mutex.size;
                frameRate = p.mutex.frameRate;
            }
            p.active->setIfChanged(active);
            p.size->setIfChanged(size);
            p.frameRate->setIfChanged(frameRate);
        }

        void OutputDevice::_run()
        {
            FTK_P();

            p.thread.render = gl::Render::create(
                p.context.lock()->getLogSystem(),
                p.context.lock()->getSystem<ftk::FontSystem>());

            auto t = std::chrono::steady_clock::now();
            while (p.thread.running)
            {
                bool createDevice = false;
                bool doRender = false;
                bool audioChanged = false;
                bool seek = false;
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    if (p.thread.cv.wait_for(
                        lock,
                        timeout,
                        [this] { return any(_p->mutex.pending); }))
                    {
                        const Update u = p.mutex.pending;
                        p.mutex.pending = Update::None;

                        // Snapshot every input in one assignment, then let the
                        // change flags decide what work to do this pass.
                        p.thread.state = p.mutex.state;

                        createDevice = any(u & deviceMask);
                        doRender = createDevice || any(u & renderMask);
                        audioChanged = createDevice || any(u & Update::Audio);
                        seek = any(u & Update::Seek);
                    }
                }

                if (createDevice)
                {
                    if (p.thread.pbo != 0)
                    {
                        glDeleteBuffers(1, &p.thread.pbo);
                        p.thread.pbo = 0;
                    }
                    p.thread.offscreenBuffer.reset();
                    p.thread.dl.reset();
                    p.thread.size = ftk::Size2I();
                    p.thread.outputPixelType = PixelType::None;

                    bool active = false;
                    FrameRate frameRate;
                    if (p.thread.state.enabled)
                    {
                        try
                        {
                            p.thread.dl.reset(new DLWrapper);
                            _createDevice(
                                p.thread.state.config,
                                active,
                                frameRate,
                                p.thread.state.videoFrameDelay);
                        }
                        catch (const std::exception& e)
                        {
                            p.context.lock()->log(
                                "tl::bmd::OutputDevice",
                                e.what(),
                                ftk::LogType::Error);
                        }
                    }
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        p.mutex.active = active;
                        p.mutex.size = p.thread.size;
                        p.mutex.frameRate = frameRate;
                    }

                    if (active)
                    {
                        glGenBuffers(1, &p.thread.pbo);
                        glBindBuffer(GL_PIXEL_PACK_BUFFER, p.thread.pbo);
                        glBufferData(
                            GL_PIXEL_PACK_BUFFER,
                            getPackPixelsSize(p.thread.size, p.thread.outputPixelType),
                            NULL,
                            GL_STREAM_READ);
                    }
                }

                if (doRender && p.thread.render &&
                    p.thread.dl && p.thread.dl->output)
                {
                    try
                    {
                        _render(
                            p.thread.state.config,
                            p.thread.state.ocioOptions,
                            p.thread.state.lutOptions,
                            p.thread.state.imageOptions,
                            p.thread.state.displayOptions,
                            p.thread.state.compareOptions,
                            p.thread.state.bgOptions,
                            p.thread.state.fgOptions);
                    }
                    catch (const std::exception& e)
                    {
                        p.context.lock()->log(
                            "tl::bmd::OutputDevice",
                            e.what(),
                            ftk::LogType::Error);
                    }
                }

                if (p.thread.dl && p.thread.dl->outputCallback)
                {
                    DLOutputCallbackData data;
                    data.playback = p.thread.state.playback;
                    data.speed = p.thread.state.speed;
                    data.currentTime = p.thread.state.currentTime;
                    data.seek = seek;
                    data.volume = p.thread.state.volume;
                    data.mute = p.thread.state.mute;
                    data.channelMute = p.thread.state.channelMute;
                    data.audioOffset = p.thread.state.audioOffset;
                    p.thread.dl->outputCallback->setData(data);
                }
                if (p.thread.dl && p.thread.dl->outputCallback && audioChanged)
                {
                    p.thread.dl->outputCallback->setAudio(p.thread.state.audioFrames);
                }

                if (p.thread.dl && p.thread.dl->output && p.thread.dl->outputCallback &&
                    doRender && p.thread.render)
                {
                    _read();
                }

                const auto t1 = std::chrono::steady_clock::now();
                const std::chrono::duration<double> diff = t1 - t;
                //std::cout << "diff: " << diff.count() * 1000 << std::endl;
                t = t1;
            }

            if (p.thread.pbo != 0)
            {
                glDeleteBuffers(1, &p.thread.pbo);
                p.thread.pbo = 0;
            }
            p.thread.offscreenBuffer.reset();
            p.thread.render.reset();
            p.thread.dl.reset();
        }

        void OutputDevice::_createDevice(
            const DeviceConfig& config,
            bool& active,
            FrameRate& frameRate,
            int videoFrameDelay)
        {
            FTK_P();
            if (config.deviceIndex != -1 &&
                config.displayModeIndex != -1 &&
                config.pixelType != PixelType::None)
            {
                std::string modelName;
                {
                    DLIteratorWrapper dlIterator;
                    if (GetDeckLinkIterator(&dlIterator.p) != S_OK)
                    {
                        throw std::runtime_error("Cannot get iterator");
                    }

                    int count = 0;
                    while (dlIterator->Next(&p.thread.dl->p) == S_OK)
                    {
                        if (count == config.deviceIndex)
                        {
#if defined(__APPLE__)
                            CFStringRef dlModelName;
                            p.thread.dl->p->GetModelName(&dlModelName);
                            StringToStdString(dlModelName, modelName);
                            CFRelease(dlModelName);
#else // __APPLE__
                            dlstring_t dlModelName;
                            p.thread.dl->p->GetModelName(&dlModelName);
                            modelName = DlToStdString(dlModelName);
                            DeleteString(dlModelName);
#endif // __APPLE__
                            break;
                        }

                        p.thread.dl->p->Release();
                        p.thread.dl->p = nullptr;

                        ++count;
                    }
                    if (!p.thread.dl->p)
                    {
                        throw std::runtime_error("Device not found");
                    }
                }

                if (p.thread.dl->p->QueryInterface(IID_IDeckLinkConfiguration, (void**)&p.thread.dl->config) != S_OK)
                {
                    throw std::runtime_error("Cannot get configuration");
                }
                for (const auto& option : config.boolOptions)
                {
                    switch (option.first)
                    {
                    case Option::_444SDIVideoOutput:
                        p.thread.dl->config->SetFlag(bmdDeckLinkConfig444SDIVideoOutput, option.second);
                        break;
                    default: break;
                    }
                }
                BOOL value = 0;
                p.thread.dl->config->GetFlag(bmdDeckLinkConfig444SDIVideoOutput, &value);
                p.context.lock()->log(
                    "tl::bmd::OutputDevice",
                    ftk::Format("444 SDI output: {0}").arg(value));

                if (p.thread.dl->p->QueryInterface(IID_IDeckLinkStatus, (void**)&p.thread.dl->status) != S_OK)
                {
                    throw std::runtime_error("Cannot get status");
                }

                if (p.thread.dl->p->QueryInterface(IID_IDeckLinkOutput, (void**)&p.thread.dl->output) != S_OK)
                {
                    throw std::runtime_error("Cannot get output");
                }

                const AudioInfo audioInfo(2, AudioType::S16, 48000);
                {
                    DLDisplayModeIteratorWrapper dlDisplayModeIterator;
                    if (p.thread.dl->output->GetDisplayModeIterator(&dlDisplayModeIterator.p) != S_OK)
                    {
                        throw std::runtime_error("Cannot get display mode iterator");
                    }
                    DLDisplayModeWrapper dlDisplayMode;
                    int count = 0;
                    while (dlDisplayModeIterator->Next(&dlDisplayMode.p) == S_OK)
                    {
                        if (count == config.displayModeIndex)
                        {
                            break;
                        }

                        dlDisplayMode->Release();
                        dlDisplayMode.p = nullptr;

                        ++count;
                    }
                    if (!dlDisplayMode.p)
                    {
                        throw std::runtime_error("Display mode not found");
                    }

                    p.thread.size.w = dlDisplayMode->GetWidth();
                    p.thread.size.h = dlDisplayMode->GetHeight();
                    p.thread.outputPixelType = getOutputType(config.pixelType);
                    BMDTimeValue frameDuration;
                    BMDTimeScale frameTimescale;
                    dlDisplayMode->GetFrameRate(&frameDuration, &frameTimescale);
                    // Note: BMD's GetFrameRate() returns the frame duration and
                    // timescale, where the actual frame rate is timescale /
                    // duration (e.g. 1001 and 30000 for 29.97). The values are
                    // stored here as num = duration, den = timescale, which is
                    // NOT a frame-rate fraction. They are used consistently with
                    // ScheduleVideoFrame() and StartScheduledPlayback() below;
                    // do not treat num/den as a frame rate.
                    frameRate.num = static_cast<int>(frameDuration);
                    frameRate.den = static_cast<int>(frameTimescale);

                    p.context.lock()->log(
                        "tl::bmd::OutputDevice",
                        ftk::Format(
                            "\n"
                            "    #{0} {1}/{2}\n"
                            "    video: {3} {4}\n"
                            "    audio: {5} {6} {7}").
                        arg(config.deviceIndex).
                        arg(modelName).
                        arg(p.thread.size).
                        arg(frameRate.num).
                        arg(frameRate.den).
                        arg(audioInfo.channelCount).
                        arg(audioInfo.type).
                        arg(audioInfo.sampleRate));

                    HRESULT r = p.thread.dl->output->EnableVideoOutput(
                        dlDisplayMode->GetDisplayMode(),
                        bmdVideoOutputFlagDefault);
                    switch (r)
                    {
                    case S_OK:
                        break;
                    case E_ACCESSDENIED:
                        throw std::runtime_error("Unable to access the hardware");
                    default:
                        throw std::runtime_error("Cannot enable video output");
                    }

                    r = p.thread.dl->output->EnableAudioOutput(
                        bmdAudioSampleRate48kHz,
                        bmdAudioSampleType16bitInteger,
                        audioInfo.channelCount,
                        bmdAudioOutputStreamContinuous);
                    switch (r)
                    {
                    case S_OK:
                        break;
                    case E_INVALIDARG:
                        throw std::runtime_error("Invalid number of channels requested");
                    case E_ACCESSDENIED:
                        throw std::runtime_error("Unable to access the hardware");
                    default:
                        throw std::runtime_error("Cannot enable audio output");
                    }
                }

                p.thread.dl->outputCallback = new DLOutputCallback(
                    p.thread.dl->output,
                    p.thread.size,
                    config.pixelType,
                    frameRate,
                    videoFrameDelay,
                    audioInfo);

                if (p.thread.dl->output->SetScheduledFrameCompletionCallback(p.thread.dl->outputCallback) != S_OK)
                {
                    throw std::runtime_error("Cannot set video callback");
                }

                if (p.thread.dl->output->SetAudioCallback(p.thread.dl->outputCallback) != S_OK)
                {
                    throw std::runtime_error("Cannot set audio callback");
                }

                active = true;
            }
        }

        void OutputDevice::_render(
            const DeviceConfig& config,
            const OCIOOptions& ocioOptions,
            const LUTOptions& lutOptions,
            const std::vector<ftk::ImageOptions>& imageOptions,
            const std::vector<DisplayOptions>& displayOptions,
            const CompareOptions& compareOptions,
            const BackgroundOptions& bgOptions,
            const ForegroundOptions& fgOptions)
        {
            FTK_P();

            // Create the offscreen buffer.
            AspectRatioOptions aspectRatioOptions;
            if (!displayOptions.empty())
            {
                aspectRatioOptions = displayOptions[0].aspectRatio;
            }
            const ftk::Size2I renderSize = getRenderSize(
                compareOptions.compare,
                aspectRatioOptions,
                p.thread.state.videoFrames);
            ftk::gl::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.depth = ftk::gl::OffscreenDepth::_24;
            offscreenBufferOptions.stencil = ftk::gl::OffscreenStencil::_8;
            if (!displayOptions.empty())
            {
                offscreenBufferOptions.colorFilters = displayOptions[0].imageFilters;
            }
            if (ftk::gl::doCreate(
                p.thread.offscreenBuffer,
                p.thread.size,
                getColorBuffer(p.thread.outputPixelType),
                offscreenBufferOptions))
            {
                p.thread.offscreenBuffer = ftk::gl::OffscreenBuffer::create(
                    p.thread.size,
                    getColorBuffer(p.thread.outputPixelType),
                    offscreenBufferOptions);
            }

            // Render the video.
            if (p.thread.offscreenBuffer)
            {
                ftk::gl::OffscreenBufferBinding binding(p.thread.offscreenBuffer);

                p.thread.render->begin(p.thread.size);
                p.thread.render->setOCIOOptions(ocioOptions);
                p.thread.render->setLUTOptions(lutOptions);

                const auto pm = ftk::ortho(
                    0.F,
                    static_cast<float>(p.thread.size.w),
                    static_cast<float>(p.thread.size.h),
                    0.F,
                    -1.F,
                    1.F);
                p.thread.render->setTransform(pm);

                const auto boxes = getBoxes(
                    compareOptions.compare,
                    aspectRatioOptions,
                    p.thread.state.videoFrames);
                ftk::V2I viewPosTmp = p.thread.state.viewPos;
                double viewZoomTmp = p.thread.state.viewZoom;
                if (p.thread.state.frameView)
                {
                    double zoom = p.thread.size.w / static_cast<double>(renderSize.w);
                    if (zoom * renderSize.h > p.thread.size.h)
                    {
                        zoom = p.thread.size.h / static_cast<double>(renderSize.h);
                    }
                    const ftk::V2I c(renderSize.w / 2, renderSize.h / 2);
                    viewPosTmp.x = p.thread.size.w / 2.0 - c.x * zoom;
                    viewPosTmp.y = p.thread.size.h / 2.0 - c.y * zoom;
                    viewZoomTmp = zoom;
                }
                ftk::M44F vm;
                vm = vm * ftk::translate(ftk::V3F(viewPosTmp.x, viewPosTmp.y, 0.F));
                vm = vm * ftk::scale(ftk::V3F(viewZoomTmp, viewZoomTmp, 1.F));
                p.thread.render->drawBackground(boxes, vm, bgOptions);

                p.thread.render->setTransform(pm * vm);
                p.thread.render->drawVideo(
                    p.thread.state.videoFrames,
                    boxes,
                    imageOptions,
                    displayOptions,
                    compareOptions,
                    getColorBuffer(p.thread.outputPixelType));

                p.thread.render->setTransform(pm);
                p.thread.render->drawForeground(boxes, vm, fgOptions);

                if (p.thread.state.overlay)
                {
                    ftk::ImageOptions imageOptions;
                    imageOptions.alphaBlend = ftk::AlphaBlend::Premultiplied;
                    imageOptions.cache = false;
                    p.thread.render->drawImage(
                        p.thread.state.overlay,
                        ftk::Box2I(
                            0,
                            0,
                            p.thread.state.overlay->getWidth(),
                            p.thread.state.overlay->getHeight()),
                        ftk::Color4F(1.F, 1.F, 1.F),
                        imageOptions);
                }

                p.thread.render->end();

                glBindBuffer(GL_PIXEL_PACK_BUFFER, p.thread.pbo);
                glPixelStorei(GL_PACK_ALIGNMENT, getPackPixelsAlign(p.thread.outputPixelType));
                glPixelStorei(GL_PACK_SWAP_BYTES, getPackPixelsSwap(p.thread.outputPixelType));
                glBindTexture(GL_TEXTURE_2D, p.thread.offscreenBuffer->getColorID());
                glGetTexImage(
                    GL_TEXTURE_2D,
                    0,
                    getPackPixelsFormat(p.thread.outputPixelType),
                    getPackPixelsType(p.thread.outputPixelType),
                    NULL);
            }
        }

        void OutputDevice::_read()
        {
            FTK_P();

            auto dlVideoFrame = std::make_shared<DLVideoFrameWrapper>();
            if (p.thread.dl->output->CreateVideoFrame(
                p.thread.size.w,
                p.thread.size.h,
                getRowByteCount(p.thread.size.w, p.thread.outputPixelType),
                toBMD(p.thread.outputPixelType),
                bmdFrameFlagFlipVertical,
                &dlVideoFrame->p) != S_OK)
            {
                throw std::runtime_error("Cannot create video frame");
            }

            void* dlVideoFrameP = nullptr;
            dlVideoFrame->p->GetBytes((void**)&dlVideoFrameP);
            glBindBuffer(GL_PIXEL_PACK_BUFFER, p.thread.pbo);
            if (void* pboP = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY))
            {
                copyPackPixels(pboP, dlVideoFrameP, p.thread.size, p.thread.outputPixelType);
                glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            }

            p.thread.dl->outputCallback->setVideo(dlVideoFrame);
        }
    }
}
