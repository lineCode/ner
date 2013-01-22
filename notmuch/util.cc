/* ner: notmuch/util.cc
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

#include <notmuch.h>

#include "util.hh"

namespace Notmuch
{
    #define DEFINE_POINTER(name, destroy)                                   \
        template <> const Deleter<notmuch_ ## name ## _t>::DeleteFunction   \
        Deleter<notmuch_ ## name ## _t>::del = &destroy;
    #define DEFINE_POINTER_DESTROY(name) \
        DEFINE_POINTER(name, notmuch_ ## name ## _destroy)

    DEFINE_POINTER_DESTROY(message)
    DEFINE_POINTER_DESTROY(messages)
    DEFINE_POINTER_DESTROY(thread)
    DEFINE_POINTER_DESTROY(threads)
    DEFINE_POINTER_DESTROY(query)

    DEFINE_POINTER(database, notmuch_database_close)
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

