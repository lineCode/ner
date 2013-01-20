/* ner: src/email_view.cc
 *
 * Copyright (c) 2010 Michael Forney
 * Copyright (c) 2011 Maxime Coste
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

#include "email_view.hh"
#include "colors.hh"
#include "ncurses.hh"
#include "util.hh"
#include "status_bar.hh"
#include "message_part_display_visitor.hh"
#include "message_part_save_visitor.hh"

const std::string lessMessage("[less]");
const std::string moreMessage("[more]");

EmailView::EmailView(const View::Geometry & geometry)
    : LineBrowserView(geometry),
        _visibleHeaders{
            "From",
            "To",
            "Cc",
            "Subject",
        },
        _lineCount(0)
{
}

EmailView::~EmailView()
{
}

void EmailView::setEmail(const std::string & filename)
{
    _parts.clear();

    FILE * file = fopen(filename.c_str(), "r");

    if (file != NULL)
    {
        GMimeStream * stream = g_mime_stream_file_new(file);
        GMimeParser * parser = g_mime_parser_new_with_stream(stream);
        GMimeMessage * message = g_mime_parser_construct_message(parser);

        /* Read relavant headers */
        _headers = {
            { "To",         internet_address_list_to_string(g_mime_message_get_recipients(message,
                GMIME_RECIPIENT_TYPE_TO), true) ? : "(null)" },
            { "From",       g_mime_message_get_sender(message) ? : "(null)" },
            { "Cc",         internet_address_list_to_string(g_mime_message_get_recipients(message,
                GMIME_RECIPIENT_TYPE_CC), true) ? : "(null)" },
            { "Bcc",         internet_address_list_to_string(g_mime_message_get_recipients(message,
                GMIME_RECIPIENT_TYPE_BCC), true) ? : "(null)" },
            { "Subject",    g_mime_message_get_subject(message) ? : "(null)" }
        };

        GMimeObject * mimePart = g_mime_message_get_mime_part(message);

        /* Locate plain text parts */
        processMimePart(mimePart, std::back_inserter(_parts));
        if (!_parts.empty())
            _parts[0]->folded = false;

        g_object_unref(message);
        g_object_unref(parser);
        g_object_unref(stream);
    }
}

void EmailView::setVisibleHeaders(const std::vector<std::string> & headers)
{
    _visibleHeaders = headers;
}

void EmailView::update()
{
    using namespace NCurses;

    int row = 0;

    _partsEndLine.clear();

    Renderer r(_window);

    for (auto & header : _visibleHeaders)
    {
        if (r.off_screen())
            return;

        r << set_color(Color::EmailViewHeader) << header << ':';
        r.skip(1);
        r << set_color() << _headers[header];

        r.add_cut_off_indicator();
        r.next_line();
    }

    whline(_window, 0, _geometry.width);
    r.next_line();

    MessagePartDisplayVisitor displayVisitor(_window, View::Geometry{ 0, r.row(),
        _geometry.width, visibleLines() }, _offset, _selectedIndex);

    for (auto & part : _parts)
    {
        part->accept(displayVisitor);
        _partsEndLine.push_back(displayVisitor.lines());
    }

    r.move(displayVisitor.row(), 0);
    _lineCount = displayVisitor.lines();

    for (; !r.off_screen(); r.next_line())
        r << styled('~', Color::EmptySpaceIndicator, A_BOLD);

    r.set_color(Color::MoreLessIndicator);

    if (_offset > 0)
    {
        r.move(_visibleHeaders.size() + 1, _geometry.width - lessMessage.size());
        r << lessMessage;
    }

    if (_offset + visibleLines() < _lineCount)
    {
        r.move(getmaxy(_window) - 1, _geometry.width - moreMessage.size());
        r << moreMessage;
    }
}

EmailView::PartList::iterator EmailView::selectedPart()
{
    for (size_t index = 0; index < _partsEndLine.size(); ++index)
    {
        if (_selectedIndex < _partsEndLine[index])
            return _parts.begin() + index;
    }
    return _parts.begin();
}

void EmailView::saveSelectedPart()
{
    MessagePartSaveVisitor saver;
    (*selectedPart())->accept(saver);
}

void EmailView::toggleSelectedPartFolding()
{
    PartList::iterator part = selectedPart();
    (*part)->folded = !(*part)->folded;

    if (part != _parts.begin())
        _selectedIndex = _partsEndLine[std::distance(_parts.begin(), part) - 1];
    else
        _selectedIndex = 0;

    makeSelectionVisible();

    StatusBar::instance().update();
    StatusBar::instance().refresh();
}

int EmailView::visibleLines() const
{
    return getmaxy(_window) - _visibleHeaders.size() - 1;
}

int EmailView::lineCount() const
{
    return _lineCount;
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

