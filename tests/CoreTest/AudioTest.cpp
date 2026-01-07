// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <CoreTest/AudioTest.h>

#include <tlRender/Core/AudioResample.h>
#include <tlRender/Core/AudioSystem.h>

#include <cstring>

namespace tl
{
    namespace core_tests
    {
        AudioTest::AudioTest(const std::shared_ptr<ftk::Context>& context) :
            ITest(context, "core_tests::AudioTest")
        {}

        std::shared_ptr<AudioTest> AudioTest::create(const std::shared_ptr<ftk::Context>& context)
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
            _enum<AudioType>("AudioType", getAudioTypeEnums);
        }

        void AudioTest::_types()
        {
            for (auto i : getAudioTypeEnums())
            {
                std::stringstream ss;
                ss << i << " byte count: " << getByteCount(i);
                _print(ss.str());
            }
            for (auto i : { 0, 1, 2, 3, 4, 5, 6, 7, 8 })
            {
                std::stringstream ss;
                ss << i << " bytes int type: " << getIntAudioType(i);
                _print(ss.str());
            }
            for (auto i : { 0, 1, 2, 3, 4, 5, 6, 7, 8 })
            {
                std::stringstream ss;
                ss << i << " bytes float type: " << getFloatAudioType(i);
                _print(ss.str());
            }
        }

        void AudioTest::_audio()
        {
            {
                const AudioInfo info(2, AudioType::S16, 44100);
                FTK_ASSERT(info == info);
                FTK_ASSERT(info != AudioInfo());
                auto audio = Audio::create(info, 1000);
                audio->zero();
                FTK_ASSERT(audio->getInfo() == info);
                FTK_ASSERT(audio->getChannelCount() == info.channelCount);
                FTK_ASSERT(audio->getType() == info.type);
                FTK_ASSERT(audio->getSampleRate() == info.sampleRate);
                FTK_ASSERT(audio->getSampleCount() == 1000);
                FTK_ASSERT(audio->isValid());
                FTK_ASSERT(audio->getData());
                FTK_ASSERT(static_cast<const Audio*>(audio.get())->getData());
            }
        }

        void AudioTest::_audioSystem()
        {
            auto system = _context->getSystem<AudioSystem>();
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
                const AudioDeviceInfo device = system->getDefaultDevice();
                ss << "default device: " << device.id.number << " " << device.id.name;
                _print(ss.str());
            }
        }

        void AudioTest::_combine()
        {
            std::list<std::shared_ptr<Audio> > list;
            auto audio = Audio::create(AudioInfo(1, AudioType::S8, 41000), 1);
            audio->getData()[0] = 1;
            list.push_back(audio);
            audio = Audio::create(AudioInfo(1, AudioType::S8, 41000), 1);
            audio->getData()[0] = 2;
            list.push_back(audio);
            audio = Audio::create(AudioInfo(1, AudioType::S8, 41000), 1);
            audio->getData()[0] = 3;
            list.push_back(audio);
            auto combined = combineAudio(list);
            FTK_ASSERT(3 == combined->getSampleCount());
            FTK_ASSERT(1 == combined->getData()[0]);
            FTK_ASSERT(2 == combined->getData()[1]);
            FTK_ASSERT(3 == combined->getData()[2]);
        }

        namespace
        {
            template<AudioType DT, typename T>
            void _mixI()
            {
                const AudioInfo info(1, DT, 48000);

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

                auto out = mixAudio({ audio0, audio1 }, 1.0);
                const T* outP = reinterpret_cast<T*>(out->getData());
                FTK_ASSERT(0 == outP[0]);
                FTK_ASSERT(std::numeric_limits<T>::max() == outP[1]);
                FTK_ASSERT(std::numeric_limits<T>::min() == outP[2]);
                FTK_ASSERT(std::numeric_limits<T>::max() + std::numeric_limits<T>::min() == outP[3]);
                FTK_ASSERT(std::numeric_limits<T>::max() + std::numeric_limits<T>::min() == outP[4]);
            }

            template<AudioType DT, typename T>
            void _mixF()
            {
                const AudioInfo info(1, DT, 48000);

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

                auto out = mixAudio({ audio0, audio1 }, 1.0);
                const T* outP = reinterpret_cast<T*>(out->getData());
                FTK_ASSERT(0 == outP[0]);
                FTK_ASSERT(2 == outP[1]);
                FTK_ASSERT(-2 == outP[2]);
                FTK_ASSERT(0 == outP[3]);
                FTK_ASSERT(0 == outP[4]);
            }
        }

        void AudioTest::_mix()
        {
            _mixI<AudioType::S8, int8_t>();
            _mixI<AudioType::S16, int16_t>();
            _mixI<AudioType::S32, int32_t>();
            _mixF<AudioType::F32, float>();
            _mixF<AudioType::F64, double>();
            {
                const AudioInfo info(2, AudioType::F32, 48000);

                auto audio0 = Audio::create(info, 5);
                float* p0 = reinterpret_cast<float*>(audio0->getData());
                p0[0] = 1; p0[1] = 1;
                p0[2] = 1; p0[3] = 1;
                p0[4] = 1; p0[5] = 1;
                p0[6] = 1; p0[7] = 1;
                p0[8] = 1; p0[9] = 1;

                auto audio1 = Audio::create(info, 5);
                float* p1 = reinterpret_cast<float*>(audio1->getData());
                p1[0] = 1; p1[1] = 1;
                p1[2] = 1; p1[3] = 1;
                p1[4] = 1; p1[5] = 1;
                p1[6] = 1; p1[7] = 1;
                p1[8] = 1; p1[9] = 1;

                auto out = mixAudio({ audio0, audio1 }, 1.0, { false, false });
                const float* outP = reinterpret_cast<float*>(out->getData());
                FTK_ASSERT(2 == outP[0]); FTK_ASSERT(2 == outP[1]);
                FTK_ASSERT(2 == outP[2]); FTK_ASSERT(2 == outP[3]);
                FTK_ASSERT(2 == outP[4]); FTK_ASSERT(2 == outP[5]);
                FTK_ASSERT(2 == outP[6]); FTK_ASSERT(2 == outP[7]);
                FTK_ASSERT(2 == outP[8]); FTK_ASSERT(2 == outP[9]);

                out = mixAudio({ audio0, audio1 }, 1.0, { true, false });
                outP = reinterpret_cast<float*>(out->getData());
                FTK_ASSERT(0 == outP[0]); FTK_ASSERT(2 == outP[1]);
                FTK_ASSERT(0 == outP[2]); FTK_ASSERT(2 == outP[3]);
                FTK_ASSERT(0 == outP[4]); FTK_ASSERT(2 == outP[5]);
                FTK_ASSERT(0 == outP[6]); FTK_ASSERT(2 == outP[7]);
                FTK_ASSERT(0 == outP[8]); FTK_ASSERT(2 == outP[9]);

                out = mixAudio({ audio0, audio1 }, 1.0, { false, true });
                outP = reinterpret_cast<float*>(out->getData());
                FTK_ASSERT(2 == outP[0]); FTK_ASSERT(0 == outP[1]);
                FTK_ASSERT(2 == outP[2]); FTK_ASSERT(0 == outP[3]);
                FTK_ASSERT(2 == outP[4]); FTK_ASSERT(0 == outP[5]);
                FTK_ASSERT(2 == outP[6]); FTK_ASSERT(0 == outP[7]);
                FTK_ASSERT(2 == outP[8]); FTK_ASSERT(0 == outP[9]);

                out = mixAudio({ audio0, audio1 }, 1.0, { true, true });
                outP = reinterpret_cast<float*>(out->getData());
                FTK_ASSERT(0 == outP[0]); FTK_ASSERT(0 == outP[1]);
                FTK_ASSERT(0 == outP[2]); FTK_ASSERT(0 == outP[3]);
                FTK_ASSERT(0 == outP[4]); FTK_ASSERT(0 == outP[5]);
                FTK_ASSERT(0 == outP[6]); FTK_ASSERT(0 == outP[7]);
                FTK_ASSERT(0 == outP[8]); FTK_ASSERT(0 == outP[9]);
            }
        }

        void AudioTest::_reverse()
        {
            auto audio = Audio::create(AudioInfo(1, AudioType::S8, 41000), 3);
            audio->getData()[0] = 1;
            audio->getData()[1] = 2;
            audio->getData()[2] = 3;
            auto reversed = reverseAudio(audio);
            FTK_ASSERT(3 == reversed->getData()[0]);
            FTK_ASSERT(2 == reversed->getData()[1]);
            FTK_ASSERT(1 == reversed->getData()[2]);
        }

        void AudioTest::_convert()
        {
            for (auto i : getAudioTypeEnums())
            {
                const auto in = Audio::create(AudioInfo(1, i, 44100), 1);
                in->zero();
                for (auto j : getAudioTypeEnums())
                {
                    const auto out = convertAudio(in, j);
                    FTK_ASSERT(out->getChannelCount() == in->getChannelCount());
                    FTK_ASSERT(out->getType() == j);
                    FTK_ASSERT(out->getSampleRate() == in->getSampleRate());
                    FTK_ASSERT(out->getSampleCount() == in->getSampleCount());
                }
            }
        }

        void AudioTest::_move()
        {
            {
                const AudioInfo info(2, AudioType::S16, 10);

                std::vector<uint8_t> data(10 * info.getByteCount(), 0);

                std::list<std::shared_ptr<Audio> > list;
                for (size_t i = 0; i < 10; ++i)
                {
                    auto item = Audio::create(info, 1);
                    reinterpret_cast<uint16_t*>(item->getData())[0] = i;
                    reinterpret_cast<uint16_t*>(item->getData())[1] = i;
                    list.push_back(item);
                }

                moveAudio(list, data.data(), 10);

                FTK_ASSERT(list.empty());
                FTK_ASSERT(0 == getSampleCount(list));
                uint16_t* p = reinterpret_cast<uint16_t*>(data.data());
                for (size_t i = 0; i < 10; ++i)
                {
                    FTK_ASSERT(i == p[i * 2]);
                    FTK_ASSERT(i == p[i * 2 + 1]);
                }
            }
            {
                const AudioInfo info(2, AudioType::S16, 10);

                std::vector<uint8_t> data(10 * info.getByteCount(), 0);

                std::list<std::shared_ptr<Audio> > list;
                for (size_t i = 0; i < 5; ++i)
                {
                    auto item = Audio::create(info, 1);
                    reinterpret_cast<uint16_t*>(item->getData())[0] = i;
                    reinterpret_cast<uint16_t*>(item->getData())[1] = i;
                    list.push_back(item);
                }

                moveAudio(list, data.data(), 10);

                FTK_ASSERT(list.empty());
                uint16_t* p = reinterpret_cast<uint16_t*>(data.data());
                size_t i = 0;
                for (; i < 5; ++i)
                {
                    FTK_ASSERT(i == p[i * 2]);
                    FTK_ASSERT(i == p[i * 2 + 1]);
                }
                for (; i < 10; ++i)
                {
                    FTK_ASSERT(0 == p[i * 2]);
                    FTK_ASSERT(0 == p[i * 2 + 1]);
                }
            }
            {
                const AudioInfo info(2, AudioType::S16, 10);

                std::vector<uint8_t> data(10 * info.getByteCount(), 0);

                std::list<std::shared_ptr<Audio> > list;
                for (size_t i = 0; i < 15; ++i)
                {
                    auto item = Audio::create(info, 1);
                    reinterpret_cast<uint16_t*>(item->getData())[0] = i;
                    reinterpret_cast<uint16_t*>(item->getData())[1] = i;
                    list.push_back(item);
                }

                moveAudio(list, data.data(), 10);

                FTK_ASSERT(5 == list.size());
                FTK_ASSERT(5 == getSampleCount(list));
                uint16_t* p = reinterpret_cast<uint16_t*>(data.data());
                for (size_t i = 0; i < 10; ++i)
                {
                    FTK_ASSERT(i == p[i * 2]);
                    FTK_ASSERT(i == p[i * 2 + 1]);
                }
            }
            {
                const AudioInfo info(2, AudioType::S16, 10);

                std::vector<uint8_t> data(10 * info.getByteCount(), 0);

                std::list<std::shared_ptr<Audio> > list;
                for (size_t i = 0; i < 4; ++i)
                {
                    auto item = Audio::create(info, 4);
                    for (size_t j = 0; j < 4; ++j)
                    {
                        reinterpret_cast<uint16_t*>(item->getData())[j * 2] = i * 4 + j;
                        reinterpret_cast<uint16_t*>(item->getData())[j * 2 + 1] = i * 4 + j;
                    }
                    list.push_back(item);
                }

                moveAudio(list, data.data(), 10);

                FTK_ASSERT(2 == list.size());
                FTK_ASSERT(6 == getSampleCount(list));
                FTK_ASSERT(2 == list.front()->getSampleCount());
                FTK_ASSERT(10 == reinterpret_cast<uint16_t*>(list.front()->getData())[0]);
                FTK_ASSERT(11 == reinterpret_cast<uint16_t*>(list.front()->getData())[2]);
                uint16_t* p = reinterpret_cast<uint16_t*>(data.data());
                for (size_t i = 0; i < 10; ++i)
                {
                    FTK_ASSERT(i == p[i * 2]);
                    FTK_ASSERT(i == p[i * 2 + 1]);
                }
            }
        }

        void AudioTest::_resample()
        {
            for (auto audioType :
                {
                    AudioType::S16,
                    AudioType::S32,
                    AudioType::F32,
                    AudioType::F64,
                    AudioType::None
                })
            {
                const AudioInfo a(2, audioType, 44100);
                const AudioInfo b(1, audioType, 44100);
                auto r = AudioResample::create(a, b);
                FTK_ASSERT(a == r->getInputInfo());
                FTK_ASSERT(b == r->getOutputInfo());
                auto in = Audio::create(a, 44100);
                auto out = r->process(in);
#if defined(TLRENDER_FFMPEG)
                if (audioType != AudioType::None)
                {
                    FTK_ASSERT(b == out->getInfo());
                    FTK_ASSERT(44100 == out->getSampleCount());
                }
#endif // TLRENDER_FFMPEG
                r->flush();
            }
        }
    }
}
