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

#ifdef __linux__

/************************************************************************/

#include <string>
#include <atomic>
#include <istream>

/************************************************************************/
/*
 * This is an attempt to provide a cancelable std::getline on linux.
 *
 * The idea is that "the terminal" handles the basic editing, like
 * backspace; therefore, the associated filehandle should only become
 * "ready" when the enter key is pressed. Therefore, I'm now trying to
 * poll on that handle and an eventFd, and only call getline() when
 * the filehandle becomes ready.
 *
 * To keep things simple, I'm not using asio or the waiter stuff for
 * this; just a hardcoded poll().
 */

/************************************************************************/

namespace SteamBot
{
    namespace UI
    {
        class GetLine
        {
        private:
            int event=-1;
            std::atomic_flag cancelled;

        public:
            GetLine();
            ~GetLine();

        public:
            std::istream& get(std::string&);
            void cancel();
        };
    }
}

/************************************************************************/

#endif
