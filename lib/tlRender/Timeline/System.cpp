// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/Timeline/System.h>

#include <tlRender/Timeline/ColorOptions.h>
#include <tlRender/Timeline/Player.h>

#include <ftk/Core/Context.h>
#include <ftk/Core/Format.h>
#include <ftk/Core/String.h>

namespace tl
{
    struct System::Private
    {
        std::vector<std::weak_ptr<Player> > players;
    };

    System::System(const std::shared_ptr<ftk::Context>& context) :
        ISystem(context, "tl::System"),
        _p(new Private)
    {
        FTK_P();
        const auto lutNames = getLUTFormatNames();
        const auto lutExts = getLUTFormatExts();
        std::vector<std::string> s;
        for (size_t i = 0; i < lutNames.size() && i < lutExts.size(); ++i)
        {
            s.push_back(ftk::Format("    * {0}: {1}").arg(lutNames[i]).arg(lutExts[i]));
        }
        _log(ftk::Format("Supported LUT formats:\n{0}").arg(ftk::join(s, '\n')));
    }

    System::~System()
    {}

    std::shared_ptr<System> System::create(const std::shared_ptr<ftk::Context>& context)
    {
        auto out = context->getSystem<System>();
        if (!out)
        {
            out = std::shared_ptr<System>(new System(context));
            context->addSystem(out);
        }
        return out;
    }

    void System::tick()
    {
        FTK_P();

        // Delete the expired players.
        auto players = p.players;
        auto i = players.begin();
        while (i != players.end())
        {
            if (i->expired())
            {
                i = players.erase(i);
            }
            else
            {
                ++i;
            }
        }

        // Tick the active players.
        for (i = players.begin(); i != players.end(); ++i)
        {
            if (auto player = i->lock())
            {
                player->_tick();
            }
        }
    }

    std::chrono::milliseconds System::getTickTime() const
    {
        return std::chrono::milliseconds(1);
    }

    void System::_addPlayer(const std::shared_ptr<Player>& player)
    {
        _p->players.push_back(player);
    }
}
