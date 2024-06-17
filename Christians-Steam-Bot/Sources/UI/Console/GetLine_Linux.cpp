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

#ifdef __linux__

/************************************************************************/

#include "GetLine_Linux.hpp"
#include "Exceptions.hpp"

#include <cstdio>
#include <array>
#include <cassert>
#include <iostream>

#include <poll.h>
#include <sys/eventfd.h>

#include <boost/log/trivial.hpp>

/************************************************************************/

typedef SteamBot::UI::GetLine GetLine;

/************************************************************************/

GetLine::GetLine()
{
    if ((event=eventfd(0, 0))==-1)
    {
        int Errno=errno;
        throw std::system_error(Errno, std::generic_category());
    }
}

/************************************************************************/

GetLine::~GetLine()
{
    if (event!=-1)
    {
        close(event);
    }
}

/************************************************************************/

void GetLine::cancel()
{
    if (!cancelled.test_and_set())
    {
        if (eventfd_write(event, 1)!=0)
        {
            int Errno=errno;
            throw std::system_error(Errno, std::generic_category());
        }
    }
}

/************************************************************************/

std::istream& GetLine::get(std::string& result)
{
    std::array<pollfd, 2> pollFds{};

    pollFds[0].fd=event;
    pollFds[0].events=POLLIN;

    pollFds[1].fd=fileno(stdin);		// stdin should be linked with std::cin
    assert(pollFds[1].fd==0);
    pollFds[1].events=POLLIN;

    const auto n=poll(pollFds.data(), pollFds.size(), -1);
    assert(n!=0);
    if (n<0)
    {
        int Errno=errno;
        throw std::system_error(Errno, std::generic_category());
    }

    if (cancelled.test())
    {
        BOOST_LOG_TRIVIAL(info) << "getLine cancelled";
        throw SteamBot::OperationCancelledException();
    }

    return std::getline(std::cin, result);
}

/************************************************************************/

#endif
