/* ner: notmuch/query.cc
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

#include "query.hh"

namespace Notmuch
{
    Query::Query(const std::string & terms, const Database * database)
        : _query(ptr(notmuch_query_create(database->get(), terms.c_str())))
    {
    }

    void Query::set_sort_mode(SortMode mode)
    {
        notmuch_sort_t sort;

        switch (mode)
        {
            case SortMode::OldestFirst:
                sort = NOTMUCH_SORT_OLDEST_FIRST;
                break;
            case SortMode::NewestFirst:
                sort = NOTMUCH_SORT_NEWEST_FIRST;
                break;
            case SortMode::MessageID:
                sort = NOTMUCH_SORT_MESSAGE_ID;
                break;
            case SortMode::Unsorted:
                sort = NOTMUCH_SORT_UNSORTED;
                break;
            default:
                throw std::invalid_argument("Invalid sort mode");
        }

        notmuch_query_set_sort(_query.get(), sort);
    }

    ThreadResults Query::threads(Thread::Parts parts)
    {
        return ThreadResults(std::move(ptr(notmuch_query_search_threads
            (_query.get()))), parts);
    }

    MessageResults Query::messages(Message::Parts parts)
    {
        return MessageResults(std::move(ptr(notmuch_query_search_messages
            (_query.get()))), parts);
    }

    unsigned Query::count_messages()
    {
        return notmuch_query_count_messages(_query.get());
    }

    unsigned Query::count_threads()
    {
        return notmuch_query_count_threads(_query.get());
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

