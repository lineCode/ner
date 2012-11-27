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

namespace Notmuch
{
    template <typename C, typename T>
    class Iterator : public std::iterator<std::input_iterator_tag, T>
    {
        public:
            Iterator(C * collection)
                : _collection(collection)
            {
            }

            Iterator()
                : _collection(NULL)
            {
            }

            T operator*() const;
            const Iterator & operator++() const;

            bool operator==(const Iterator & other) const
            {
                return at_end() == other.at_end();
            }

            bool operator!=(const Iterator & other) const
            {
                return !operator==(other);
            }

            bool at_end() const;

        private:

            C * _collection;
    };

    typedef Iterator<notmuch_messages_t, notmuch_message_t *> MessageIterator;
    typedef Iterator<notmuch_threads_t, notmuch_thread_t *> ThreadIterator;
    typedef Iterator<notmuch_tags_t, const char *> TagIterator;

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

namespace std
{
    Notmuch::MessageIterator begin(notmuch_messages_t * messages);
    Notmuch::MessageIterator end(notmuch_messages_t * messages);

    Notmuch::ThreadIterator begin(notmuch_threads_t * threads);
    Notmuch::ThreadIterator end(notmuch_threads_t * threads);

    Notmuch::TagIterator begin(notmuch_tags_t * tags);
    Notmuch::TagIterator end(notmuch_tags_t * tags);

    Notmuch::MessageTreeIterator begin(notmuch_thread_t * thread);
    Notmuch::MessageTreeIterator end(notmuch_thread_t * thread);
}

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

