/* ner: notmuch/config.hh
 *
 * Copyright (c) 2012 Michael Forney
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

#ifndef NER_NOTMUCH_CONFIG_H
#define NER_NOTMUCH_CONFIG_H 1

#include <string>
#include <vector>

namespace Notmuch
{
    class Config
    {
        public:
            static const Config & instance();

            Config();

            void load();

            struct
            {
                std::string path;
            } database;

            struct
            {
                std::string name;
                std::string primary_email;
                std::vector<std::string> other_email;
            } user;

        private:
            static const Config * _instance;
    };
}

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

