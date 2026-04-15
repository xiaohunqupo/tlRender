// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/IO/AVF.h>

#include <ftk/Core/Assert.h>
#include <ftk/Core/Format.h>
#include <ftk/Core/LogSystem.h>

#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#import <Foundation/Foundation.h>

namespace tl
{
    namespace avf
    {
        namespace
        {
            const size_t requestTimeout = 5;

            // RAII wrapper for CMSampleBufferRef.
            struct SampleBufferRef
            {
                SampleBufferRef() = default;
                explicit SampleBufferRef(CMSampleBufferRef p) : p(p) {}
                ~SampleBufferRef() { if (p) CFRelease(p); }
                SampleBufferRef(const SampleBufferRef&) = delete;
                SampleBufferRef& operator=(const SampleBufferRef&) = delete;
                CMSampleBufferRef p = nullptr;
            };

            // Core AVFoundation reader object. Lives on the read thread.
            // AVAssetReader is sequential — for random access we recreate
            // it with the appropriate timeRange before each read operation.
            class AVFObject
            {
            public:
                AVFObject(const ftk::Path& path);
                ~AVFObject() = default;

                double getDuration() const { return _duration; }
                double getVideoSpeed() const { return _videoSpeed; }
                const ftk::ImageInfo& getImageInfo() const { return _imageInfo; }
                const AudioInfo& getAudioInfo() const { return _audioInfo; }

                std::shared_ptr<ftk::Image> readImage(const OTIO_NS::RationalTime&);

            private:
                void _initVideo(AVAsset*);
                void _initAudio(AVAsset*);
                void _createReader(const OTIO_NS::RationalTime&);

                NSURL*         _url        = nil;
                double         _duration   = 0.0;
                double         _videoSpeed = 0.0;
                ftk::ImageInfo _imageInfo;
                AudioInfo      _audioInfo;

                // Current sequential reader — recreated on seek.
                AVAssetReader*            _reader      = nil;
                AVAssetReaderTrackOutput* _videoOutput = nil;
                OTIO_NS::RationalTime     _readerStart;

                // Chosen pixel format.
                OSType _pixelFormat = kCVPixelFormatType_24RGB;
            };

            AVFObject::AVFObject(const ftk::Path& path)
            {
                NSString* filePath = [NSString stringWithUTF8String:path.get().c_str()];
                _url = [NSURL fileURLWithPath:filePath];

                AVURLAsset* asset = [AVURLAsset URLAssetWithURL:_url options:nil];

                // Block until asset is loaded — acceptable on the read thread.
                NSError* error = nil;
                [asset loadTracksWithMediaType:AVMediaTypeVideo
                    completionHandler:^(NSArray<AVAssetTrack*>*, NSError*) {}];

                CMTime duration = asset.duration;
                _duration = CMTimeGetSeconds(duration);

                _initVideo(asset);
                _initAudio(asset);
            }

            void AVFObject::_initVideo(AVAsset* asset)
            {
                NSArray<AVAssetTrack*>* tracks =
                    [asset tracksWithMediaType:AVMediaTypeVideo];
                if (tracks.count == 0)
                    return;

                AVAssetTrack* track = tracks[0];

                // Frame rate.
                _videoSpeed = track.nominalFrameRate;
                if (_videoSpeed <= 0.0)
                    _videoSpeed = 24.0;

                // Size.
                CGSize size = track.naturalSize;
                _imageInfo.size.w = static_cast<int>(size.width);
                _imageInfo.size.h = static_cast<int>(size.height);

                // Pixel aspect ratio.
                CMFormatDescriptionRef desc =
                    (__bridge CMFormatDescriptionRef)track.formatDescriptions[0];
                if (desc)
                {
                    CGSize par = CMVideoFormatDescriptionGetPresentationDimensions(
                        desc, false, false);
                    if (par.height > 0 &&
                        par.width != size.width &&
                        par.height != size.height)
                    {
                        _imageInfo.pixelAspectRatio = par.width / par.height;
                    }
                }

                _imageInfo.type = ftk::ImageType::RGB_U8;
            }

            void AVFObject::_initAudio(AVAsset* asset)
            {
                NSArray<AVAssetTrack*>* tracks =
                    [asset tracksWithMediaType:AVMediaTypeAudio];
                if (tracks.count == 0)
                    return;

                AVAssetTrack* track = tracks[0];
                CMFormatDescriptionRef desc =
                    (__bridge CMFormatDescriptionRef)track.formatDescriptions[0];
                if (!desc)
                    return;

                const AudioStreamBasicDescription* asbd =
                    CMAudioFormatDescriptionGetStreamBasicDescription(desc);
                if (!asbd)
                    return;

                _audioInfo.sampleRate   = static_cast<int>(asbd->mSampleRate);
                _audioInfo.channelCount = asbd->mChannelsPerFrame;

                // Map bit depth to AudioType.
                const bool isFloat = (asbd->mFormatFlags & kAudioFormatFlagIsFloat) != 0;
                const UInt32 bitsPerChannel = asbd->mBitsPerChannel;
                if (isFloat && bitsPerChannel == 32)
                    _audioInfo.type = AudioType::F32;
                else if (bitsPerChannel == 32)
                    _audioInfo.type = AudioType::S32;
                else
                    _audioInfo.type = AudioType::S16;
            }

            void AVFObject::_createReader(const OTIO_NS::RationalTime& time)
            {
                AVURLAsset* asset = [AVURLAsset URLAssetWithURL:_url options:nil];
                NSArray<AVAssetTrack*>* tracks =
                    [asset tracksWithMediaType:AVMediaTypeVideo];
                if (tracks.count == 0)
                    return;

                AVAssetTrack* track = tracks[0];

                // Set timeRange from requested frame to end of asset.
                const double startSec = time.rescaled_to(1.0).value();
                CMTime cmStart = CMTimeMakeWithSeconds(startSec, 600);
                CMTime cmDuration = CMTimeMakeWithSeconds(
                    std::max(0.0, _duration - startSec), 600);
                CMTimeRange timeRange = CMTimeRangeMake(cmStart, cmDuration);

                NSError* error = nil;
                _reader = [AVAssetReader assetReaderWithAsset:asset error:&error];
                if (!_reader || error)
                {
                    throw std::runtime_error("Cannot create AVAssetReader");
                }
                _reader.timeRange = timeRange;

                NSDictionary* outputSettings = @{
                    (__bridge NSString*)kCVPixelBufferPixelFormatTypeKey:
                    @(_pixelFormat)
                };
                _videoOutput = [AVAssetReaderTrackOutput
                    assetReaderTrackOutputWithTrack:track
                    outputSettings:outputSettings];
                _videoOutput.alwaysCopiesSampleData = NO;
                [_reader addOutput:_videoOutput];

                if (![_reader startReading])
                {
                    throw std::runtime_error("Cannot start AVAssetReader");
                }

                _readerStart = time;
            }

            std::shared_ptr<ftk::Image> AVFObject::readImage(
                const OTIO_NS::RationalTime& time)
            {
                std::shared_ptr<ftk::Image> out;
                @autoreleasepool
                {
                    // Recreate the reader if this is a seek (non-sequential access).
                    const OTIO_NS::RationalTime nextTime =
                        _readerStart + OTIO_NS::RationalTime(1.0, _videoSpeed);
                    const bool needSeek = (!_reader) ||
                        (time != nextTime && time != _readerStart);
                    if (needSeek)
                    {
                        _reader = nil;
                        _videoOutput = nil;
                        _createReader(time);
                    }

                    while (_reader.status == AVAssetReaderStatusReading)
                    {
                        SampleBufferRef sampleBuf([_videoOutput copyNextSampleBuffer]);
                        if (!sampleBuf.p)
                            break;

                        // Get the presentation timestamp and check it against
                        // the requested time.
                        CMTime pts = CMSampleBufferGetPresentationTimeStamp(sampleBuf.p);
                        const double ptsSec = CMTimeGetSeconds(pts);
                        const OTIO_NS::RationalTime sampleTime(
                            ptsSec * _videoSpeed, _videoSpeed);
                        const OTIO_NS::RationalTime roundedSampleTime = sampleTime.round();

                        if (roundedSampleTime < time)
                        {
                            // Not there yet — keep reading.
                            continue;
                        }

                        // Got the right frame — extract pixel data.
                        CVImageBufferRef imageBuffer =
                            CMSampleBufferGetImageBuffer(sampleBuf.p);
                        if (!imageBuffer)
                            break;

                        CVPixelBufferLockBaseAddress(
                            imageBuffer, kCVPixelBufferLock_ReadOnly);

                        const int w = static_cast<int>(
                            CVPixelBufferGetWidth(imageBuffer));
                        const int h = static_cast<int>(
                            CVPixelBufferGetHeight(imageBuffer));

                        // Update imageInfo size in case it differs from track size.
                        _imageInfo.size.w = w;
                        _imageInfo.size.h = h;

                        out = ftk::Image::create(_imageInfo);

                        if (kCVPixelFormatType_24RGB == _pixelFormat)
                        {
                            const uint8_t* src = static_cast<const uint8_t*>(
                                CVPixelBufferGetBaseAddress(imageBuffer));
                            const size_t srcStride =
                                CVPixelBufferGetBytesPerRow(imageBuffer);
                            uint8_t* outP = out->getData();

                            const int c = ftk::getChannelCount(_imageInfo.type);
                            for (int y = 0; y < h; ++y)
                            {
                                memcpy(outP + (h - 1 - y) * w * c, src + y * srcStride, w * c);
                            }
                        }

                        CVPixelBufferUnlockBaseAddress(
                            imageBuffer, kCVPixelBufferLock_ReadOnly);

                        _readerStart = time +
                            OTIO_NS::RationalTime(1.0, _videoSpeed);
                        break;
                    }
                }
                return out;
            }
        }

        struct Read::Private
        {
            IOInfo info;

            struct InfoRequest
            {
                std::promise<IOInfo> promise;
            };

            struct VideoRequest
            {
                OTIO_NS::RationalTime time = invalidTime;
                IOOptions options;
                std::promise<VideoData> promise;
            };

            struct AudioRequest
            {
                OTIO_NS::TimeRange timeRange = invalidTimeRange;
                IOOptions options;
                std::promise<AudioData> promise;
            };

            struct Mutex
            {
                std::list<std::shared_ptr<InfoRequest>>  infoRequests;
                std::list<std::shared_ptr<VideoRequest>> videoRequests;
                std::list<std::shared_ptr<AudioRequest>> audioRequests;
                bool stopped = false;
                std::mutex mutex;
            };
            Mutex mutex;

            struct Thread
            {
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            Thread thread;
        };

        void Read::_init(
            const ftk::Path& path,
            const std::vector<ftk::MemFile>& memory,
            const IOOptions& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            IRead::_init(path, memory, options, logSystem);
            FTK_P();

            p.thread.running = true;
            p.thread.thread = std::thread(
                [this, path]
                {
                    FTK_P();
                    try
                    {
                        _thread(path);
                    }
                    catch (const std::exception& e)
                    {
                        if (auto logSystem = _logSystem.lock())
                        {
                            logSystem->print(
                                "tl::avf::Read",
                                e.what(),
                                ftk::LogType::Error);
                        }
                    }
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        p.mutex.stopped = true;
                    }
                    cancelRequests();
                });
        }

        Read::Read() :
            _p(new Private)
        {}

        Read::~Read()
        {
            FTK_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
        }

        std::shared_ptr<Read> Read::create(
            const ftk::Path& path,
            const IOOptions& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, {}, options, logSystem);
            return out;
        }

        std::shared_ptr<Read> Read::create(
            const ftk::Path& path,
            const std::vector<ftk::MemFile>& memory,
            const IOOptions& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, memory, options, logSystem);
            return out;
        }

        std::future<IOInfo> Read::getInfo()
        {
            FTK_P();
            auto request = std::make_shared<Private::InfoRequest>();
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.infoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(IOInfo());
            }
            return future;
        }

        std::future<VideoData> Read::readVideo(
            const OTIO_NS::RationalTime& time,
            const IOOptions& options)
        {
            FTK_P();
            auto request = std::make_shared<Private::VideoRequest>();
            request->time = time;
            request->options = merge(options, _options);
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.videoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(VideoData());
            }
            return future;
        }

        std::future<AudioData> Read::readAudio(
            const OTIO_NS::TimeRange& timeRange,
            const IOOptions& options)
        {
            FTK_P();
            auto request = std::make_shared<Private::AudioRequest>();
            request->timeRange = timeRange;
            request->options = merge(options, _options);
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.audioRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(AudioData());
            }
            return future;
        }

        void Read::cancelRequests()
        {
            FTK_P();
            std::list<std::shared_ptr<Private::InfoRequest>> infoRequests;
            std::list<std::shared_ptr<Private::VideoRequest>> videoRequests;
            std::list<std::shared_ptr<Private::AudioRequest>> audioRequests;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                infoRequests = std::move(p.mutex.infoRequests);
                videoRequests = std::move(p.mutex.videoRequests);
                audioRequests = std::move(p.mutex.audioRequests);
            }
            for (auto& r : infoRequests)
            {
                r->promise.set_value(IOInfo());
            }
            for (auto& r : videoRequests)
            {
                r->promise.set_value(VideoData());
            }
            for (auto& r : audioRequests)
            {
                r->promise.set_value(AudioData());
            }
        }

        void Read::_thread(const ftk::Path& path)
        {
            @autoreleasepool
            {
                FTK_P();

                AVFObject avf(path);

                p.info.video.push_back(avf.getImageInfo());
                p.info.videoTime = OTIO_NS::TimeRange(
                    OTIO_NS::RationalTime(0.0, avf.getVideoSpeed()),
                    OTIO_NS::RationalTime(
                        avf.getDuration() * avf.getVideoSpeed(),
                        avf.getVideoSpeed()).floor());
                p.info.audio = avf.getAudioInfo();
                if (p.info.audio.sampleRate > 0)
                {
                    p.info.audioTime = OTIO_NS::TimeRange(
                        OTIO_NS::RationalTime(0.0, p.info.audio.sampleRate),
                        OTIO_NS::RationalTime(
                            avf.getDuration() * p.info.audio.sampleRate,
                            p.info.audio.sampleRate).floor());
                }

                while (p.thread.running)
                {
                    std::list<std::shared_ptr<Private::InfoRequest>> infoRequests;
                    std::shared_ptr<Private::VideoRequest> videoRequest;
                    std::shared_ptr<Private::AudioRequest> audioRequest;
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        if (p.thread.cv.wait_for(
                            lock,
                            std::chrono::milliseconds(requestTimeout),
                            [this]
                            {
                                return
                                    !_p->mutex.infoRequests.empty() ||
                                    !_p->mutex.videoRequests.empty() ||
                                    !_p->mutex.audioRequests.empty();
                            }))
                        {
                            infoRequests = std::move(p.mutex.infoRequests);
                            if (!p.mutex.videoRequests.empty())
                            {
                                videoRequest = p.mutex.videoRequests.front();
                                p.mutex.videoRequests.pop_front();
                            }
                            if (!p.mutex.audioRequests.empty())
                            {
                                audioRequest = p.mutex.audioRequests.front();
                                p.mutex.audioRequests.pop_front();
                            }
                        }
                    }

                    // Information requests.
                    for (auto& request : infoRequests)
                    {
                        request->promise.set_value(p.info);
                    }

                    // Video requests.
                    if (videoRequest)
                    {
                        VideoData data;
                        data.time = videoRequest->time;
                        try
                        {
                            data.image = avf.readImage(videoRequest->time);
                        }
                        catch (const std::exception& e)
                        {
                            if (auto logSystem = _logSystem.lock())
                            {
                                logSystem->print(
                                    "tl::avf::Read",
                                    e.what(),
                                    ftk::LogType::Error);
                            }
                        }
                        videoRequest->promise.set_value(data);
                    }

                    // Audio requests — return silence for now (audio decode
                    // via AVAssetReaderTrackOutput can be added similarly).
                    if (audioRequest)
                    {
                        AudioData audioData;
                        audioData.time = audioRequest->timeRange.start_time();
                        audioData.audio = Audio::create(
                            p.info.audio,
                            audioRequest->timeRange.duration().value());
                        audioData.audio->zero();
                        audioRequest->promise.set_value(audioData);
                    }
                }
            }
        }
    }
}
