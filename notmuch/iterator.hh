/* ner: notmuch/iterator.hh
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

#ifndef NER_NOTMUCH_ITERATOR_H
#define NER_NOTMUCH_ITERATOR_H 1

#include <iterator>
#include <vector>
#include <cassert>
#include <notmuch.h>

namespace Notmuch
{
    class Message;
    class Thread;

    template <typename Collection>
    struct CollectionTraits;

    template<>
    struct CollectionTraits<notmuch_messages_t>
    {
        typedef notmuch_message_t * Value;
    };

    template<>
    struct CollectionTraits<notmuch_threads_t>
    {
        typedef notmuch_thread_t * Value;
    };

    template<>
    struct CollectionTraits<notmuch_tags_t>
    {
        typedef const char * Value;
    };

    template <typename Type, typename Collection>
    class Iterator : public std::iterator<std::input_iterator_tag, Type>
    {
        public:
            explicit Iterator(Collection * collection)
                : _collection(collection)
            {
            }

            Iterator()
                : _collection(NULL)
            {
            }

            Type operator*() const
            {
                assert(valid(_collection));
                auto object = ptr(get(_collection));
                return Type(object.get());
            }

            const Iterator & operator++() const
            {
                move_to_next(_collection);
                return *this;
            }

            bool operator==(const Iterator & other) const
            {
                return (_collection == nullptr || !valid(_collection))
                    && (other._collection == nullptr || !valid(other._collection));
            }

            bool operator!=(const Iterator & other) const
            {
                return !operator==(other);
            }

        private:
            typedef typename CollectionTraits<Collection>::Value (* GetFunction)(Collection *);
            typedef notmuch_bool_t (* ValidFunction)(Collection *);
            typedef void (* MoveToNextFunction)(Collection *);

            static const GetFunction get;
            static const ValidFunction valid;
            static const MoveToNextFunction move_to_next;

            Collection * _collection;
    };

    typedef Iterator<Message, notmuch_messages_t> MessageIterator;
    typedef Iterator<Thread, notmuch_threads_t> ThreadIterator;
    typedef Iterator<const char *, notmuch_tags_t> TagIterator;
}

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

