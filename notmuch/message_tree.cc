/* ner: notmuch/message_tree.cc
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

#include "message_tree.hh"

namespace Notmuch
{
    void build_message_tree(Tree<Message> & tree, notmuch_messages_t * messages)
    {
        for (; notmuch_messages_valid(messages); notmuch_messages_move_to_next(messages))
        {
            notmuch_message_t * message = notmuch_messages_get(messages);
            tree.children.push_back({ Message(message), Tree<Message>() });
            build_message_tree(tree.children.back().branch,
                notmuch_message_get_replies(message));
        }
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

