/* ner: src/message_part_save_visitor.cc
 *
 * Copyright (c) 2010 Maxime Coste
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

#include "message_part_save_visitor.hh"
#include "message_part.hh"
#include "status_bar.hh"
#include "line_editor.hh"

#include <sys/stat.h>

MessagePartSaveVisitor::MessagePartSaveVisitor()
{
}

void MessagePartSaveVisitor::visit(const TextPart & part)
{
}

void MessagePartSaveVisitor::visit(const Attachment & part)
{
    std::string filename;
    if (StatusBar::instance().prompt(filename, "Save attachement to file: ",
        "save-attachment", part.filename) && !filename.empty())
    {
        struct stat dummy;
        if (stat(filename.c_str(), &dummy) == 0)
        {
            std::string answer;

            if (StatusBar::instance().prompt(answer, "File exists, overwrite? [y,N]: ")
                && (answer == "y" || answer == "Y"))
            {
                FILE * file = fopen(filename.c_str(), "w");
                GMimeStream * stream = g_mime_stream_file_new(file);
                g_mime_data_wrapper_write_to_stream(part.data, stream);
                g_object_unref(stream);
            }
        }
    }
}
