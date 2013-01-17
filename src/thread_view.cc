/* ner: src/thread_view.cc
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
#include <iterator>

#include "thread_view.hh"
#include "notmuch.hh"
#include "util.hh"
#include "colors.hh"
#include "ncurses.hh"
#include "view_manager.hh"
#include "message_view.hh"
#include "status_bar.hh"
#include "reply_view.hh"

using namespace Notmuch;

ThreadView::ThreadView(const View::Geometry & geometry)
    : LineBrowserView(geometry)
{
    /* Key Sequences */
    addHandledSequence("\n", std::bind(&ThreadView::openSelectedMessage, this));
    addHandledSequence("r", std::bind(&ThreadView::reply, this));
}

ThreadView::~ThreadView()
{
}

void ThreadView::update()
{
    std::string leading;

    werase(_window);

    int index = 0;

    for (auto message = _topMessages.begin(), e = _topMessages.end();
        message != e && index < getmaxy(_window) + _offset;
        ++message)
    {
        index = displayMessageLine(*message, leading, (message + 1) == e, index);
    }
}

std::vector<std::string> ThreadView::status() const
{
    std::ostringstream messagePosition;

    messagePosition << "message " << (_selectedIndex + 1) << " of " << _messageCount;

    return std::vector<std::string>{
        "thread:" + _id,
        messagePosition.str()
    };
}

void ThreadView::set_thread(const std::string & id)
{
    _id = id;

    notmuch_query_t * query = notmuch_query_create(Database(), ("thread:" + id).c_str());
    notmuch_threads_t * threads = notmuch_query_search_threads(query);
    notmuch_thread_t * thread;
    notmuch_messages_t * messages;

    if (!notmuch_threads_valid(threads) || !(thread = notmuch_threads_get(threads)))
    {
        notmuch_threads_destroy(threads);
        notmuch_query_destroy(query);

        throw InvalidThreadException(id);
    }

    for (messages = notmuch_thread_get_toplevel_messages(thread);
        notmuch_messages_valid(messages);
        notmuch_messages_move_to_next(messages))
    {
        _topMessages.push_back(notmuch_messages_get(messages));
    }

    notmuch_messages_destroy(messages);
    notmuch_threads_destroy(threads);
    notmuch_query_destroy(query);

    _messageCount = notmuch_thread_get_total_messages(thread);

    _selectedIndex = 0;

    /* Find first unread message */
    int messageIndex = 0;

    for (Message::const_iterator message(_topMessages.rbegin(), _topMessages.rend()), e;
        message != e; ++message, ++messageIndex)
    {
        if (message->tags.find("unread") != message->tags.end())
        {
            _selectedIndex = messageIndex;
            break;
        }
    }

    makeSelectionVisible();
}

void ThreadView::openSelectedMessage()
{
    try
    {
        std::shared_ptr<MessageView> messageView(new MessageView());
        messageView->setMessage(selectedMessage().id);
        ViewManager::instance().addView(messageView);
    }
    catch (const InvalidMessageException & e)
    {
        StatusBar::instance().displayMessage(e.what());
    }
}

const Message & ThreadView::selectedMessage() const
{
    Message::const_iterator message(_topMessages.rbegin(), _topMessages.rend());
    std::advance(message, _selectedIndex);
    return *message;
}

void ThreadView::reply()
{
    try
    {
        ViewManager::instance().addView(std::make_shared<ReplyView>(selectedMessage().id));
    }
    catch (const InvalidMessageException & e)
    {
        StatusBar::instance().displayMessage(e.what());
    }
}

int ThreadView::lineCount() const
{
    return _messageCount;
}

uint32_t ThreadView::displayMessageLine(const Message & message,
    std::string & leading, bool last, int index)
{
    using namespace NCurses;

    if (index >= _offset)
    {
        bool selected = index == _selectedIndex;
        bool unread = message.tags.find("unread") != message.tags.end();

        Renderer r(_window, false);
        r.move(index - _offset, 0);

        attr_t attributes = 0;

        if (selected)
            attributes |= A_REVERSE;

        if (unread)
            attributes |= A_BOLD;

        r.set_line_attributes(attributes);

        r << set_color(Color::ThreadViewArrow) << acs << leading
            << chchar(last ? ACS_LLCORNER : ACS_LTEE) << noacs << '>';

        /* Sender */
        r.skip(1);
        r << set_color() << message.headers.find("From")->second;

        /* Date */
        r.skip(1);
        r << styled(relativeTime(message.date), Color::ThreadViewDate);

        /* Tags */
        r.set_color(Color::ThreadViewTags);
        for (auto & tag : message.tags)
        {
            r.skip(1);
            r << tag;
        }

        r.add_cut_off_indicator();
    }

    ++index;

    leading.push_back(last ? ' ' : ACS_VLINE);

    for (auto reply = message.replies.begin(), e = message.replies.end();
        reply != e; ++reply)
    {
        if (index >= getmaxy(_window) + _offset)
            break;

        index = displayMessageLine(*reply, leading, (reply + 1) == e, index);
    }

    leading.resize(leading.size() - 1);

    return index;
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

