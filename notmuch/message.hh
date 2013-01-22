/* ner: notmuch/message.hh
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

#ifndef NER_NOTMUCH_MESSAGE_H
#define NER_NOTMUCH_MESSAGE_H 1

#include <chrono>
#include <map>
#include <set>
#include <string>
#include <notmuch.h>

#include "notmuch/tag_operations.hh"
#include "notmuch/util.hh"
#include "notmuch/tree.hh"

namespace Notmuch
{
    class Message
    {
        public:
            enum
            {
                MetadataPart = 1 << 0,
                FilenamePart = 1 << 1
            };

            typedef unsigned int Parts;
            static const Parts AllParts = ~0;

            Message() = default;

            std::string id;
            std::string subject;
            std::chrono::system_clock::time_point date;
            std::set<std::string> tags;
            std::string filename;

            std::map<CaseInsensitiveString, std::string> headers;

            bool perform_tag_operations(const TagOperations & ops);

        private:
            Message(notmuch_message_t * message, Parts parts = AllParts);

        friend class Database;
        friend void build_message_tree(Tree<Message> & tree, notmuch_messages_t * messages);
    };
}

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

