/* ner: src/main.cc
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

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <locale>
#include <csignal>
#include <unistd.h>
#include <gmime/gmime.h>

#include "ner.hh"
#include "view_manager.hh"
#include "search_list_view.hh"
#include "identity_manager.hh"
#include "ner_config.hh"
#include "notmuch/config.hh"

void terminate()
{
    NCurses::cleanup();
    g_mime_shutdown();
}

void resize(int arg)
{
    endwin();
    refresh();

    ViewManager::instance().resize();
    StatusBar::instance().resize();

    refresh();

    ViewManager::instance().update();
    StatusBar::instance().update();

    ViewManager::instance().refresh();
    StatusBar::instance().refresh();

    /* Clear the -1 character */
    getch();
}

int main(int argc, char * argv[])
{
    std::locale::global(std::locale(""));

    srand(time(NULL));
    g_mime_init(0);

    std::set_terminate(&terminate);

    Notmuch::Config notmuch_config;
    notmuch_config.load();

    NerConfig config;
    config.load();

    NCurses::initialize(config.color_map);

    std::signal(SIGWINCH, &resize);

    /* Refresh the view every minute (or when the user presses a key). */
    if (config.refresh_view)
        timeout(60000);

    Ner ner;

    std::shared_ptr<View> searchListView(new SearchListView());
    ner.viewManager().addView(searchListView);

    ner.run();

    NCurses::cleanup();

    g_mime_shutdown();

    return EXIT_SUCCESS;
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

