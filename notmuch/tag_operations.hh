/* ner: notmuch/tag_operations.hh
 *
 * Copyright (c) 2013 Michael Forney
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

#ifndef NER_NOTMUCH_TAG_OPERATIONS_H
#define NER_NOTMUCH_TAG_OPERATIONS_H 1

#include <string>
#include <vector>

namespace Notmuch
{
    struct TagOperation
    {
        enum Type
        {
            Add,
            Remove,
            Clear
        };

        Type type;
        std::string tag;
    };

    typedef std::vector<TagOperation> TagOperations;

    TagOperation add_operation(const std::string & tag);
    TagOperation remove_operation(const std::string & tag);
    TagOperation clear_operation();
}

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

