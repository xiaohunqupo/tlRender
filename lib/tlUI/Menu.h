// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Action.h>
#include <tlUI/IPopup.h>

namespace tl
{
    namespace ui
    {
        //! Menu.
        class Menu : public IPopup
        {
            TLRENDER_NON_COPYABLE(Menu);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            Menu();

        public:
            ~Menu() override;

            //! Create a new widget.
            static std::shared_ptr<Menu> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Add an action.
            void addAction(const std::shared_ptr<Action>&);

            //! Get the actions.
            const std::list<std::shared_ptr<Action> >& getActions() const;

            //! Add a sub menu.
            std::shared_ptr<Menu> addSubMenu(const std::string&);

            //! Add a divider.
            void addDivider();

            //! Clear the menu.
            void clear();

        private:
            TLRENDER_PRIVATE();
        };
    }
}
