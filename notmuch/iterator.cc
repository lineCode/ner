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

#include "iterator.hh"

namespace Notmuch
{
    #define DEFINE_ITERATOR_FUNCTIONS(iterator, name)                       \
        template <> const iterator::GetFunction                             \
        iterator::get = &notmuch_ ## name ## _get;                          \
        template <> const iterator::ValidFunction                           \
        iterator::valid = &notmuch_ ## name ## _valid;                      \
        template <> const iterator::MoveToNextFunction                      \
        iterator::move_to_next = &notmuch_ ## name ## _move_to_next;

    DEFINE_ITERATOR_FUNCTIONS(MessageIterator, messages);
    DEFINE_ITERATOR_FUNCTIONS(ThreadIterator, threads);
    DEFINE_ITERATOR_FUNCTIONS(TagIterator, tags);
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

