// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Timeline/IRender.h>
#include <tlRender/Timeline/Player.h>
#include <tlRender/Timeline/TimeUnits.h>

#include <tlRender/Core/AudioSystem.h>

#include <ftk/Core/RenderOptions.h>

#include <QMetaType>

Q_DECLARE_METATYPE(ftk::AlphaBlend);
Q_DECLARE_METATYPE(ftk::ChannelDisplay);
Q_DECLARE_METATYPE(ftk::ImageType);
Q_DECLARE_METATYPE(ftk::ImageFilter);
Q_DECLARE_METATYPE(ftk::InputVideoLevels);
Q_DECLARE_METATYPE(ftk::Size2I);

Q_DECLARE_METATYPE(tl::AudioType);

Q_DECLARE_METATYPE(tl::FileType);

Q_DECLARE_METATYPE(tl::timeline::Compare);
Q_DECLARE_METATYPE(tl::timeline::CompareTime);
Q_DECLARE_METATYPE(tl::timeline::ImageSeqAudio);
Q_DECLARE_METATYPE(tl::timeline::Loop);
Q_DECLARE_METATYPE(tl::timeline::Playback);
Q_DECLARE_METATYPE(tl::timeline::TimeAction);
Q_DECLARE_METATYPE(tl::timeline::TimeUnits);
Q_DECLARE_METATYPE(tl::timeline::Transition);
