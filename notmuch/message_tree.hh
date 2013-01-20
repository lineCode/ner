/* ner: notmuch/message_tree.hh
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

#ifndef NER_NOTMUCH_MESSAGE_TREE_H
#define NER_NOTMUCH_MESSAGE_TREE_H 1

#include <vector>
#include <notmuch.h>

#include "notmuch/message.hh"
#include "notmuch/tree.hh"

namespace Notmuch
{
    void build_message_tree(Tree<Message> & tree, notmuch_messages_t * messages);
}

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

