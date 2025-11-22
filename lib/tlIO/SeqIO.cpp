// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIO/SeqIO.h>

#include <ftk/Core/Format.h>

namespace tl
{
    namespace io
    {
        bool SeqOptions::operator == (const SeqOptions& other) const
        {
            return
                defaultSpeed == other.defaultSpeed &&
                threadCount == other.threadCount;
        }

        bool SeqOptions::operator != (const SeqOptions& other) const
        {
            return !(*this == other);
        }

        Options getOptions(const SeqOptions& value)
        {
            Options out;
            out["SeqIO/DefaultSpeed"] = ftk::Format("{0}").arg(value.defaultSpeed);
            out["SeqIO/ThreadCount"] = ftk::Format("{0}").arg(value.threadCount);
            return out;
        }

        void to_json(nlohmann::json& json, const SeqOptions& value)
        {
            json["DefaultSpeed"] = value.defaultSpeed;
            json["ThreadCount"] = value.threadCount;
        }

        void from_json(const nlohmann::json& json, SeqOptions& value)
        {
            json.at("DefaultSpeed").get_to(value.defaultSpeed);
            json.at("ThreadCount").get_to(value.threadCount);
        }
    }
}
