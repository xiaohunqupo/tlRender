// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <feather-tk/core/Range.h>
#include <feather-tk/core/Util.h>

#include <limits>
#include <list>
#include <memory>
#include <string>
#include <vector>

namespace tl
{
    //! Audio
    namespace audio
    {
        //! \name Audio Types
        ///@{

        //! Audio data types.
        enum class DataType
        {
            None,
            S8,
            S16,
            S32,
            F32,
            F64,

            Count,
            First = None
        };
        FEATHER_TK_ENUM(DataType);

        typedef int8_t   S8_T;
        typedef int16_t S16_T;
        typedef int32_t S32_T;
        typedef float   F32_T;
        typedef double  F64_T;

        const feather_tk::Range<S8_T> S8Range(
            std::numeric_limits<S8_T>::min(),
            std::numeric_limits<S8_T>::max());
        const feather_tk::Range<S16_T> S16Range(
            std::numeric_limits<S16_T>::min(),
            std::numeric_limits<S16_T>::max());
        const feather_tk::Range<S32_T> S32Range(
            std::numeric_limits<S32_T>::min(),
            std::numeric_limits<S32_T>::max());
        const feather_tk::Range<F32_T> F32Range(-1.F, 1.F);
        const feather_tk::Range<F64_T> F64Range(-1.F, 1.F);

        //! Get the byte count for the given data type.
        size_t getByteCount(DataType);

        //! Determine the integer data type for a given byte count.
        DataType getIntType(size_t);

        //! Determine the floating point data type for a given byte count.
        DataType getFloatType(size_t);

        ///@}

        //! \name Audio Type Conversion
        ///@{

        void S8ToS16(S8_T, S16_T&);
        void S8ToS32(S8_T, S32_T&);
        void S8ToF32(S8_T, F32_T&);
        void S8ToF64(S8_T, F64_T&);

        void S16ToS8(S16_T, S8_T&);
        void S16ToS32(S16_T, S32_T&);
        void S16ToF32(S16_T, F32_T&);
        void S16ToF64(S16_T, F64_T&);

        void S32ToS8(S32_T, S8_T&);
        void S32ToS16(S32_T, S16_T&);
        void S32ToF32(S32_T, F32_T&);
        void S32ToF64(S32_T, F64_T&);

        void F32ToS8(F32_T, S8_T&);
        void F32ToS16(F32_T, S16_T&);
        void F32ToS32(F32_T, S32_T&);
        void F32ToF64(F32_T, F64_T&);

        void F64ToS8(F64_T, S8_T&);
        void F64ToS16(F64_T, S16_T&);
        void F64ToS32(F64_T, S32_T&);
        void F64ToF32(F64_T, F32_T&);

        ///@}

        //! Audio data information.
        class Info
        {
        public:
            Info();
            Info(
                size_t   channelCount,
                DataType dataType,
                size_t   sampleRate);

            std::string name         = "Default";
            size_t      channelCount = 0;
            DataType    dataType     = DataType::None;
            size_t      sampleRate   = 0;

            //! Is the audio valid?
            bool isValid() const;

            //! Get the byte count.
            size_t getByteCount() const;

            bool operator == (const Info&) const;
            bool operator != (const Info&) const;
        };

        //! Audio data.
        class Audio : public std::enable_shared_from_this<Audio>
        {
            FEATHER_TK_NON_COPYABLE(Audio);

        protected:
            void _init(const Info&, size_t sampleCount);

            Audio();

        public:
            ~Audio();

            //! Create new audio.
            static std::shared_ptr<Audio> create(
                const Info& info,
                size_t      sampleCount);

            //! Get the audio information.
            const Info& getInfo() const;

            //! Get the audio channel count.
            size_t getChannelCount() const;

            //! Get the audio data type.
            DataType getDataType() const;

            //! Get the audio sample rate.
            size_t getSampleRate() const;

            //! Get the audio sample count.
            size_t getSampleCount() const;

            //! Is the audio valid?
            bool isValid() const;

            //! Get the audio data byte count.
            size_t getByteCount() const;

            //! Get the audio data.
            uint8_t* getData();

            //! Get the audio data.
            const uint8_t* getData() const;

            //! Zero the audio data.
            void zero();

        private:
            Info _info;
            size_t _sampleCount = 0;
            std::vector<uint8_t> _data;
        };

        //! \name Utility
        ///@{

        //! Combine chunks of audio. The chunks should all have the same
        //! number of channels and type.
        std::shared_ptr<audio::Audio> combine(
            const std::list<std::shared_ptr<audio::Audio> >&);

        //! Mix audio sources.
        std::shared_ptr<Audio> mix(
            const std::vector<std::shared_ptr<Audio> >&,
            float volume,
            const std::vector<bool>& channelMute = {});

        //! Reverse audio.
        std::shared_ptr<Audio> reverse(const std::shared_ptr<Audio>&);

        //! Change audio speed.
        std::shared_ptr<Audio> changeSpeed(const std::shared_ptr<Audio>&, double);

        //! Convert audio data.
        std::shared_ptr<Audio> convert(const std::shared_ptr<Audio>&, DataType);

        //! Get the total sample count from a list of audio data.
        size_t getSampleCount(const std::list<std::shared_ptr<audio::Audio> >&);

        //! Move audio data.
        void move(
            std::list<std::shared_ptr<Audio> >& in,
            uint8_t* out,
            size_t sampleCount);

        ///@}
    }
}

#include <tlCore/AudioInline.h>
