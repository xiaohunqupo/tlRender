// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/Qt/Init.h>

#include <tlRender/Qt/MetaTypes.h>
#include <tlRender/Qt/TimeObject.h>

#include <tlRender/Timeline/Init.h>

#include <ftk/Core/Context.h>

#include <QSurfaceFormat>

namespace tl
{
    namespace qt
    {
        void init(
            const std::shared_ptr<ftk::Context>& context,
            DefaultSurfaceFormat defaultSurfaceFormat)
        {
            tl::init(context);
            System::create(context, defaultSurfaceFormat);
        }

        System::System(
            const std::shared_ptr<ftk::Context>& context,
            DefaultSurfaceFormat defaultSurfaceFormat) :
            ISystem(context, "tl::qt::System")
        {
            qRegisterMetaType<OTIO_NS::RationalTime>("OTIO_NS::RationalTime");
            qRegisterMetaType<OTIO_NS::TimeRange>("OTIO_NS::TimeRange");
            qRegisterMetaType<std::vector<OTIO_NS::TimeRange> >("std::vector<OTIO_NS::TimeRange>");

            qRegisterMetaType<ftk::AlphaBlend>("ftk::AlphaBlend");
            qRegisterMetaType<ftk::ChannelDisplay>("ftk::ChannelDisplay");
            qRegisterMetaType<ftk::ImageType>("ftk::ImageType");
            qRegisterMetaType<ftk::ImageFilter>("ftk::ImageFilter");
            qRegisterMetaType<ftk::InputVideoLevels>("ftk::InputVideoLevels");
            qRegisterMetaType<ftk::PathOptions>("ftk::PathOptions");
            qRegisterMetaType<ftk::Size2I>("ftk::Size2I");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QMetaType::registerComparators<ftk::AlphaBlend>();
            QMetaType::registerComparators<ftk::ChannelDisplay>();
            QMetaType::registerComparators<ftk::ImageType>();
            QMetaType::registerComparators<ftk::ImageFilter>();
            QMetaType::registerComparators<ftk::InputVideoLevels>();
#endif // QT_VERSION

            qRegisterMetaType<AudioType>("tl::AudioType");
            qRegisterMetaType<AudioDeviceID>("tl::AudioDeviceID");
            qRegisterMetaType<AudioDeviceInfo>("tl::AudioDeviceInfo");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QMetaType::registerComparators<AudioType>();
#endif // QT_VERSION

            qRegisterMetaType<FileType>("tl::FileType");
            qRegisterMetaType<IOInfo>("tl::IOInfo");
            qRegisterMetaType<VideoData>("tl::VideoData");
            qRegisterMetaType<AudioData>("tl::AudioData");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QMetaType::registerComparators<io::FileType>();
#endif // QT_VERSION

            qRegisterMetaType<AudioFrame>("tl::AudioFrame");
            qRegisterMetaType<AudioLayer>("tl::AudioLayer");
            qRegisterMetaType<Color>("tl::Color");
            qRegisterMetaType<Compare>("tl::Compare");
            qRegisterMetaType<CompareTime>("tl::CompareTime");
            qRegisterMetaType<CompareOptions>("tl::CompareOptions");
            qRegisterMetaType<EXRDisplay>("tl::EXRDisplay");
            qRegisterMetaType<ImageSeqAudio>("tl::ImageSeqAudio");
            qRegisterMetaType<LUTOptions>("tl::LUTOptions");
            qRegisterMetaType<Levels>("tl::Levels");
            qRegisterMetaType<Loop>("tl::Loop");
            qRegisterMetaType<OCIOOptions>("tl::OCIOOptions");
            qRegisterMetaType<Options>("tl::Options");
            qRegisterMetaType<Playback>("tl::Playback");
            qRegisterMetaType<PlayerCacheInfo>("tl::PlayerCacheInfo");
            qRegisterMetaType<PlayerCacheOptions>("tl::PlayerCacheOptions");
            qRegisterMetaType<PlayerOptions>("tl::PlayerOptions");
            qRegisterMetaType<TimeAction>("tl::TimeAction");
            qRegisterMetaType<TimeUnits>("tl::TimeUnits");
            qRegisterMetaType<Transition>("tl::Transition");
            qRegisterMetaType<VideoFrame>("tl::VideoFrame");
            qRegisterMetaType<VideoLayer>("tl::VideoLayer");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QMetaType::registerComparators<Compare>();
            QMetaType::registerComparators<CompareTime>();
            QMetaType::registerComparators<ImageuenceAudio>();
            QMetaType::registerComparators<Loop>();
            QMetaType::registerComparators<Playback>();
            QMetaType::registerComparators<TimeAction>();
            QMetaType::registerComparators<TimeUnits>();
            QMetaType::registerComparators<Transition>();
#endif // QT_VERSION

            switch (defaultSurfaceFormat)
            {
            case DefaultSurfaceFormat::OpenGL_4_1_CoreProfile:
            {
                QSurfaceFormat surfaceFormat;
                surfaceFormat.setMajorVersion(4);
                surfaceFormat.setMinorVersion(1);
                surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
                QSurfaceFormat::setDefaultFormat(surfaceFormat);
                break;
            }
            default: break;
            }
        }

        System::~System()
        {}

        std::shared_ptr<System> System::create(
            const std::shared_ptr<ftk::Context>& context,
            DefaultSurfaceFormat defaultSurfaceFormat)
        {
            auto out = context->getSystem<System>();
            if (!out)
            {
                out = std::shared_ptr<System>(new System(context, defaultSurfaceFormat));
                context->addSystem(out);
            }
            return out;
        }
    }
}

