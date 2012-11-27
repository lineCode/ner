/* ner: src/notmuch_iterator.cc
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

#include <cstring>
#include <cassert>

#include "notmuch_iterator.hh"

namespace Notmuch
{
    #define DEFINE_ITERATOR(iterator, name)                                 \
        template <>                                                         \
        iterator::value_type iterator::operator*() const                    \
        {                                                                   \
            assert(notmuch_ ## name ## _valid(_collection));                \
            return notmuch_ ## name ## _get(_collection);                   \
        }                                                                   \
                                                                            \
        template <>                                                         \
        const iterator & iterator::operator++() const                       \
        {                                                                   \
            notmuch_ ## name ## _move_to_next(_collection);                 \
                                                                            \
            return *this;                                                   \
        }                                                                   \
                                                                            \
        template <>                                                         \
        bool iterator::at_end() const                                       \
        {                                                                   \
            return _collection == NULL                                      \
                || !notmuch_ ## name ## _valid(_collection);                \
        }

    DEFINE_ITERATOR(MessageIterator, messages)
    DEFINE_ITERATOR(ThreadIterator, threads)
    DEFINE_ITERATOR(TagIterator, tags)

    #undef DEFINE_ITERATOR

    MessageTreeIterator::MessageTreeIterator()
    {
    }

    notmuch_message_t * MessageTreeIterator::operator*() const
    {
        return messages.front();
    }

    const MessageTreeIterator & MessageTreeIterator::operator++()
    {
        notmuch_message_t * message = messages.front();
        messages.pop_front();

        std::list<notmuch_message_t *> replies_list;
        notmuch_messages_t * replies = notmuch_message_get_replies(message);
        std::copy(std::begin(replies), std::end(replies), std::back_inserter(replies_list));
        messages.splice(messages.begin(), replies_list);

        return *this;
    }

    bool MessageTreeIterator::operator==(const MessageTreeIterator & other) const
    {
        return messages.size() == other.messages.size() &&
            (messages.size() == 0 || std::strcmp(notmuch_message_get_message_id(messages.back()),
            notmuch_message_get_message_id(other.messages.back())) == 0);
    }

    bool MessageTreeIterator::operator!=(const MessageTreeIterator & other) const
    {
        return !operator==(other);
    }

}

namespace std
{
    #define DEFINE_BEGIN_END(iterator, name)                                \
        Notmuch::iterator begin(notmuch_ ## name ## _t * collection)        \
        {                                                                   \
            return Notmuch::iterator(collection);                           \
        }                                                                   \
                                                                            \
        Notmuch::iterator end(notmuch_ ## name ## _t * collection)          \
        {                                                                   \
            return Notmuch::iterator();                                     \
        }

    DEFINE_BEGIN_END(MessageIterator, messages)
    DEFINE_BEGIN_END(ThreadIterator, threads)
    DEFINE_BEGIN_END(TagIterator, tags)

    #undef DEFINE_BEGIN_END

    Notmuch::MessageTreeIterator begin(notmuch_thread_t * thread)
    {
        notmuch_messages_t * messages = notmuch_thread_get_toplevel_messages(thread);
        return Notmuch::MessageTreeIterator(std::begin(messages), std::end(messages));
    }

    Notmuch::MessageTreeIterator end(notmuch_thread_t * thread)
    {
        return Notmuch::MessageTreeIterator();
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

