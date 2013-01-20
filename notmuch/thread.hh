/* ner: notmuch/thread.hh
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

#ifndef NER_NOTMUCH_THREAD_H
#define NER_NOTMUCH_THREAD_H 1

#include <chrono>
#include <set>
#include <string>
#include <notmuch.h>

#include "util.hh"
#include "tree.hh"
#include "iterator.hh"
#include "message.hh"

namespace Notmuch
{
    class Thread
    {
        public:
            enum
            {
                MetadataPart    = 1 << 0,
                TreePart        = 1 << 1
            };

            typedef unsigned int Parts;
            static const Parts AllParts = ~0;

            Thread() = default;

            /* Metadata */
            std::string id;
            std::string subject;
            std::string authors;
            std::chrono::system_clock::time_point date;
            std::set<std::string> tags;
            int total_messages;
            int matched_messages;

            /* Message Tree */
            Tree<Message> tree;

        private:
            Thread(notmuch_thread_t * thread, Parts parts = AllParts);

        friend ThreadIterator;
    };
}

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

