/* ner: notmuch/database.cc
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

#include <stdexcept>

#include "config.hh"
#include "database.hh"
#include "exception.hh"
#include "query.hh"

namespace Notmuch
{
    Database::Database(Mode mode)
    {
        notmuch_database_mode_t notmuch_mode;

        switch (mode)
        {
            case Mode::ReadOnly:
                notmuch_mode = NOTMUCH_DATABASE_MODE_READ_ONLY;
                break;
            case Mode::ReadWrite:
                notmuch_mode = NOTMUCH_DATABASE_MODE_READ_WRITE;
                break;
            default:
                throw std::invalid_argument("Invalid mode");
        }

        notmuch_database_t * notmuch_database;

        notmuch_status_t status = notmuch_database_open(
            Config::instance().database.path.c_str(), notmuch_mode, &notmuch_database);

        _database = ptr(notmuch_database);

        if (status != NOTMUCH_STATUS_SUCCESS)
            throw std::runtime_error("Could not open database");
    }

    Database::~Database()
    {
    }

    void Database::close()
    {
        _database.reset();
    }

    Message Database::find_message(const std::string & id, Message::Parts parts)
    {
        notmuch_message_t * message;
        auto status = notmuch_database_find_message(_database.get(), id.c_str(),
            &message);

        if (status != NOTMUCH_STATUS_SUCCESS)
            throw InvalidMessageException(id);

        return Message(message, parts);
    }

    Thread Database::find_thread(const std::string & id, Thread::Parts parts)
    {
        Query query("thread:" + id, this);
        ThreadResults threads = query.threads(parts);
        auto thread = threads.begin(), e = threads.end();

        if (thread == e)
            throw InvalidThreadException(id);

        return *thread;
    }

    notmuch_database_t * Database::get() const
    {
        return _database.get();
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

