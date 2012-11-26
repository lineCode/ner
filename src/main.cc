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
#include <clocale>
#include <csignal>
#include <unistd.h>
#include <gmime/gmime.h>

#include "notmuch.hh"
#include "ner.hh"
#include "view_manager.hh"
#include "search_list_view.hh"
#include "identity_manager.hh"
#include "ner_config.hh"

const std::string notmuchConfigFile(".notmuch-config");

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
    std::setlocale(LC_ALL, "");

    srand(time(NULL));
    g_mime_init(0);

    const char * environmentConfigPath = std::getenv("NOTMUCH_CONFIG");
    std::string defaultConfigPath(std::string(std::getenv("HOME")) + "/" + notmuchConfigFile);

    const std::string & configPath = (environmentConfigPath != NULL &&
        access(environmentConfigPath, R_OK) == 0) ? environmentConfigPath : defaultConfigPath;

    NCurses::initialize();

    std::signal(SIGWINCH, &resize);

    try
    {
        Notmuch::setConfig(configPath);
        NerConfig::instance().load();

        if (NerConfig::instance().refresh_view)
            /* Refresh the view every minute (or when the user presses a key). */
            timeout(60000);

        Ner ner;

        std::shared_ptr<View> searchListView(new SearchListView());
        ner.viewManager().addView(searchListView);

        ner.run();
    }
    catch (const std::exception & e)
    {
        NCurses::cleanup();
        throw;
    }

    NCurses::cleanup();

    g_mime_shutdown();

    return EXIT_SUCCESS;
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

