/* ner: notmuch/message.cc
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

#include "message.hh"
#include "database.hh"

namespace Notmuch
{
    Message::Message(notmuch_message_t * message, Parts parts)
    {
        if (parts & MetadataPart)
        {
            id = notmuch_message_get_message_id(message);
            subject = notmuch_message_get_header(message, "subject");
            date = std::chrono::system_clock::from_time_t(
                notmuch_message_get_date(message));

            headers.insert(std::make_pair("from",
                notmuch_message_get_header(message, "from") ?: "(null)"));

            notmuch_tags_t * notmuch_tags;
            for (notmuch_tags = notmuch_message_get_tags(message);
                notmuch_tags_valid(notmuch_tags);
                notmuch_tags_move_to_next(notmuch_tags))
            {
                tags.insert(notmuch_tags_get(notmuch_tags));
            }
        }

        if (parts & FilenamePart)
            filename = notmuch_message_get_filename(message);
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

