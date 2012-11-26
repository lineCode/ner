/* ner: src/colors.cc
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

#include <ncurses.h>

#include "colors.hh"

std::map<Color, ColorPair> defaultColorMap = {
    /* General */
    { Color::CutOffIndicator,           ColorPair{ COLOR_GREEN,  COLOR_BLACK } },
    { Color::MoreLessIndicator,         ColorPair{ COLOR_BLACK,  COLOR_GREEN } },
    { Color::EmptySpaceIndicator,       ColorPair{ COLOR_CYAN,   COLOR_BLACK } },
    { Color::LineWrapIndicator,         ColorPair{ COLOR_GREEN,  COLOR_BLACK } },

    /* Status Bar */
    { Color::StatusBarStatus,           ColorPair{ COLOR_WHITE,  COLOR_BLUE } },
    { Color::StatusBarStatusDivider,    ColorPair{ COLOR_WHITE,  COLOR_BLUE } },
    { Color::StatusBarMessage,          ColorPair{ COLOR_BLACK,  COLOR_WHITE } },
    { Color::StatusBarPrompt,           ColorPair{ COLOR_WHITE,  COLOR_BLACK } },

    /* Search View */
    { Color::SearchViewDate,                    ColorPair{ COLOR_YELLOW,     COLOR_BLACK } },
    { Color::SearchViewMessageCountComplete,    ColorPair{ COLOR_GREEN,      COLOR_BLACK } },
    { Color::SearchViewMessageCountPartial,     ColorPair{ COLOR_MAGENTA,    COLOR_BLACK } },
    { Color::SearchViewAuthors,                 ColorPair{ COLOR_CYAN,       COLOR_BLACK } },
    { Color::SearchViewSubject,                 ColorPair{ COLOR_WHITE,      COLOR_BLACK } },
    { Color::SearchViewTags,                    ColorPair{ COLOR_RED,        COLOR_BLACK } },

    /* ThreadView */
    { Color::ThreadViewArrow,   ColorPair{ COLOR_GREEN,  COLOR_BLACK } },
    { Color::ThreadViewDate,    ColorPair{ COLOR_CYAN,   COLOR_BLACK } },
    { Color::ThreadViewTags,    ColorPair{ COLOR_RED,    COLOR_BLACK } },

    /* Email View */
    { Color::EmailViewHeader,   ColorPair{ COLOR_CYAN, COLOR_BLACK } },

    /* View View */
    { Color::ViewViewNumber,    ColorPair{ COLOR_CYAN,   COLOR_BLACK } },
    { Color::ViewViewName,      ColorPair{ COLOR_GREEN,  COLOR_BLACK } },
    { Color::ViewViewStatus,    ColorPair{ COLOR_WHITE,  COLOR_BLACK } },

    /* Search List View */
    { Color::SearchListViewName,    ColorPair{ COLOR_CYAN,   COLOR_BLACK } },
    { Color::SearchListViewTerms,   ColorPair{ COLOR_YELLOW, COLOR_BLACK } },
    { Color::SearchListViewResults, ColorPair{ COLOR_GREEN,  COLOR_BLACK } },

    /* Message Parts */
    { Color::AttachmentFilename,    ColorPair{ COLOR_YELLOW,  COLOR_BLACK } },
    { Color::AttachmentMimeType,    ColorPair{ COLOR_MAGENTA, COLOR_BLACK } },
    { Color::AttachmentFilesize,    ColorPair{ COLOR_GREEN,   COLOR_BLACK } },

    /* Citation levels */
    { Color::CitationLevel1,    ColorPair{ COLOR_GREEN,   COLOR_BLACK } },
    { Color::CitationLevel2,    ColorPair{ COLOR_YELLOW,  COLOR_BLACK } },
    { Color::CitationLevel3,    ColorPair{ COLOR_CYAN,    COLOR_BLACK } },
    { Color::CitationLevel4,    ColorPair{ COLOR_MAGENTA, COLOR_BLACK } }
};

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

