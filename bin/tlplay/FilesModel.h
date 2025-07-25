// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Player.h>

namespace tl
{
    namespace play
    {
        class SettingsModel;

        //! Files model.
        class FilesModel : public std::enable_shared_from_this<FilesModel>
        {
            FEATHER_TK_NON_COPYABLE(FilesModel);

        protected:
            void _init(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<SettingsModel>&);

            FilesModel() = default;

        public:
            ~FilesModel();

            //! Create a new model.
            static std::shared_ptr<FilesModel> create(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<SettingsModel>&);

            void open(const std::filesystem::path&);
            void open();
            void close();
            void close(int);
            void closeAll();
            void reload();
            void setCurrent(int);
            void next();
            void prev();

            std::shared_ptr<feather_tk::IObservableList<std::shared_ptr<timeline::Player> > > observePlayers() const;
            std::shared_ptr<feather_tk::IObservableValue<std::shared_ptr<timeline::Player> > > observePlayer() const;
            std::shared_ptr<feather_tk::IObservableValue<int> > observePlayerIndex() const;

            void setB(int);
            void setCompare(timeline::Compare);

            std::shared_ptr<feather_tk::IObservableValue<std::shared_ptr<timeline::Player> > > observeBPlayer() const;
            std::shared_ptr<feather_tk::IObservableValue<int> > observeBPlayerIndex() const;
            std::shared_ptr<feather_tk::IObservableValue<timeline::Compare> > observeCompare() const;

            void tick();

        private:
            std::weak_ptr<feather_tk::Context> _context;
            std::shared_ptr<feather_tk::ObservableList<std::shared_ptr<timeline::Player> > > _players;
            std::shared_ptr<feather_tk::ObservableValue<std::shared_ptr<timeline::Player> > > _player;
            std::shared_ptr<feather_tk::ObservableValue<int> > _playerIndex;
            std::shared_ptr<feather_tk::ObservableValue<std::shared_ptr<timeline::Player> > > _bPlayer;
            std::shared_ptr<feather_tk::ObservableValue<int> > _bPlayerIndex;
            std::shared_ptr<feather_tk::ObservableValue<timeline::Compare> > _compare;
            std::shared_ptr<feather_tk::ValueObserver<timeline::PlayerCacheOptions> > _cacheObserver;
        };
    }
}