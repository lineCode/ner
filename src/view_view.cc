/* ner: src/view_view.cc
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

#include "view_view.hh"
#include "view_manager.hh"
#include "ncurses.hh"

#include <sstream>
#include <iterator>
#include <iomanip>

const int nameWidth = 25;

ViewView::ViewView(const View::Geometry & geometry)
    : LineBrowserView(geometry)
{
    /* Key Sequences */
    addHandledSequence("\n", std::bind(&ViewView::openSelectedView, this));
    addHandledSequence("x", std::bind(&ViewView::closeSelectedView, this));
}

ViewView::~ViewView()
{
}

void ViewView::update()
{
    using namespace NCurses;

    Renderer r(_window);

    if (_offset > lineCount())
        return;

    for (auto view = ViewManager::instance()._views.begin() + _offset,
        e = ViewManager::instance()._views.end();
        view != e && !r.off_screen(); ++view, r.next_line())
    {
        /* Don't list the ViewView */
        if (view->get() == this)
            continue;

        bool selected = r.row() + _offset == _selectedIndex;

        r.set_line_attributes(selected ? A_REVERSE : 0);

        /* Number */
        r << set_color(Color::ViewViewNumber) << std::setw(2) << r.row() + _offset << '.';

        /* Name */
        r.skip(1);
        r.set_max_width(nameWidth - 1);
        r << set_color(Color::ViewViewName) << (*view)->name();
        r.advance(nameWidth);

        /* Status */
        r.set_color(Color::ViewViewStatus);
        r.set_max_width();
        std::vector<std::string> status((*view)->status());
        if (status.size() > 0)
            r << status.at(0);
    }
}

void ViewView::unfocus()
{
    /* Close the active view (this one). We don't want multiple instances of
     * ViewView around */
    ViewManager::instance().closeActiveView();
}

int ViewView::lineCount() const
{
    return ViewManager::instance()._views.size() - 1;
}

void ViewView::openSelectedView()
{
    ViewManager::instance().openView(_selectedIndex);
}

void ViewView::closeSelectedView()
{
    /* Decrement by one to account for ViewView */
    int views = ViewManager::instance()._views.size() - 1;

    if (views > 1)
    {
        ViewManager::instance().closeView(_selectedIndex);
        _selectedIndex = std::min(_selectedIndex, views - 2);
    }
    else
        StatusBar::instance().displayMessage("This is the last view left, use Q to quit");
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

