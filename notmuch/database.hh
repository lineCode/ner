/* ner: notmuch/database.hh
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

#ifndef NER_NOTMUCH_DATABASE_H
#define NER_NOTMUCH_DATABASE_H 1

#include <notmuch.h>

#include "notmuch/util.hh"
#include "notmuch/message.hh"
#include "notmuch/thread.hh"

namespace Notmuch
{
    class Database
    {
        public:
            enum class Mode
            {
                ReadOnly,
                ReadWrite
            };

            /**
             * Open a new database connection.
             */
            Database(Mode mode = Mode::ReadOnly);

            ~Database();

            void close();

            Message find_message(const std::string & id, Message::Parts parts = Message::AllParts);
            Thread find_thread(const std::string & id, Thread::Parts parts = Thread::AllParts);

        private:
            notmuch_database_t * get() const;

            Pointer<notmuch_database_t> _database;

        friend class Query;
        friend class Message;
    };
}

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

