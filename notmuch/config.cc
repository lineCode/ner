/* ner: notmuch/config.cc
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

#include <glib-object.h>
#include <stdexcept>
#include <unistd.h>

#include "config.hh"

namespace Notmuch
{
    const Config * Config::_instance = nullptr;

    const Config & Config::instance()
    {
        return *_instance;
    }

    Config::Config()
    {
        _instance = this;
    }

    void Config::load()
    {
        const char * environment_path = std::getenv("NOTMUCH_CONFIG");
        std::string default_path(std::string(std::getenv("HOME")) + "/.notmuch-config");

        const std::string & path = (environment_path != NULL &&
            access(environment_path, R_OK) == 0) ? environment_path : default_path;

        GKeyFile * config = g_key_file_new();

        if (!g_key_file_load_from_file(config, path.c_str(), G_KEY_FILE_NONE, NULL))
            throw std::runtime_error("Invalid config file: " + path);

        const char * value;

        if ((value = g_key_file_get_string(config, "database", "path", NULL)))
            database.path = value;

        if ((value = g_key_file_get_string(config, "user", "name", NULL)))
            user.name = value;
        if ((value = g_key_file_get_string(config, "user", "primary_email", NULL)))
            user.primary_email = value;

        gsize addresses_length;
        char ** addresses = g_key_file_get_string_list(config, "user", "other_email",
            &addresses_length, NULL);
        for (int i = 0; i < addresses_length; ++i, ++addresses)
            user.other_email.push_back(*addresses);

        g_key_file_free(config);
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

