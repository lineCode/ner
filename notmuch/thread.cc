/* ner: notmuch/thread.cc
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

#include "thread.hh"

namespace Notmuch
{
    Thread::Thread(notmuch_thread_t * thread, Parts parts)
    {
        if (parts & MetadataPart)
        {
            id = notmuch_thread_get_thread_id(thread);
            subject = notmuch_thread_get_subject(thread);
            authors = notmuch_thread_get_authors(thread) ?: "(null)";
            date = std::chrono::system_clock::from_time_t(
                notmuch_thread_get_newest_date(thread));
            matched_messages = notmuch_thread_get_matched_messages(thread);
            total_messages = notmuch_thread_get_total_messages(thread);

            notmuch_tags_t * notmuch_tags;
            for (notmuch_tags = notmuch_thread_get_tags(thread);
                notmuch_tags_valid(notmuch_tags);
                notmuch_tags_move_to_next(notmuch_tags))
            {
                tags.insert(notmuch_tags_get(notmuch_tags));
            }
        }

        if (parts & TreePart)
            build_message_tree(tree, notmuch_thread_get_toplevel_messages(thread));
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

