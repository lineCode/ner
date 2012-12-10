/* ner: src/status_bar.cc
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

#include "status_bar.hh"
#include "ncurses.hh"
#include "colors.hh"
#include "view.hh"
#include "view_manager.hh"
#include "line_editor.hh"
#include "util.hh"
#include <unistd.h>

StatusBar * StatusBar::_instance = 0;

StatusBar::StatusBar()
    : _statusWindow(newwin(1, COLS, LINES - 2, 0)),
        _promptWindow(newwin(1, COLS, LINES - 1, 0)),
        _messageCleared(true)
{
    _instance = this;

    wbkgd(_statusWindow, COLOR_PAIR(Color::StatusBarStatus));

    wrefresh(_statusWindow);
    wrefresh(_promptWindow);
}

StatusBar::~StatusBar()
{
    _instance = 0;

    if (_messageClearThread.joinable())
        _messageClearThread.join();
}

void StatusBar::update()
{
    using namespace NCurses;

    const View & view = ViewManager::instance().activeView();

    Renderer r(_statusWindow);

    /* View Name */
    r << enable_attr(A_BOLD) << '[' << view.name() << ']' << clear_attr;

    /* Status */
    for (auto & status : view.status())
    {
        r.skip(1);
        r << styled('|', Color::StatusBarStatusDivider, A_BOLD);

        r.skip(1);
        r << styled(status, Color::StatusBarStatus);
    }
}

void StatusBar::refresh()
{
    wrefresh(_statusWindow);
    wrefresh(_promptWindow);
}

void StatusBar::resize()
{
    wresize(_statusWindow, 1, COLS);
    wresize(_promptWindow, 1, COLS);

    mvwin(_statusWindow, LINES - 2, 0);
    mvwin(_promptWindow, LINES - 1, 0);
}

void StatusBar::displayMessage(const std::string & message)
{
    werase(_promptWindow);
    wbkgd(_promptWindow, COLOR_PAIR(Color::StatusBarMessage));

    wmove(_promptWindow, 0, (getmaxx(_promptWindow) - message.size()) / 2);
    wattron(_promptWindow, A_BOLD);
    waddstr(_promptWindow, message.c_str());
    wattroff(_promptWindow, A_BOLD);

    wrefresh(_promptWindow);

    _messageCleared = false;

    if (_messageClearThread.joinable())
        _messageClearThread.detach();

    _messageClearThread = std::thread(std::bind(&StatusBar::delayedClearMessage, this, 1500));
}

bool StatusBar::prompt(std::string & result, const std::string & message,
    const std::string & field, const std::string & initialValue)
{
    if (!_messageCleared)
        clearMessage();

    wmove(_promptWindow, 0, 0);
    wattron(_promptWindow, COLOR_PAIR(Color::StatusBarPrompt));
    waddstr(_promptWindow, message.c_str());
    wrefresh(_promptWindow);

    LineEditor editor(_promptWindow, getcurx(_promptWindow), 0);
    bool status = editor.line(result, field, initialValue);

    wattroff(_promptWindow, COLOR_PAIR(Color::StatusBarPrompt));

    /* Clear the prompt window after we're done */
    werase(_promptWindow);
    wrefresh(_promptWindow);

    return status;
}

void StatusBar::delayedClearMessage(int delay)
{
    usleep(delay * 1000);

    if (_messageCleared)
        return;

    clearMessage();
}

void StatusBar::clearMessage()
{
    werase(_promptWindow);
    wbkgd(_promptWindow, COLOR_PAIR(Color::StatusBarPrompt));
    wrefresh(_promptWindow);
    _messageCleared = true;
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

