/* ner: src/ner.cc
 *
 * Copyright (c) 2010 Michael Forney
 *
 * This file is a part of ner.
 *
 * ner is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2, as published by the Free
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

#include <iostream>
#include <ncurses.h>

#include "ner.hh"
#include "util.hh"
#include "status_bar.hh"
#include "view_manager.hh"
#include "search_view.hh"

Ner::Ner()
    : _viewManager(new ViewManager),
        _statusBar(new StatusBar)
{
}

Ner::~Ner()
{
    delete _viewManager;
}

void Ner::initializeScreen()
{
    /* Initialize the screen */
    initscr();

    /* Initialize colors */
    if (has_colors())
    {
        start_color();
        init_pair(NER_COLOR_RED,        COLOR_RED,      COLOR_BLACK);
        init_pair(NER_COLOR_GREEN,      COLOR_GREEN,    COLOR_BLACK);
        init_pair(NER_COLOR_YELLOW,     COLOR_YELLOW,   COLOR_BLACK);
        init_pair(NER_COLOR_BLUE,       COLOR_BLUE,     COLOR_BLACK);
        init_pair(NER_COLOR_CYAN,       COLOR_CYAN,     COLOR_BLACK);
        init_pair(NER_COLOR_MAGENTA,    COLOR_MAGENTA,  COLOR_BLACK);
        init_pair(NER_COLOR_WHITE,      COLOR_WHITE,    COLOR_BLACK);
    }

    /* Enable raw input */
    raw();

    /* Do not echo input */
    noecho();

    /* Enable keyboard mapping */
    keypad(stdscr, TRUE);

    /* Make the cursor invisible */
    curs_set(0);
    refresh();
}

void Ner::cleanupScreen()
{
    endwin();
}

void Ner::run()
{
    uint32_t key;

    _running = true;

    _viewManager->refresh();

    while (_running)
    {
        key = getch();

        if (!handleKeyPress(key))
            _viewManager->handleKeyPress(key);

        if (!_running) break;

        _viewManager->update();
        _viewManager->refresh();
    }
}

void Ner::quit()
{
    _running = false;
}

void Ner::search()
{
    _viewManager->addView(new SearchView(StatusBar::instance().prompt("Search: ")));
}

bool Ner::handleKeyPress(const int key)
{
    switch (key)
    {
        case 'Q':
            quit();
            break;
        case 's':
            search();
            break;
        default:
            return false;
    }

    return true;
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8
