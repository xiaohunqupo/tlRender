// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCoreTest/AudioTest.h>

#include <tlCore/AudioResample.h>
#include <tlCore/AudioSystem.h>

#include <cstring>

using namespace tl::audio;

namespace tl
{
    namespace core_tests
    {
        AudioTest::AudioTest(const std::shared_ptr<feather_tk::Context>& context) :
            ITest(context, "core_tests::AudioTest")
        {}

        std::shared_ptr<AudioTest> AudioTest::create(const std::shared_ptr<feather_tk::Context>& context)
        {
            return std::shared_ptr<AudioTest>(new AudioTest(context));
        }

        void AudioTest::run()
        {
            _enums();
            _types();
            _audio();
            _audioSystem();
            _combine();
            _mix();
            _reverse();
            _convert();
            _move();
            _resample();
        }

        void AudioTest::_enums()
        {
            _enum<DataType>("DataType", getDataTypeEnums);
        }

        void AudioTest::_types()
        {
            for (auto i : getDataTypeEnums())
            {
                std::stringstream ss;
                ss << i << " byte count: " << getByteCount(i);
                _print(ss.str());
            }
            for (auto i : { 0, 1, 2, 3, 4, 5, 6, 7, 8 })
            {
                std::stringstream ss;
                ss << i << " bytes int type: " << getIntType(i);
                _print(ss.str());
            }
            for (auto i : { 0, 1, 2, 3, 4, 5, 6, 7, 8 })
            {
                std::stringstream ss;
                ss << i << " bytes float type: " << getFloatType(i);
                _print(ss.str());
            }
        }

        void AudioTest::_audio()
        {
            {
                const Info info(2, DataType::S16, 44100);
                FEATHER_TK_ASSERT(info == info);
                FEATHER_TK_ASSERT(info != Info());
                auto audio = Audio::create(info, 1000);
                audio->zero();
                FEATHER_TK_ASSERT(audio->getInfo() == info);
                FEATHER_TK_ASSERT(audio->getChannelCount() == info.channelCount);
                FEATHER_TK_ASSERT(audio->getDataType() == info.dataType);
                FEATHER_TK_ASSERT(audio->getSampleRate() == info.sampleRate);
                FEATHER_TK_ASSERT(audio->getSampleCount() == 1000);
                FEATHER_TK_ASSERT(audio->isValid());
                FEATHER_TK_ASSERT(audio->getData());
                FEATHER_TK_ASSERT(static_cast<const Audio*>(audio.get())->getData());
            }
        }

        void AudioTest::_audioSystem()
        {
            auto system = _context->getSystem<System>();
            for (const auto& i : system->getDrivers())
            {
                std::stringstream ss;
                ss << "api: " << i;
                _print(ss.str());
            }
            for (const auto& i : system->getDevices())
            {
                std::stringstream ss;
                ss << "device: " << i.id.number << " " << i.id.name;
                _print(ss.str());
            }
            {
                std::stringstream ss;
                const DeviceInfo device = system->getDefaultDevice();
                ss << "default device: " << device.id.number << " " << device.id.name;
                _print(ss.str());
            }
        }

        void AudioTest::_combine()
        {
            std::list<std::shared_ptr<Audio> > list;
            auto audio = Audio::create(Info(1, DataType::S8, 41000), 1);
            audio->getData()[0] = 1;
            list.push_back(audio);
            audio = Audio::create(Info(1, DataType::S8, 41000), 1);
            audio->getData()[0] = 2;
            list.push_back(audio);
            audio = Audio::create(Info(1, DataType::S8, 41000), 1);
            audio->getData()[0] = 3;
            list.push_back(audio);
            auto combined = combine(list);
            FEATHER_TK_ASSERT(3 == combined->getSampleCount());
            FEATHER_TK_ASSERT(1 == combined->getData()[0]);
            FEATHER_TK_ASSERT(2 == combined->getData()[1]);
            FEATHER_TK_ASSERT(3 == combined->getData()[2]);
        }

        namespace
        {
            template<DataType DT, typename T>
            void _mixI()
            {
                const Info info(1, DT, 48000);

                auto audio0 = Audio::create(info, 5);
                T* p0 = reinterpret_cast<T*>(audio0->getData());
                p0[0] = 0;
                p0[1] = std::numeric_limits<T>::max();
                p0[2] = std::numeric_limits<T>::min();
                p0[3] = std::numeric_limits<T>::max();
                p0[4] = std::numeric_limits<T>::min();

                auto audio1 = Audio::create(info, 5);
                T* p1 = reinterpret_cast<T*>(audio1->getData());
                p1[0] = 0;
                p1[1] = std::numeric_limits<T>::max();
                p1[2] = std::numeric_limits<T>::min();
                p1[3] = std::numeric_limits<T>::min();
                p1[4] = std::numeric_limits<T>::max();

                auto out = mix({ audio0, audio1 }, 1.0);
                const T* outP = reinterpret_cast<T*>(out->getData());
                FEATHER_TK_ASSERT(0 == outP[0]);
                FEATHER_TK_ASSERT(std::numeric_limits<T>::max() == outP[1]);
                FEATHER_TK_ASSERT(std::numeric_limits<T>::min() == outP[2]);
                FEATHER_TK_ASSERT(std::numeric_limits<T>::max() + std::numeric_limits<T>::min() == outP[3]);
                FEATHER_TK_ASSERT(std::numeric_limits<T>::max() + std::numeric_limits<T>::min() == outP[4]);
            }

            template<DataType DT, typename T>
            void _mixF()
            {
                const Info info(1, DT, 48000);

                auto audio0 = Audio::create(info, 5);
                T* p0 = reinterpret_cast<T*>(audio0->getData());
                p0[0] = 0;
                p0[1] = 1;
                p0[2] = -1;
                p0[3] = 1;
                p0[4] = -1;

                auto audio1 = Audio::create(info, 5);
                T* p1 = reinterpret_cast<T*>(audio1->getData());
                p1[0] = 0;
                p1[1] = 1;
                p1[2] = -1;
                p1[3] = -1;
                p1[4] = 1;

                auto out = mix({ audio0, audio1 }, 1.0);
                const T* outP = reinterpret_cast<T*>(out->getData());
                FEATHER_TK_ASSERT(0 == outP[0]);
                FEATHER_TK_ASSERT(2 == outP[1]);
                FEATHER_TK_ASSERT(-2 == outP[2]);
                FEATHER_TK_ASSERT(0 == outP[3]);
                FEATHER_TK_ASSERT(0 == outP[4]);
            }
        }

        void AudioTest::_mix()
        {
            _mixI<DataType::S8, int8_t>();
            _mixI<DataType::S16, int16_t>();
            _mixI<DataType::S32, int32_t>();
            _mixF<DataType::F32, float>();
            _mixF<DataType::F64, double>();
            {
                const Info info(2, DataType::F32, 48000);

                auto audio0 = Audio::create(info, 5);
                F32_T* p0 = reinterpret_cast<F32_T*>(audio0->getData());
                p0[0] = 1; p0[1] = 1;
                p0[2] = 1; p0[3] = 1;
                p0[4] = 1; p0[5] = 1;
                p0[6] = 1; p0[7] = 1;
                p0[8] = 1; p0[9] = 1;

                auto audio1 = Audio::create(info, 5);
                F32_T* p1 = reinterpret_cast<F32_T*>(audio1->getData());
                p1[0] = 1; p1[1] = 1;
                p1[2] = 1; p1[3] = 1;
                p1[4] = 1; p1[5] = 1;
                p1[6] = 1; p1[7] = 1;
                p1[8] = 1; p1[9] = 1;

                auto out = mix({ audio0, audio1 }, 1.0, { false, false });
                const F32_T* outP = reinterpret_cast<F32_T*>(out->getData());
                FEATHER_TK_ASSERT(2 == outP[0]); FEATHER_TK_ASSERT(2 == outP[1]);
                FEATHER_TK_ASSERT(2 == outP[2]); FEATHER_TK_ASSERT(2 == outP[3]);
                FEATHER_TK_ASSERT(2 == outP[4]); FEATHER_TK_ASSERT(2 == outP[5]);
                FEATHER_TK_ASSERT(2 == outP[6]); FEATHER_TK_ASSERT(2 == outP[7]);
                FEATHER_TK_ASSERT(2 == outP[8]); FEATHER_TK_ASSERT(2 == outP[9]);

                out = mix({ audio0, audio1 }, 1.0, { true, false });
                outP = reinterpret_cast<F32_T*>(out->getData());
                FEATHER_TK_ASSERT(0 == outP[0]); FEATHER_TK_ASSERT(2 == outP[1]);
                FEATHER_TK_ASSERT(0 == outP[2]); FEATHER_TK_ASSERT(2 == outP[3]);
                FEATHER_TK_ASSERT(0 == outP[4]); FEATHER_TK_ASSERT(2 == outP[5]);
                FEATHER_TK_ASSERT(0 == outP[6]); FEATHER_TK_ASSERT(2 == outP[7]);
                FEATHER_TK_ASSERT(0 == outP[8]); FEATHER_TK_ASSERT(2 == outP[9]);

                out = mix({ audio0, audio1 }, 1.0, { false, true });
                outP = reinterpret_cast<F32_T*>(out->getData());
                FEATHER_TK_ASSERT(2 == outP[0]); FEATHER_TK_ASSERT(0 == outP[1]);
                FEATHER_TK_ASSERT(2 == outP[2]); FEATHER_TK_ASSERT(0 == outP[3]);
                FEATHER_TK_ASSERT(2 == outP[4]); FEATHER_TK_ASSERT(0 == outP[5]);
                FEATHER_TK_ASSERT(2 == outP[6]); FEATHER_TK_ASSERT(0 == outP[7]);
                FEATHER_TK_ASSERT(2 == outP[8]); FEATHER_TK_ASSERT(0 == outP[9]);

                out = mix({ audio0, audio1 }, 1.0, { true, true });
                outP = reinterpret_cast<F32_T*>(out->getData());
                FEATHER_TK_ASSERT(0 == outP[0]); FEATHER_TK_ASSERT(0 == outP[1]);
                FEATHER_TK_ASSERT(0 == outP[2]); FEATHER_TK_ASSERT(0 == outP[3]);
                FEATHER_TK_ASSERT(0 == outP[4]); FEATHER_TK_ASSERT(0 == outP[5]);
                FEATHER_TK_ASSERT(0 == outP[6]); FEATHER_TK_ASSERT(0 == outP[7]);
                FEATHER_TK_ASSERT(0 == outP[8]); FEATHER_TK_ASSERT(0 == outP[9]);
            }
        }

        void AudioTest::_reverse()
        {
            auto audio = Audio::create(Info(1, DataType::S8, 41000), 3);
            audio->getData()[0] = 1;
            audio->getData()[1] = 2;
            audio->getData()[2] = 3;
            auto reversed = reverse(audio);
            FEATHER_TK_ASSERT(3 == reversed->getData()[0]);
            FEATHER_TK_ASSERT(2 == reversed->getData()[1]);
            FEATHER_TK_ASSERT(1 == reversed->getData()[2]);
        }

        void AudioTest::_convert()
        {
            for (auto i : getDataTypeEnums())
            {
                const auto in = Audio::create(Info(1, i, 44100), 1);
                in->zero();
                for (auto j : getDataTypeEnums())
                {
                    const auto out = convert(in, j);
                    FEATHER_TK_ASSERT(out->getChannelCount() == in->getChannelCount());
                    FEATHER_TK_ASSERT(out->getDataType() == j);
                    FEATHER_TK_ASSERT(out->getSampleRate() == in->getSampleRate());
                    FEATHER_TK_ASSERT(out->getSampleCount() == in->getSampleCount());
                }
            }
        }

        void AudioTest::_move()
        {
            {
                const Info info(2, DataType::S16, 10);

                std::vector<uint8_t> data(10 * info.getByteCount(), 0);

                std::list<std::shared_ptr<Audio> > list;
                for (size_t i = 0; i < 10; ++i)
                {
                    auto item = Audio::create(info, 1);
                    reinterpret_cast<audio::S16_T*>(item->getData())[0] = i;
                    reinterpret_cast<audio::S16_T*>(item->getData())[1] = i;
                    list.push_back(item);
                }

                move(list, data.data(), 10);

                FEATHER_TK_ASSERT(list.empty());
                FEATHER_TK_ASSERT(0 == getSampleCount(list));
                audio::S16_T* p = reinterpret_cast<audio::S16_T*>(data.data());
                for (size_t i = 0; i < 10; ++i)
                {
                    FEATHER_TK_ASSERT(i == p[i * 2]);
                    FEATHER_TK_ASSERT(i == p[i * 2 + 1]);
                }
            }
            {
                const Info info(2, DataType::S16, 10);

                std::vector<uint8_t> data(10 * info.getByteCount(), 0);

                std::list<std::shared_ptr<Audio> > list;
                for (size_t i = 0; i < 5; ++i)
                {
                    auto item = Audio::create(info, 1);
                    reinterpret_cast<audio::S16_T*>(item->getData())[0] = i;
                    reinterpret_cast<audio::S16_T*>(item->getData())[1] = i;
                    list.push_back(item);
                }

                move(list, data.data(), 10);

                FEATHER_TK_ASSERT(list.empty());
                audio::S16_T* p = reinterpret_cast<audio::S16_T*>(data.data());
                size_t i = 0;
                for (; i < 5; ++i)
                {
                    FEATHER_TK_ASSERT(i == p[i * 2]);
                    FEATHER_TK_ASSERT(i == p[i * 2 + 1]);
                }
                for (; i < 10; ++i)
                {
                    FEATHER_TK_ASSERT(0 == p[i * 2]);
                    FEATHER_TK_ASSERT(0 == p[i * 2 + 1]);
                }
            }
            {
                const Info info(2, DataType::S16, 10);

                std::vector<uint8_t> data(10 * info.getByteCount(), 0);

                std::list<std::shared_ptr<Audio> > list;
                for (size_t i = 0; i < 15; ++i)
                {
                    auto item = Audio::create(info, 1);
                    reinterpret_cast<audio::S16_T*>(item->getData())[0] = i;
                    reinterpret_cast<audio::S16_T*>(item->getData())[1] = i;
                    list.push_back(item);
                }

                move(list, data.data(), 10);

                FEATHER_TK_ASSERT(5 == list.size());
                FEATHER_TK_ASSERT(5 == getSampleCount(list));
                audio::S16_T* p = reinterpret_cast<audio::S16_T*>(data.data());
                for (size_t i = 0; i < 10; ++i)
                {
                    FEATHER_TK_ASSERT(i == p[i * 2]);
                    FEATHER_TK_ASSERT(i == p[i * 2 + 1]);
                }
            }
            {
                const Info info(2, DataType::S16, 10);

                std::vector<uint8_t> data(10 * info.getByteCount(), 0);

                std::list<std::shared_ptr<Audio> > list;
                for (size_t i = 0; i < 4; ++i)
                {
                    auto item = Audio::create(info, 4);
                    for (size_t j = 0; j < 4; ++j)
                    {
                        reinterpret_cast<audio::S16_T*>(item->getData())[j * 2] = i * 4 + j;
                        reinterpret_cast<audio::S16_T*>(item->getData())[j * 2 + 1] = i * 4 + j;
                    }
                    list.push_back(item);
                }

                move(list, data.data(), 10);

                FEATHER_TK_ASSERT(2 == list.size());
                FEATHER_TK_ASSERT(6 == getSampleCount(list));
                FEATHER_TK_ASSERT(2 == list.front()->getSampleCount());
                FEATHER_TK_ASSERT(10 == reinterpret_cast<audio::S16_T*>(list.front()->getData())[0]);
                FEATHER_TK_ASSERT(11 == reinterpret_cast<audio::S16_T*>(list.front()->getData())[2]);
                audio::S16_T* p = reinterpret_cast<audio::S16_T*>(data.data());
                for (size_t i = 0; i < 10; ++i)
                {
                    FEATHER_TK_ASSERT(i == p[i * 2]);
                    FEATHER_TK_ASSERT(i == p[i * 2 + 1]);
                }
            }
        }

        void AudioTest::_resample()
        {
            for (auto dataType :
                {
                    DataType::S16,
                    DataType::S32,
                    DataType::F32,
                    DataType::F64,
                    DataType::None
                })
            {
                const Info a(2, dataType, 44100);
                const Info b(1, dataType, 44100);
                auto r = AudioResample::create(a, b);
                FEATHER_TK_ASSERT(a == r->getInputInfo());
                FEATHER_TK_ASSERT(b == r->getOutputInfo());
                auto in = Audio::create(a, 44100);
                auto out = r->process(in);
#if defined(TLRENDER_FFMPEG)
                if (dataType != DataType::None)
                {
                    FEATHER_TK_ASSERT(b == out->getInfo());
                    FEATHER_TK_ASSERT(44100 == out->getSampleCount());
                }
#endif // TLRENDER_FFMPEG
                r->flush();
            }
        }
    }
}
