/* ner: src/util.cc
 *
 * Copyright (c) 2010 Michael Forney
 * Copyright (c) 2010 Mike Kelly
 *
 * This file is a part of ner.
 *
 * ner is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 3, as published by the Free
 * Software Foundation.
 *
 * ner is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ner.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdio>
#include <sstream>
#include <iomanip>

#include "util.hh"

std::string relativeTime(time_t time)
{
    using namespace std::chrono;

    typedef duration<int, std::ratio<86400>> days;

    auto now = system_clock::now();
    auto then = system_clock::from_time_t(time);

    if (then > now)
        return "the future";

    time_t current_time = system_clock::to_time_t(now);

    auto difference = now - then;
    struct tm local_time = *std::localtime(&time);
    struct tm current_local_time = *std::localtime(&current_time);

    /* std::put_time is not available in GCC yet (part of C++11). */

    char string[16];

    if (difference > days(180))
        strftime(string, sizeof(string), "%F", &local_time);
    else if (difference < hours(1))
        snprintf(string, sizeof(string), "%lu mins. ago", duration_cast<minutes>(difference).count());
    else if (difference < days(7))
    {
        if (local_time.tm_wday == current_local_time.tm_wday && difference < days(1))
            strftime(string, sizeof(string), "Today %R", &local_time);
        else if ((current_local_time.tm_wday + 7 - local_time.tm_wday) % 7 == 1)
            strftime(string, sizeof(string), "Yest. %R", &local_time);
        else
            strftime(string, sizeof(string), "%a. %R", &local_time);
    }
    else
        strftime(string, sizeof(string), "%B %d", &local_time);

    return string;
}

std::string formatByteSize(long size)
{
    int i(0);
    std::string suffix[] = { "B", "KiB", "MiB", "GiB" };
    std::ostringstream val;

    while (size >= 1024.0 && i < 3)
    {
        size /= 1024.0;
        i++;
    }

    if (i >= 1)
        val << std::fixed << std::setprecision(2);

    val << size << " " << suffix[i];

    return val.str();
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

