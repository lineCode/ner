/* ner: src/notmuch_iterator.hh
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
#include <list>
#include <notmuch.h>
#include <cassert>

namespace Notmuch
{
    template <typename C>
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

    template <typename C, typename T = typename CollectionTraits<C>::Value>
    class Iterator : public std::iterator<std::input_iterator_tag, T>
    {
        public:
            explicit Iterator(C * collection)
                : _collection(collection)
            {
            }

            Iterator()
                : _collection(NULL)
            {
            }

            T operator*() const
            {
                assert(valid(_collection));
                return get(_collection);
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
            typedef T (* GetFunction)(C *);
            typedef notmuch_bool_t (* ValidFunction)(C *);
            typedef void (* MoveToNextFunction)(C *);

            static GetFunction get;
            static ValidFunction valid;
            static MoveToNextFunction move_to_next;

            C * _collection;
    };

    typedef Iterator<notmuch_messages_t> MessageIterator;
    typedef Iterator<notmuch_threads_t> ThreadIterator;
    typedef Iterator<notmuch_tags_t> TagIterator;

    class MessageTreeIterator : public std::iterator<std::input_iterator_tag, notmuch_message_t *>
    {
        public:
            template <class InputIterator>
            explicit MessageTreeIterator(InputIterator begin, InputIterator end)
            {
                std::copy(begin, end, std::back_inserter(messages));
            }

            template <class Container>
            explicit MessageTreeIterator(const Container & container)
            {
                std::copy(container.begin(), container.end(), std::back_inserter(messages));
            }

            MessageTreeIterator();

            notmuch_message_t * operator*() const;
            const MessageTreeIterator & operator++();
            bool operator==(const MessageTreeIterator & other) const;
            bool operator!=(const MessageTreeIterator & other) const;

        private:
            std::list<notmuch_message_t *> messages;
    };
}

template <typename C>
Notmuch::Iterator<C> begin(C * collection)
{
    return Notmuch::Iterator<C>(collection);
}

template <typename C>
Notmuch::Iterator<C> end(C * collection)
{
    return Notmuch::Iterator<C>();
}

Notmuch::MessageTreeIterator begin(notmuch_thread_t * thread);
Notmuch::MessageTreeIterator end(notmuch_thread_t * thread);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

