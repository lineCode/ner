/* ner: src/message_part_display_visitor.cc
 *
 * Copyright (c) 2010 Michael Forney
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

#include <sstream>

#include "message_part_display_visitor.hh"
#include "colors.hh"
#include "message_part.hh"
#include "line_wrapper.hh"
#include "util.hh"

const int wrapWidth(80);

MessagePartDisplayVisitor::MessagePartDisplayVisitor(WINDOW * window,
    const View::Geometry & area, int offset, int selection)
    : _renderer(window, false), _area(area), _offset(offset), _messageRow(0),
        _selection(selection)
{
    _renderer.move(area.y, 0);
}

void MessagePartDisplayVisitor::visit(const TextPart & part)
{
    using namespace NCurses;

    Renderer & r = _renderer;

    if (_messageRow >= _offset && r.row() < _area.y + _area.height)
    {
        bool selected = _messageRow == _selection;

        r << styled(part.folded ? '+' : '-', Color::AttachmentFilename, A_BOLD);
        r.skip(1);

        r.set_line_attributes(selected ? A_REVERSE : 0);
        r << "Text Part: " << styled(part.contentType, Color::AttachmentMimeType)
            << clear_attr;

        r.next_line();
    }

    ++_messageRow;

    if (part.folded)
        return;

    for (auto & line : part.lines)
    {
        unsigned citationLevel = 0;
        for (auto c : line)
        {
            if (c == '>')
                ++citationLevel;
            else if (c != ' ')
                break;
        }

        Color color = None;
        if (citationLevel)
        {
            switch (citationLevel % 4)
            {
                case 1: color = Color::CitationLevel1; break;
                case 2: color = Color::CitationLevel2; break;
                case 3: color = Color::CitationLevel3; break;
                case 0: color = Color::CitationLevel4; break;
            }
        }

        for (auto lineWrapper = LineWrapper(line, _area.width-1); !lineWrapper.done(); ++_messageRow)
        {
            bool selected = _messageRow == _selection;
            bool wrapped = lineWrapper.wrapped();

            std::string wrappedLine(lineWrapper.next());

            if (_messageRow < _offset || r.row() >= _area.y + _area.height)
                continue;

            if (wrapped)
                r << styled(ch(ACS_CKBOARD), Color::LineWrapIndicator);

            r.advance(2);

            r.set_line_attributes(selected ? A_REVERSE : 0);

            r << styled(wrappedLine, color) << clear_attr;
            r.add_cut_off_indicator();
            r.next_line();
        }
    }
}

void MessagePartDisplayVisitor::visit(const Attachment & part)
{
    using namespace NCurses;

    Renderer & r = _renderer;

    if (_messageRow >= _offset && r.row() < _area.y + _area.height)
    {
        bool selected = _messageRow == _selection;

        r << styled('*', Color::AttachmentFilename, A_BOLD);
        r.skip(1);

        if (selected)
            r.set_line_attributes(A_REVERSE);

        r << "Attachment: " << set_color(Color::AttachmentFilename) << part.filename;
        r.skip(1);
        r << set_color(Color::AttachmentMimeType) << part.contentType;
        r.skip(1);
        r << set_color(Color::AttachmentFilesize) << part.filesize;

        r.add_cut_off_indicator();
        r.next_line();
        ++_messageRow;
    }
}

int MessagePartDisplayVisitor::row() const
{
    return _renderer.row();
}

int MessagePartDisplayVisitor::lines() const
{
    return _messageRow;
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

