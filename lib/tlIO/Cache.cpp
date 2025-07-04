// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/Cache.h>

#include <feather-tk/core/Format.h>
#include <feather-tk/core/LRUCache.h>
#include <feather-tk/core/String.h>

#include <mutex>

namespace tl
{
    namespace io
    {
        std::string getInfoCacheKey(
            const file::Path& path,
            const Options& options)
        {
            std::vector<std::string> s;
            s.push_back(path.get());
            s.push_back(path.getNumber());
            for (const auto& i : options)
            {
                s.push_back(feather_tk::Format("{0}:{1}").arg(i.first).arg(i.second));
            }
            return feather_tk::join(s, ';');
        }

        std::string getVideoCacheKey(
            const file::Path& path,
            const OTIO_NS::RationalTime& time,
            const Options& initOptions,
            const Options& frameOptions)
        {
            std::stringstream ss;
            ss << path.get() << ";" << path.getNumber() << ";" << time << ";";
            for (const auto& i : initOptions)
            {
                ss << i.first << ":" << i.second << ";";
            }
            for (const auto& i : frameOptions)
            {
                ss << i.first << ":" << i.second << ";";
            }
            return ss.str();
        }

        std::string getAudioCacheKey(
            const file::Path& path,
            const OTIO_NS::TimeRange& timeRange,
            const Options& initOptions,
            const Options& frameOptions)
        {
            std::stringstream ss;
            ss << path.get() << ";" << path.getNumber() << ";" << timeRange << ";";
            for (const auto& i : initOptions)
            {
                ss << i.first << ":" << i.second << ";";
            }
            for (const auto& i : frameOptions)
            {
                ss << i.first << ":" << i.second << ";";
            }
            return ss.str();
        }

        struct Cache::Private
        {
            size_t max = feather_tk::gigabyte;
            feather_tk::LRUCache<std::string, VideoData> video;
            feather_tk::LRUCache<std::string, AudioData> audio;
            std::mutex mutex;
        };

        void Cache::_init()
        {
            _maxUpdate();
        }

        Cache::Cache() :
            _p(new Private)
        {}

        Cache::~Cache()
        {}

        std::shared_ptr<Cache> Cache::create()
        {
            auto out = std::shared_ptr<Cache>(new Cache);
            out->_init();
            return out;
        }

        size_t Cache::getMax() const
        {
            return _p->max;
        }

        void Cache::setMax(size_t value)
        {
            FEATHER_TK_P();
            if (value == p.max)
                return;
            p.max = value;
            _maxUpdate();
        }

        size_t Cache::getSize() const
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.video.getSize() + p.audio.getSize();
        }

        float Cache::getPercentage() const
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return
                (p.video.getSize() + p.audio.getSize()) /
                static_cast<float>(p.video.getMax() + p.audio.getMax()) * 100.F;
        }

        void Cache::addVideo(const std::string& key, const VideoData& videoData)
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.video.add(
                key,
                videoData,
                videoData.image ? videoData.image->getByteCount() : 1);
        }

        bool Cache::containsVideo(const std::string& key) const
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.video.contains(key);
        }

        bool Cache::getVideo(const std::string& key, VideoData& videoData) const
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.video.get(key, videoData);
        }

        void Cache::addAudio(const std::string& key, const AudioData& audioData)
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.audio.add(
                key,
                audioData,
                audioData.audio ? audioData.audio->getByteCount() : 1);
        }

        bool Cache::containsAudio(const std::string& key) const
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.audio.contains(key);
        }

        bool Cache::getAudio(const std::string& key, AudioData& audioData) const
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.audio.get(key, audioData);
        }

        void Cache::clear()
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.video.clear();
            p.audio.clear();
        }

        void Cache::_maxUpdate()
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.video.setMax(p.max * .9F);
            p.audio.setMax(p.max * .1F);
        }
    }
}
