/*
 * This file is part of "Christians-Steam-Framework"
 * Copyright (C) 2023- Christian Stieber
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file LICENSE.  If not see
 * <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "Client/ClientInfo.hpp"
#include "Modules/BadgeData.hpp"

#include "../Console.hpp"

/************************************************************************/

typedef SteamBot::UI::CLI CLI;

/************************************************************************/

class CLI::Helpers
{
private:
    CLI& cli;

public:
    Helpers(CLI&);
    ~Helpers();

public:
    typedef SteamBot::Modules::LicenseList::Whiteboard::Licenses::LicenseInfo LicenseInfo;
    static std::vector<std::shared_ptr<const LicenseInfo>> getLicenseInfo(const SteamBot::ClientInfo&, SteamBot::AppID);

public:
    typedef SteamBot::Modules::OwnedGames::Whiteboard::OwnedGames OwnedGames;
    typedef SteamBot::Modules::BadgeData::Whiteboard::BadgeData BadgeData;

    class GameInfo
    {
    public:
        OwnedGames::Ptr ownedGames;
        BadgeData::Ptr badgeData;

    public:
        GameInfo(const SteamBot::ClientInfo&);
    };
};
