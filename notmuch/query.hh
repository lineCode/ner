/* ner: notmuch/query.hh
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

#ifndef NER_NOTMUCH_QUERY_H
#define NER_NOTMUCH_QUERY_H 1

#include <notmuch.h>

#include "database.hh"

namespace Notmuch
{
    class Threads;
    class Messages;

    enum class SortMode
    {
        OldestFirst,
        NewestFirst,
        MessageID,
        Unsorted
    };

    template <typename Type, typename Collection>
    class Results
    {
        public:
            Results(Results && other)
                : _collection(std::move(other._collection)), _parts(other._parts)
            {
            }

            Results(const Results & other) = delete;

            Iterator<Type, Collection> begin()
            {
                return Iterator<Type, Collection>(_collection.get());
            }

            Iterator<Type, Collection> end()
            {
                return Iterator<Type, Collection>();
            }

        private:
            Results(Pointer<Collection> collection, typename Type::Parts parts)
                : _collection(std::move(collection)), _parts(parts)
            {
            }

            Pointer<Collection> _collection;
            typename Type::Parts _parts;

        friend class Query;
    };

    typedef Results<Thread, notmuch_threads_t> ThreadResults;
    typedef Results<Message, notmuch_messages_t> MessageResults;

    class Query
    {
        public:
            Query(const std::string & terms, const Database * database);

            void set_sort_mode(SortMode mode);

            MessageResults messages(Message::Parts parts = Message::MetadataPart);
            ThreadResults threads(Thread::Parts parts = Thread::MetadataPart);

            unsigned count_messages();
            unsigned count_threads();

        private:
            Pointer<notmuch_query_t> _query;
    };
}

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

