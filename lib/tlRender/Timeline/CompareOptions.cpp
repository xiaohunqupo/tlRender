// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/Timeline/CompareOptions.h>

#include <tlRender/Timeline/DisplayOptions.h>

#include <ftk/Core/Error.h>
#include <ftk/Core/String.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <sstream>

namespace tl
{
    TL_ENUM_IMPL(
        Compare,
        "A",
        "B",
        "Wipe",
        "Overlay",
        "Difference",
        "Horizontal",
        "Vertical",
        "Tile");

    TL_ENUM_IMPL(
        CompareTime,
        "Relative",
        "Absolute");

    bool CompareOptions::operator == (const CompareOptions& other) const
    {
        return
            compare == other.compare &&
            wipeCenter == other.wipeCenter &&
            wipeRotation == other.wipeRotation &&
            overlay == other.overlay &&
            fitToA == other.fitToA;
    }

    bool CompareOptions::operator != (const CompareOptions& other) const
    {
        return !(*this == other);
    }

    std::vector<ftk::Box2I> getBoxes(
        const CompareOptions& options,
        const AspectRatioOptions& aspectRatioOptions,
        const std::vector<ftk::ImageInfo>& infos)
    {
        std::vector<ftk::Box2I> out;
        const size_t count = infos.size();
        switch (options.compare)
        {
        case Compare::Horizontal:
        {
            if (options.fitToA)
            {
                ftk::Size2I size;
                if (count > 0)
                {
                    size = getRenderSize(infos[0], aspectRatioOptions);
                    out.push_back(ftk::Box2I(0, 0, size.w, size.h));
                }
                if (count > 1)
                {
                    out.push_back(getBox(
                        ftk::Box2I(size.w, 0, size.w, size.h),
                        infos[1],
                        aspectRatioOptions,
                        BoxHAlign::Left));
                }
            }
            else
            {
                ftk::Size2I size;
                if (count > 0)
                {
                    size = getRenderSize(infos[0], aspectRatioOptions);
                    out.push_back(ftk::Box2I(0, 0, size.w, size.h));
                }
                if (count > 1)
                {
                    const ftk::Size2I sizeB = getRenderSize(infos[1], aspectRatioOptions);
                    out.push_back(ftk::Box2I(size.w, 0, sizeB.w, sizeB.h));
                }
            }
            break;
        }
        case Compare::Vertical:
        {
            if (options.fitToA)
            {
                ftk::Size2I size;
                if (count > 0)
                {
                    size = getRenderSize(infos[0], aspectRatioOptions);
                    out.push_back(ftk::Box2I(0, 0, size.w, size.h));
                }
                if (count > 1)
                {
                    out.push_back(getBox(
                        ftk::Box2I(0, size.h, size.w, size.h),
                        infos[1],
                        aspectRatioOptions,
                        BoxHAlign::Center,
                        BoxVAlign::Top));
                }
            }
            else
            {
                ftk::Size2I size;
                if (count > 0)
                {
                    size = getRenderSize(infos[0], aspectRatioOptions);
                    out.push_back(ftk::Box2I(0, 0, size.w, size.h));
                }
                if (count > 1)
                {
                    const ftk::Size2I sizeB = getRenderSize(infos[1], aspectRatioOptions);
                    out.push_back(ftk::Box2I(0, size.h, sizeB.w, sizeB.h));
                }
            }
            break;
        }
        case Compare::Tile:
            if (count > 0)
            {
                const int cols = std::max(1, static_cast<int>(std::sqrt(count)));
                if (options.fitToA)
                {
                    const ftk::Size2I size = getRenderSize(infos[0], aspectRatioOptions);
                    int c = 0;
                    int x = 0;
                    int y = 0;
                    for (size_t i = 0; i < count; ++i, ++c)
                    {
                        out.push_back(getBox(
                            ftk::Box2I(x, y, size.w, size.h),
                            infos[i],
                            aspectRatioOptions));
                        if (c == cols)
                        {
                            c = 0;
                            x = 0;
                            y += size.h;
                        }
                        else
                        {
                            x += size.w;
                        }
                    }
                }
                else
                {
                    ftk::Size2I size;
                    for (size_t i = 0; i < count; ++i)
                    {
                        const ftk::Size2I size2 = getRenderSize(infos[i], aspectRatioOptions);
                        size.w = std::max(size.w, size2.w);
                        size.h = std::max(size.h, size2.h);
                    }
                    int c = 0;
                    int x = 0;
                    int y = 0;
                    for (size_t i = 0; i < count; ++i, ++c)
                    {
                        const ftk::Size2I size2 = getRenderSize(infos[i], aspectRatioOptions);
                        out.push_back(ftk::Box2I(
                            x + size.w / 2 - size2.w / 2,
                            y + size.h / 2 - size2.h / 2,
                            size2.w,
                            size2.h));
                        if (c == cols)
                        {
                            c = 0;
                            x = 0;
                            y += size.h;
                        }
                        else
                        {
                            x += size.w;
                        }
                    }
                }
            }
            break;
        default:
            if (count > 0)
            {
                if (options.fitToA)
                {
                    const ftk::Size2I size = getRenderSize(infos[0], aspectRatioOptions);
                    out.push_back(ftk::Box2I(0, 0, size.w, size.h));
                    for (size_t i = 1; i < count; ++i)
                    {
                        out.push_back(getBox(
                            ftk::Box2I(0, 0, size.w, size.h),
                            infos[i],
                            aspectRatioOptions));
                    }
                }
                else
                {
                    for (size_t i = 0; i < count; ++i)
                    {
                        const ftk::Size2I size = getRenderSize(infos[i], aspectRatioOptions);
                        out.push_back(ftk::Box2I(0, 0, size.w, size.h));
                    }
                }
            }
            break;
        }
        return out;
    }

    std::vector<ftk::Box2I> getBoxes(
        const CompareOptions& options,
        const AspectRatioOptions& aspectRatioOptions,
        const std::vector<VideoFrame>& videoFrame)
    {
        std::vector<ftk::ImageInfo> infos;
        for (const auto& i : videoFrame)
        {
            ftk::ImageInfo info;
            for (const auto& layer : i.layers)
            {
                if (layer.image)
                {
                    info = layer.image->getInfo();
                    break;
                }
                else if (layer.imageB)
                {
                    info = layer.imageB->getInfo();
                    break;
                }
            }
            infos.push_back(info);
        }
        return getBoxes(options, aspectRatioOptions, infos);
    }

    ftk::Size2I getRenderSize(
        const CompareOptions& options,
        const AspectRatioOptions& aspectRatioOptions,
        const std::vector<ftk::ImageInfo>& infos)
    {
        ftk::Size2I out;
        ftk::Box2I box;
        const auto boxes = getBoxes(options, aspectRatioOptions, infos);
        if (!boxes.empty())
        {
            box = boxes[0];
            for (size_t i = 1; i < boxes.size(); ++i)
            {
                box = ftk::expand(box, boxes[i]);
            }
            out.w = box.w();
            out.h = box.h();
        }
        return out;
    }

    ftk::Size2I getRenderSize(
        const CompareOptions& options,
        const AspectRatioOptions& aspectRatioOptions,
        const std::vector<VideoFrame>& videoFrame)
    {
        std::vector<ftk::ImageInfo> infos;
        for (const auto& i : videoFrame)
        {
            ftk::ImageInfo info;
            for (const auto& layer : i.layers)
            {
                if (layer.image)
                {
                    info = layer.image->getInfo();
                    break;
                }
                else if (layer.imageB)
                {
                    info = layer.imageB->getInfo();
                    break;
                }
            }
            infos.push_back(info);
        }
        return getRenderSize(options, aspectRatioOptions, infos);
    }

    OTIO_NS::RationalTime getCompareTime(
        const OTIO_NS::RationalTime& sourceTime,
        const OTIO_NS::TimeRange& sourceTimeRange,
        const OTIO_NS::TimeRange& compareTimeRange,
        CompareTime compare)
    {
        OTIO_NS::RationalTime out;
        switch (compare)
        {
        case CompareTime::Relative:
        {
            const OTIO_NS::RationalTime relativeTime =
                sourceTime - sourceTimeRange.start_time();
            const OTIO_NS::RationalTime relativeTimeRescaled = relativeTime.
                rescaled_to(compareTimeRange.duration().rate()).
                floor();
            out = compareTimeRange.start_time() + relativeTimeRescaled;
            break;
        }
        case CompareTime::Absolute:
            out = sourceTime.
                rescaled_to(compareTimeRange.duration().rate()).
                floor();
            break;
        default: break;
        }
        return out;
    }

    void to_json(nlohmann::json& json, const CompareOptions& in)
    {
        json["Compare"] = to_string(in.compare);
        json["WipeCenter"] = in.wipeCenter;
        json["WipeRotation"] = in.wipeRotation;
        json["Overlay"] = in.overlay;
        json["FitToA"] = in.fitToA;
    }

    void from_json(const nlohmann::json& json, CompareOptions& out)
    {
        from_string(json.at("Compare").get<std::string>(), out.compare);
        json.at("WipeCenter").get_to(out.wipeCenter);
        json.at("WipeRotation").get_to(out.wipeRotation);
        json.at("Overlay").get_to(out.overlay);
        json.at("FitToA").get_to(out.fitToA);
    }
}
