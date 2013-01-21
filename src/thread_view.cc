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
#include "util.hh"
#include "colors.hh"
#include "ncurses.hh"
#include "view_manager.hh"
#include "message_view.hh"
#include "status_bar.hh"
#include "reply_view.hh"

#include "notmuch/database.hh"
#include "notmuch/exception.hh"

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
    using namespace NCurses;

    std::string leading;
    unsigned index = 0;

    Renderer r(_window);

    display_message_tree(r, _thread.tree, leading, index);
}

std::vector<std::string> ThreadView::status() const
{
    std::ostringstream messagePosition;

    messagePosition << "message " << (_selectedIndex + 1) << " of "
        << _thread.total_messages;

    return std::vector<std::string>{
        "thread:" + _thread.id,
        messagePosition.str()
    };
}

void ThreadView::set_thread(const std::string & id)
{
    Database database;

    /* Don't care about thread metadata. */
    _thread = database.find_thread(id, Thread::TreePart);
    focus_first_unread();
}

void ThreadView::set_thread(const Thread & thread)
{
    _thread = thread;
    focus_first_unread();
}

void ThreadView::focus_first_unread()
{
    _selectedIndex = 0;
    int message_index = 0;

    for (auto & message : _thread.tree)
    {
        if (message.tags.find("unread") != message.tags.end())
        {
            _selectedIndex = message_index;
            break;
        }

        ++message_index;
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
    auto message = _thread.tree.cbegin();
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
    return _thread.total_messages;
}

void ThreadView::display_message_tree(NCurses::Renderer & r,
    const Tree<Message> & tree, std::string & leading, unsigned & index) const
{
    using namespace NCurses;

    auto last_node = &tree.children.back();

    for (auto & node : tree.children)
    {
        bool last = &node == last_node;

        if (index >= _offset)
        {
            if (r.off_screen())
                break;

            const Message & message = node.value;

            /* Draw message line */
            bool selected = r.row() + _offset == _selectedIndex;
            bool unread = message.tags.find("unread") != message.tags.end();

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
            r << styled(relative_time(message.date), Color::ThreadViewDate);

            /* Tags */
            r.set_color(Color::ThreadViewTags);
            for (auto & tag : message.tags)
            {
                r.skip(1);
                r << tag;
            }

            r.add_cut_off_indicator();
            r.next_line();
        }

        leading.push_back(last ? ' ' : ACS_VLINE);
        display_message_tree(r, node.branch, leading, ++index);
        leading.pop_back();
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

