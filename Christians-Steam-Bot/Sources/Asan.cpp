/*
 * This file is part of "Christians-Steam-Bot"
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

/************************************************************************/
/*
 * https://stackoverflow.com/questions/51060801/how-to-suppress-leaksanitizer-report-when-running-under-fsanitize-address
 */

#if defined(__GNUC__) && defined(__SANITIZE_ADDRESS__)
extern "C"
{
    const char* __asan_default_options()
    {
        return "detect_leaks=0";
    }
}
#endif
