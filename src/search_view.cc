/* ner: src/search_view.cc
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

#include <iostream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <iterator>
#include <sched.h>

#include "search_view.hh"
#include "thread_message_view.hh"
#include "view_manager.hh"
#include "util.hh"
#include "colors.hh"
#include "ncurses.hh"
#include "status_bar.hh"

#include "notmuch/query.hh"
#include "notmuch/exception.hh"

using namespace Notmuch;

const int newestDateWidth = 13;
const int messageCountWidth = 8;
const int authorsWidth = 20;

const auto conditionWaitTime = std::chrono::milliseconds(50);

SearchView::SearchView(const std::string & search, const View::Geometry & geometry)
    : LineBrowserView(geometry),
        _searchTerms(search)
{
    _collecting = true;
    _thread = std::thread(std::bind(&SearchView::collectThreads, this));

    /* Key Sequences */
    addHandledSequence("=", std::bind(&SearchView::refreshThreads, this));
    addHandledSequence("\n", std::bind(&SearchView::openSelectedThread, this));

    std::unique_lock<std::mutex> lock(_mutex);
    while (_threads.size() < getmaxy(_window) && _collecting)
        _condition.wait_for(lock, conditionWaitTime);
}

SearchView::~SearchView()
{
    if (_thread.joinable())
    {
        _collecting = false;
        _thread.join();
    }
}

void SearchView::update()
{
    using namespace NCurses;

    Renderer r(_window);

    if (_offset > _threads.size())
        return;

    for (auto thread = _threads.begin() + _offset, e = _threads.end();
        thread != e && !r.off_screen(); ++thread, r.next_line())
    {
        bool selected = r.row() + _offset == _selectedIndex;
        bool unread = thread->tags.find("unread") != thread->tags.end();
        bool completeMatch = thread->matched_messages == thread->total_messages;

        attr_t attributes = 0;

        if (unread)
            attributes |= A_BOLD;

        if (selected)
            attributes |= A_REVERSE;

        r.set_line_attributes(attributes);

        /* Date */
        r.set_max_width(newestDateWidth - 1);
        r << styled(relative_time(thread->date), Color::SearchViewDate);
        r.advance(newestDateWidth);

        /* Message Count */
        Color message_count_color = completeMatch
            ? Color::SearchViewMessageCountComplete
            : Color::SearchViewMessageCountPartial;
        r.set_max_width(messageCountWidth - 1);
        r << set_color() << '[' << set_color(message_count_color)
            << thread->matched_messages << '/' << thread->total_messages
            << set_color() << ']';
        r.advance(messageCountWidth);

        /* Authors */
        r.set_max_width(authorsWidth - 1);
        r << styled(thread->authors, Color::SearchViewAuthors);
        r.advance(authorsWidth);

        /* Subject */
        r.set_max_width();
        r << styled(thread->subject, Color::SearchViewSubject);

        /* Tags */
        r.set_color(Color::SearchViewTags);
        for (auto & tag : thread->tags)
        {
            r.skip(1);
            r << tag;
        }

        r.add_cut_off_indicator();
    }
}

std::vector<std::string> SearchView::status() const
{
    std::ostringstream threadPosition;

    if (_threads.size() > 0)
        threadPosition << "thread " << (_selectedIndex + 1) << " of " << _threads.size();
    else
        threadPosition << "no matching threads";

    return std::vector<std::string>{
        "search-terms: \"" + _searchTerms + '"',
        threadPosition.str()
    };
}

void SearchView::openSelectedThread()
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_selectedIndex < _threads.size())
    {
        try
        {
            auto thread_view = std::make_shared<ThreadMessageView>();
            thread_view->set_thread(_threads.at(_selectedIndex).id);
            ViewManager::instance().addView(thread_view);
        }
        catch (const InvalidThreadException & e)
        {
            StatusBar::instance().displayMessage(e.what());
        }
        catch (const InvalidMessageException & e)
        {
            StatusBar::instance().displayMessage(e.what());
        }
    }
}

void SearchView::refreshThreads()
{
    /* If the thread is still going, stop it, and wait for it to return */
    if (_thread.joinable())
    {
        _collecting = false;
        _thread.join();
    }

    bool empty = _threads.empty();
    std::string selectedId;

    if (!empty)
        selectedId = _threads.at(_selectedIndex).id;

    _threads.clear();

    /* Start collecting threads in the background */
    _collecting = true;
    _thread = std::thread(std::bind(&SearchView::collectThreads, this));

    /* Locate the previously selected thread ID */
    bool found = false;
    std::unique_lock<std::mutex> lock(_mutex);

    if (empty)
        found = true;
    else
    {
        int index = 0;

        while (!found && _collecting)
        {
            for (; index < _threads.size(); ++index)
            {
                /* Stop if we found the thread ID */
                if (_threads.at(index).id == selectedId)
                {
                    found = true;
                    _selectedIndex = index;
                    break;
                }
            }

            _condition.wait_for(lock, conditionWaitTime);
        }
    }

    /* Wait until we have enough threads to fill the screen */
    while (_threads.size() - _offset < getmaxy(_window) && _collecting)
        _condition.wait_for(lock, conditionWaitTime);

    /* If we didn't find it, make sure the selected index is valid */
    if (!found)
    {
        if (_threads.size() <= _selectedIndex)
            _selectedIndex = _threads.size() - 1;
    }

    StatusBar::instance().update();
    makeSelectionVisible();
}

int SearchView::lineCount() const
{
    return _threads.size();
}

void SearchView::collectThreads()
{
    Database database;
    Query query(_searchTerms, &database);

    query.set_sort_mode(NerConfig::instance().sort_mode);

    std::unique_lock<std::mutex> lock(_mutex);
    lock.unlock();

    for (const auto & thread : query.threads())
    {
        if (!_collecting)
            break;

        lock.lock();

        _threads.push_back(thread);
        _condition.notify_one();

        lock.unlock();

        sched_yield();
    }

    _collecting = false;

    /* For cases when there are no matching threads */
    _condition.notify_one();
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

