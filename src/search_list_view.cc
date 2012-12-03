/* ner: src/search_list_view.cc
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

#include "search_list_view.hh"
#include "view_manager.hh"
#include "search_view.hh"
#include "ncurses.hh"
#include "ner_config.hh"

using namespace Notmuch;

const int searchNameWidth = 15;
const int searchTermsWidth = 30;

SearchListView::SearchListView(const View::Geometry & geometry)
    : LineBrowserView(geometry),
        _searches(NerConfig::instance().searches)
{
    /* Key Sequences */
    addHandledSequence("\n", std::bind(&SearchListView::openSelectedSearch, this));
}

SearchListView::~SearchListView()
{
}

void SearchListView::update()
{
    using namespace NCurses;

    Renderer r(_window);

    if (_offset > _searches.size())
        return;

    for (auto search = _searches.begin() + _offset;
        search != _searches.end() && !r.off_screen(); ++search, r.next_line())
    {
        bool selected = r.row() + _offset == _selectedIndex;

        r.set_line_attributes(selected ? A_REVERSE : 0);

        /* Search Name */
        r.set_max_width(searchNameWidth - 1);
        r << styled(search->name, Color::SearchListViewName);
        r.advance(searchNameWidth);

        /* Search Terms */
        r.set_max_width(searchTermsWidth - 1);
        r << styled(search->query, Color::SearchListViewTerms);
        r.advance(searchTermsWidth);

        /* Number of Results */
        notmuch_query_t * query = notmuch_query_create(Database(), search->query.c_str());
        r << set_color(Color::SearchListViewResults)
            << notmuch_query_count_messages(query) << " results";
        notmuch_query_destroy(query);

        r.add_cut_off_indicator();
    }
}

std::vector<std::string> SearchListView::status() const
{
    std::ostringstream searchPosition;

    if (_searches.size() > 0)
        searchPosition << "search " << (_selectedIndex + 1) << " of " <<
            _searches.size();
    else
        searchPosition << "no configured searches";

    return std::vector<std::string>{ searchPosition.str() };
}

int SearchListView::lineCount() const
{
    return _searches.size();
}

void SearchListView::openSelectedSearch()
{
    ViewManager::instance().addView(std::make_shared<SearchView>(
        _searches.at(_selectedIndex).query));
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

