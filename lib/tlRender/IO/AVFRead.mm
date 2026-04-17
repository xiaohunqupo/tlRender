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

#include <list>

// Holds the persistent audio AVAssetReader and its track output.
// Declared at global scope (required for Objective-C @interface/@implementation)
// and stored inside AVFObject so ARC correctly retains the reader and output
// across C++ call boundaries.
@interface AVFAudioReader : NSObject
@property (nonatomic, strong) AVAssetReader*            reader;
@property (nonatomic, strong) AVAssetReaderTrackOutput* output;
@end
@implementation AVFAudioReader
@end

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
            // The video AVAssetReader is sequential — for random access we
            // recreate it with the appropriate timeRange before each read.
            // The audio AVAssetReader is persistent across sequential calls
            // to avoid the per-call cost of reopening the asset.
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
                std::shared_ptr<Audio>      readAudio(const OTIO_NS::TimeRange&);

            private:
                void _initVideo(AVAsset*);
                void _initAudio(AVAsset*);
                void _createReader(const OTIO_NS::RationalTime&);
                void _createAudioReader(int64_t startSample);

                // Build the AVAssetReaderTrackOutput settings dictionary for
                // linear PCM output.
                NSDictionary* _audioOutputSettings() const;

                // Interleave keepCount planar frames from abl into dst,
                // starting at leadTrim within each channel plane.
                void _copyPlanarAudio(
                    const AudioBufferList* abl,
                    int64_t                leadTrim,
                    int64_t                keepCount,
                    int                    bytesPerSample,
                    uint8_t*               dst) const;

                // Copy keepCount interleaved frames from abl into a new Audio
                // object, handling both interleaved (mNumberBuffers==1) and
                // planar (mNumberBuffers>1) layouts.  leadTrim frames are
                // skipped at the start of each buffer.
                std::shared_ptr<Audio> _copyFromABL(
                    const AudioBufferList* abl,
                    int64_t                leadTrim,
                    int64_t                keepCount,
                    int                    bytesPerSample,
                    const AudioInfo&       info) const;

                // Convert a double timestamp (seconds) to an exact integer
                // sample index using CMTime arithmetic to avoid float drift.
                int64_t _secondsToSamples(double seconds) const;

                NSURL*         _url        = nil;
                double         _duration   = 0.0;
                double         _videoSpeed = 0.0;
                ftk::ImageInfo _imageInfo;
                AudioInfo      _audioInfo;

                // Current sequential video reader — recreated on seek.
                AVAssetReader*            _reader      = nil;
                AVAssetReaderTrackOutput* _videoOutput = nil;
                OTIO_NS::RationalTime     _readerStart;

                // Current sequential audio reader — kept alive between calls.
                // Stored in an Objective-C object so ARC manages the lifetime
                // correctly across C++ call boundaries.
                AVFAudioReader* _audioReaderState = nil;
                int64_t         _audioReaderNext  = -1; // next expected sample

                // Carry buffer: decoded Audio chunks from the previous call
                // that weren't yet consumed.  Drained at the start of the
                // next call via moveAudio so no samples are discarded at
                // chunk boundaries.
                std::list<std::shared_ptr<Audio> > _audioCarry;

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

            NSDictionary* AVFObject::_audioOutputSettings() const
            {
                const bool wantFloat =
                    (_audioInfo.type == AudioType::F32 ||
                     _audioInfo.type == AudioType::F64);
                const int bitsPerSample = wantFloat ? 32 : 16;
                return @{
                    AVFormatIDKey:               @(kAudioFormatLinearPCM),
                    AVLinearPCMBitDepthKey:      @(bitsPerSample),
                    AVLinearPCMIsFloatKey:       @(wantFloat ? YES : NO),
                    AVLinearPCMIsBigEndianKey:   @NO,
                    AVLinearPCMIsNonInterleaved: @NO,
                    AVNumberOfChannelsKey:       @(_audioInfo.channelCount),
                    AVSampleRateKey:             @((double)_audioInfo.sampleRate)
                };
            }

            void AVFObject::_copyPlanarAudio(
                const AudioBufferList* abl,
                int64_t                leadTrim,
                int64_t                keepCount,
                int                    bytesPerSample,
                uint8_t*               dst) const
            {
                const size_t chCount = static_cast<size_t>(_audioInfo.channelCount);
                for (int64_t s = 0; s < keepCount; ++s)
                {
                    for (UInt32 c = 0; c < abl->mNumberBuffers && c < chCount; ++c)
                    {
                        const uint8_t* src = static_cast<const uint8_t*>(
                            abl->mBuffers[c].mData) +
                            (leadTrim + s) * bytesPerSample;
                        std::memcpy(dst, src, bytesPerSample);
                        dst += bytesPerSample;
                    }
                }
            }

            std::shared_ptr<Audio> AVFObject::_copyFromABL(
                const AudioBufferList* abl,
                int64_t                leadTrim,
                int64_t                keepCount,
                int                    bytesPerSample,
                const AudioInfo&       info) const
            {
                auto audio = Audio::create(info, static_cast<size_t>(keepCount));
                if (abl->mNumberBuffers == 1)
                {
                    const size_t frameBytes = info.getByteCount();
                    const uint8_t* src =
                        static_cast<const uint8_t*>(abl->mBuffers[0].mData) +
                        leadTrim * frameBytes;
                    std::memcpy(audio->getData(), src, keepCount * frameBytes);
                }
                else
                {
                    _copyPlanarAudio(abl, leadTrim, keepCount,
                        bytesPerSample, audio->getData());
                }
                return audio;
            }

            int64_t AVFObject::_secondsToSamples(double seconds) const
            {
                return CMTimeConvertScale(
                    CMTimeMakeWithSeconds(seconds, 1000000),
                    _audioInfo.sampleRate,
                    kCMTimeRoundingMethod_RoundHalfAwayFromZero).value;
            }

            void AVFObject::_createAudioReader(int64_t startSample)
            {
                _audioReaderState = nil;
                _audioReaderNext  = -1;
                _audioCarry.clear();

                if (_audioInfo.sampleRate <= 0 || _audioInfo.channelCount == 0)
                    return;

                AVURLAsset* asset = [AVURLAsset URLAssetWithURL:_url options:nil];
                NSArray<AVAssetTrack*>* tracks =
                    [asset tracksWithMediaType:AVMediaTypeAudio];
                if (tracks.count == 0)
                    return;

                // Open the reader from the exact requested start sample.
                // AVFoundation will round the seek point to a packet boundary;
                // the first delivered buffer may start slightly before or after
                // requestStart — the decode loop handles both cases via leadTrim
                // and the forward-skip check.
                const double startSec =
                    static_cast<double>(startSample) / _audioInfo.sampleRate;
                CMTimeRange cmRange = CMTimeRangeMake(
                    CMTimeMakeWithSeconds(startSec, 1000000),
                    kCMTimePositiveInfinity);

                NSError* error = nil;
                AVAssetReader* reader =
                    [AVAssetReader assetReaderWithAsset:asset error:&error];
                if (!reader || error)
                    return;
                reader.timeRange = cmRange;

                AVAssetReaderTrackOutput* output = [AVAssetReaderTrackOutput
                    assetReaderTrackOutputWithTrack:tracks[0]
                    outputSettings:_audioOutputSettings()];
                output.alwaysCopiesSampleData = NO;
                [reader addOutput:output];

                if (![reader startReading])
                    return;

                AVFAudioReader* state = [[AVFAudioReader alloc] init];
                state.reader = reader;
                state.output = output;
                _audioReaderState = state;
                _audioReaderNext  = startSample;
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

            std::shared_ptr<Audio> AVFObject::readAudio(
                const OTIO_NS::TimeRange& timeRange)
            {
                std::shared_ptr<Audio> out;
                if (_audioInfo.sampleRate <= 0 || _audioInfo.channelCount == 0)
                    return out;

                @autoreleasepool
                {
                    // Output format — must match _audioOutputSettings().
                    const bool      wantFloat      = (_audioInfo.type == AudioType::F32 ||
                                                      _audioInfo.type == AudioType::F64);
                    const int       bytesPerSample = wantFloat ? 4 : 2;
                    const AudioType outputType     = wantFloat ? AudioType::F32
                                                               : AudioType::S16;
                    AudioInfo outInfo = _audioInfo;
                    outInfo.type      = outputType;

                    // Requested window in exact sample units (CMTime arithmetic
                    // avoids floating-point drift over long files).
                    const int64_t requestStart =
                        _secondsToSamples(timeRange.start_time().rescaled_to(1.0).value());
                    const int64_t requestCount =
                        _secondsToSamples(timeRange.duration().rescaled_to(1.0).value());

                    if (requestCount <= 0)
                        return out;

                    // --- Seek decision ---
                    // Reuse the persistent reader for sequential access.
                    // maxForwardSkip covers AVFoundation's tendency to round seek
                    // points forward to the next packet boundary (observed up to
                    // ~25000 samples in practice for a 60-second AAC file).
                    // Gaps smaller than one full second are treated as sequential.
                    const int64_t maxForwardSkip = _audioInfo.sampleRate;
                    const bool    readerDead     =
                        (_audioReaderState == nil) ||
                        (_audioReaderState.reader.status != AVAssetReaderStatusReading);
                    const bool    seekBack       =
                        !readerDead && (requestStart < _audioReaderNext);
                    const bool    seekForward    =
                        !readerDead && !seekBack &&
                        (requestStart > _audioReaderNext + maxForwardSkip);

                    if (readerDead || seekBack || seekForward)
                    {
                        _createAudioReader(requestStart);
                        if (!_audioReaderState)
                            return out;
                    }

                    AVAssetReader*            audioReader = _audioReaderState.reader;
                    AVAssetReaderTrackOutput* audioOutput = _audioReaderState.output;

                    // Accumulate decoded Audio chunks.
                    std::list<std::shared_ptr<Audio>> buffer;

                    int64_t nextExpectedSample  = requestStart;
                    // Tracks where decoded audio actually starts — may be after
                    // requestStart when AVFoundation rounds a seek point forward.
                    int64_t actualConsumedStart = requestStart;

                    // --- Phase 1: drain carry buffer ---
                    // The carry buffer holds Audio chunks left over from the
                    // previous call.  moveAudio handles partial-chunk splitting
                    // so no samples are discarded at chunk boundaries.
                    if (!_audioCarry.empty())
                    {
                        const size_t carryCount = getSampleCount(_audioCarry);
                        const size_t keepCount  = std::min(
                            carryCount,
                            static_cast<size_t>(requestCount));
                        buffer.splice(buffer.end(), _audioCarry,
                            _audioCarry.begin(), _audioCarry.end());
                        // Trim the list to keepCount — moveAudio will split the
                        // last chunk correctly when we build the output.
                        (void)keepCount; // list is trimmed implicitly by requestCount cap
                        nextExpectedSample = requestStart +
                            static_cast<int64_t>(getSampleCount(buffer));
                    }

                    // --- Phase 2: decode new buffers ---
                    while (audioReader.status == AVAssetReaderStatusReading &&
                           static_cast<int64_t>(getSampleCount(buffer)) < requestCount)
                    {
                        SampleBufferRef sampleBuf([audioOutput copyNextSampleBuffer]);
                        if (!sampleBuf.p)
                            break;

                        const int64_t bufStart = CMTimeConvertScale(
                            CMSampleBufferGetPresentationTimeStamp(sampleBuf.p),
                            _audioInfo.sampleRate,
                            kCMTimeRoundingMethod_RoundHalfAwayFromZero).value;
                        const int64_t bufCount = static_cast<int64_t>(
                            CMSampleBufferGetNumSamples(sampleBuf.p));

                        // Discard buffers that lie entirely before our window.
                        if (bufStart + bufCount <= requestStart)
                            continue;

                        // First buffer after a seek: AVFoundation may have rounded
                        // the start forward.  Accept the actual start rather than
                        // reading garbage; the caller gets silence for the gap.
                        if (buffer.empty() && bufStart > requestStart)
                        {
                            nextExpectedSample  = bufStart;
                            actualConsumedStart = bufStart;
                        }

                        // leadTrim: skip samples before our window.
                        // Use requestStart for the first buffer, nextExpectedSample
                        // for subsequent ones to maintain sample-exact continuity.
                        const int64_t anchor   = buffer.empty()
                                                    ? requestStart : nextExpectedSample;
                        const int64_t leadTrim = std::max(int64_t(0), anchor - bufStart);

                        // Unpack the PCM data from the sample buffer.
                        // Two-call pattern: first call gets the required storage size.
                        size_t ablSize = 0;
                        CMSampleBufferGetAudioBufferListWithRetainedBlockBuffer(
                            sampleBuf.p, &ablSize, nullptr, 0,
                            nullptr, nullptr, 0, nullptr);

                        std::vector<uint8_t> ablStorage(ablSize);
                        auto* abl = reinterpret_cast<AudioBufferList*>(ablStorage.data());

                        CMBlockBufferRef blockBuffer = nil;
                        const OSStatus ablStatus =
                            CMSampleBufferGetAudioBufferListWithRetainedBlockBuffer(
                                sampleBuf.p, nullptr, abl, ablSize,
                                nullptr, nullptr,
                                kCMSampleBufferFlag_AudioBufferList_Assure16ByteAlignment,
                                &blockBuffer);
                        if (ablStatus != noErr || !blockBuffer)
                            continue;

                        const int64_t keepCount = bufCount - leadTrim;
                        if (keepCount > 0)
                        {
                            buffer.push_back(
                                _copyFromABL(abl, leadTrim, keepCount,
                                    bytesPerSample, outInfo));
                            nextExpectedSample = bufStart + leadTrim + keepCount;
                        }

                        CFRelease(blockBuffer);
                    }

                    // Save any samples beyond requestCount into the carry buffer
                    // so they are used on the next call.  moveAudio handles
                    // splitting the last Audio chunk at the exact sample boundary.
                    const size_t accumulated = getSampleCount(buffer);
                    if (static_cast<int64_t>(accumulated) > requestCount)
                    {
                        // Build carry from the overflow portion.
                        const size_t leftover =
                            accumulated - static_cast<size_t>(requestCount);
                        auto carry = Audio::create(outInfo, leftover);
                        // Flatten buffer into a contiguous block, then slice.
                        auto flat = Audio::create(outInfo, accumulated);
                        moveAudio(buffer, flat->getData(), accumulated);
                        std::memcpy(carry->getData(),
                            flat->getData() + static_cast<size_t>(requestCount) *
                                outInfo.getByteCount(),
                            carry->getByteCount());
                        _audioCarry.push_back(carry);
                        // Put just requestCount samples back for the output.
                        auto keep = Audio::create(outInfo,
                            static_cast<size_t>(requestCount));
                        std::memcpy(keep->getData(), flat->getData(), keep->getByteCount());
                        buffer.clear();
                        buffer.push_back(keep);
                        _audioReaderNext =
                            nextExpectedSample - static_cast<int64_t>(leftover);
                    }
                    else
                    {
                        _audioReaderNext = nextExpectedSample;
                    }

                    // On EOF or error, mark the reader dead so it is recreated next call.
                    if (audioReader.status != AVAssetReaderStatusReading)
                    {
                        _audioReaderState = nil;
                        _audioReaderNext  = -1;
                        _audioCarry.clear();
                    }

                    if (buffer.empty())
                        return out;

                    // --- Phase 3: build output Audio ---
                    // If AVFoundation rounded the seek forward past requestStart,
                    // prepend silence so the returned buffer starts at requestStart.
                    // padAudioToOneSecond uses the requested timeline range and
                    // expects audio to begin at requestStart; the silence fills
                    // the unavoidable gap at seek boundaries.
                    const int64_t silenceSamples =
                        std::max(int64_t(0), actualConsumedStart - requestStart);

                    if (silenceSamples > 0)
                    {
                        auto silence = Audio::create(outInfo,
                            static_cast<size_t>(silenceSamples));
                        silence->zero();
                        buffer.push_front(silence);
                    }

                    const size_t totalSamples = getSampleCount(buffer);
                    out = Audio::create(outInfo, totalSamples);
                    moveAudio(buffer, out->getData(), totalSamples);
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

                    // Audio requests.
                    if (audioRequest)
                    {
                        AudioData audioData;
                        audioData.time = audioRequest->timeRange.start_time();
                        try
                        {
                            audioData.audio = avf.readAudio(audioRequest->timeRange);
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
                        if (!audioData.audio)
                        {
                            // Fall back to silence on decode failure.
                            const size_t sampleCount = static_cast<size_t>(
                                audioRequest->timeRange
                                    .duration()
                                    .rescaled_to(p.info.audio.sampleRate)
                                    .value());
                            audioData.audio = Audio::create(p.info.audio, sampleCount);
                            audioData.audio->zero();
                        }
                        audioRequest->promise.set_value(audioData);
                    }
                }
            }
        }
    }
}

