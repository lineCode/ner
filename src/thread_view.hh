/* ner: src/thread_view.hh
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

#ifndef NER_THREAD_VIEW_H
#define NER_THREAD_VIEW_H 1

#include <vector>

#include "line_browser_view.hh"

#include "notmuch/thread.hh"
#include "notmuch/message.hh"

class ThreadView : public LineBrowserView
{
    public:
        ThreadView(const View::Geometry & geometry = View::Geometry());
        virtual ~ThreadView();

        virtual void update();
        virtual std::string name() const { return "thread-view"; }
        virtual std::vector<std::string> status() const;

        void set_thread(const std::string & id);
        void set_thread(const Notmuch::Thread & thread);

        void focus_first_unread();

        const Notmuch::Message & selectedMessage() const;
        virtual void openSelectedMessage();

        void reply();

    protected:
        virtual int lineCount() const;

        std::string _id;

    private:
        void display_message_tree(NCurses::Renderer & r,
            const Notmuch::Tree<Notmuch::Message> & tree, std::string & leading,
            unsigned & index) const;

        Notmuch::Thread _thread;
};

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

