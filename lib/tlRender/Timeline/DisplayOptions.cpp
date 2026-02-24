// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/Timeline/DisplayOptions.h>

#include <ftk/Core/Error.h>
#include <ftk/Core/String.h>

#include <algorithm>
#include <array>

namespace tl
{
    bool Color::operator == (const Color& other) const
    {
        return
            enabled == other.enabled &&
            add == other.add &&
            brightness == other.brightness &&
            contrast == other.contrast &&
            saturation == other.saturation &&
            hue == other.hue &&
            invert == other.invert;
    }

    bool Color::operator != (const Color& other) const
    {
        return !(*this == other);
    }

    ftk::M44F color(const Color& in)
    {
        return
            ftk::brightness(in.brightness) *
            ftk::contrast(in.contrast) *
            ftk::saturation(in.saturation) *
            ftk::hue(in.hue);
    }

    bool Levels::operator == (const Levels& other) const
    {
        return
            enabled == other.enabled &&
            inLow == other.inLow &&
            inHigh == other.inHigh &&
            gamma == other.gamma &&
            outLow == other.outLow &&
            outHigh == other.outHigh;
    }

    bool Levels::operator != (const Levels& other) const
    {
        return !(*this == other);
    }

    bool EXRDisplay::operator == (const EXRDisplay& other) const
    {
        return
            enabled == other.enabled &&
            exposure == other.exposure &&
            defog == other.defog &&
            kneeLow == other.kneeLow &&
            kneeHigh == other.kneeHigh;
    }

    bool EXRDisplay::operator != (const EXRDisplay& other) const
    {
        return !(*this == other);
    }

    bool SoftClip::operator == (const SoftClip& other) const
    {
        return
            enabled == other.enabled &&
            value == other.value;
    }

    bool SoftClip::operator != (const SoftClip& other) const
    {
        return !(*this == other);
    }

    bool DisplayOptions::operator == (const DisplayOptions& other) const
    {
        return
            channels == other.channels &&
            mirror == other.mirror &&
            color == other.color &&
            levels == other.levels &&
            exrDisplay == other.exrDisplay &&
            softClip == other.softClip &&
            imageFilters == other.imageFilters;
    }

    bool DisplayOptions::operator != (const DisplayOptions& other) const
    {
        return !(*this == other);
    }

    void to_json(nlohmann::json& json, const Color& in)
    {
        json["Enabled"] = in.enabled;
        json["Add"] = in.add;
        json["Brightness"] = in.brightness;
        json["Contrast"] = in.contrast;
        json["Saturation"] = in.saturation;
        json["Hue"] = in.hue;
        json["Invert"] = in.invert;
    }

    void to_json(nlohmann::json& json, const Levels& in)
    {
        json["Enabled"] = in.enabled;
        json["InLow"] = in.inLow;
        json["InHigh"] = in.inHigh;
        json["Gamma"] = in.gamma;
        json["OutLow"] = in.outLow;
        json["OutHigh"] = in.outHigh;
    }

    void to_json(nlohmann::json& json, const EXRDisplay& in)
    {
        json["Enabled"] = in.enabled;
        json["Exposure"] = in.exposure;
        json["Defog"] = in.defog;
        json["KneeLow"] = in.kneeLow;
        json["KneeHigh"] = in.kneeHigh;
    }

    void to_json(nlohmann::json& json, const SoftClip& in)
    {
        json["Enabled"] = in.enabled;
        json["Value"] = in.value;
    }

    void to_json(nlohmann::json& json, const DisplayOptions& in)
    {
        json["Channels"] = to_string(in.channels);
        json["Mirror"] = in.mirror;
        json["Color"] = in.color;
        json["Levels"] = in.levels;
        json["EXRDisplay"] = in.exrDisplay;
        json["SoftClip"] = in.softClip;
        json["ImageFilters"] = in.imageFilters;
    }

    void from_json(const nlohmann::json& json, Color& out)
    {
        json.at("Enabled").get_to(out.enabled);
        json.at("Add").get_to(out.add);
        json.at("Brightness").get_to(out.brightness);
        json.at("Contrast").get_to(out.contrast);
        json.at("Saturation").get_to(out.saturation);
        json.at("Hue").get_to(out.hue);
        json.at("Invert").get_to(out.invert);
    }

    void from_json(const nlohmann::json& json, Levels& out)
    {
        json.at("Enabled").get_to(out.enabled);
        json.at("InLow").get_to(out.inLow);
        json.at("InHigh").get_to(out.inHigh);
        json.at("Gamma").get_to(out.gamma);
        json.at("OutLow").get_to(out.outLow);
        json.at("OutHigh").get_to(out.outHigh);
    }

    void from_json(const nlohmann::json& json, EXRDisplay& out)
    {
        json.at("Enabled").get_to(out.enabled);
        json.at("Exposure").get_to(out.exposure);
        json.at("Defog").get_to(out.defog);
        json.at("KneeLow").get_to(out.kneeLow);
        json.at("KneeHigh").get_to(out.kneeHigh);
    }

    void from_json(const nlohmann::json& json, SoftClip& out)
    {
        json.at("Enabled").get_to(out.enabled);
        json.at("Value").get_to(out.value);
    }

    void from_json(const nlohmann::json& json, DisplayOptions& out)
    {
        from_string(json.at("Channels").get<std::string>(), out.channels);
        json.at("Mirror").get_to(out.mirror);
        json.at("Color").get_to(out.color);
        json.at("Levels").get_to(out.levels);
        json.at("EXRDisplay").get_to(out.exrDisplay);
        json.at("SoftClip").get_to(out.softClip);
        json.at("ImageFilters").get_to(out.imageFilters);
    }
}
