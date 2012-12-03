/* ner: src/ncurses.cc
 *
 * Copyright (c) 2010, 2012 Michael Forney
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

#include <cassert>

#include "ncurses.hh"

namespace NCurses
{
    const size_t cchar_buffer_size = 256;
    const int state_index = std::ios_base::xalloc();

    void initialize(const ColorMap & color_map)
    {
        /* Initialize the screen */
        initscr();

        /* Initialize colors */
        if (has_colors())
            start_color();

        /* Enable raw input */
        raw();

        /* Do not echo input */
        noecho();

        /* Enable keyboard mapping */
        keypad(stdscr, TRUE);

        /* Make the cursor invisible */
        curs_set(0);
        refresh();

        /* Initialize colors from color map. */
        for (auto color : color_map)
            init_pair(color.first, color.second.foreground, color.second.background);
    }

    void cleanup()
    {
        endwin();
    }

    template <>
    Buffer<char>::Buffer(std::vector<cchar_t> & cchar_buffer, State & render_state)
        : std::streambuf(), _conversion_state(), _cchars(cchar_buffer), _state(render_state)
    {
        typedef std::codecvt<wchar_t, char, std::mbstate_t> codecvt;

        std::locale locale(getloc());

        assert(std::has_facet<codecvt>(locale));
        _codec = &std::use_facet<codecvt>(locale);
    }

    template <>
    bool Buffer<char>::get_wc(const char *& from, const char * from_end, wchar_t & wc)
    {
        /* Unused, but we need it as an argument to codecvt::in. */
        wchar_t * wc_next;

        auto result = _codec->in(_conversion_state, from, from_end, from,
            &wc, &wc + 1, wc_next);

        return result == std::codecvt_base::ok
            || result == std::codecvt_base::partial && &wc + 1 == wc_next;
    }

    Renderer::Renderer(WINDOW * window, bool erase)
        : Stream<char>(_cchars, _state), _window(window), _off_screen(false), _row(0)
    {
        _cchars.reserve(cchar_buffer_size);
        wmove(_window, 0, 0);

        if (erase)
            werase(_window);
    }

    Renderer::~Renderer()
    {
        /* Add whatever is in the render buffer to the screen. */
        render();
    }

    void Renderer::set_color(Color color)
    {
        _state.color = color;
    }

    void Renderer::set_line_attributes(attr_t attributes)
    {
        wchgat(_window, -1, attributes, 0, NULL);
        _state.attributes = attributes;
    }

    void Renderer::set_max_width(size_t max_width)
    {
        _state.max_width = max_width;
    }

    void Renderer::move(int y, int x)
    {
        render();

        set_off_screen(wmove(_window, _row = y, x) == ERR);
    }

    void Renderer::advance(size_t amount)
    {
        if (_off_screen)
            return;

        move(getcury(_window), getcurx(_window) + amount);
    }

    void Renderer::skip(size_t amount)
    {
        if (_off_screen)
            return;

        if (amount > _state.max_width - _state.display_width)
            set_off_screen();
        else
        {
            advance(_state.display_width + amount);
            _state.max_width -= amount;
        }
    }

    void Renderer::next_line()
    {
        move(++_row, 0);
    }

    bool Renderer::off_screen() const
    {
        return _off_screen;
    }

    int Renderer::row() const
    {
        return _row;
    }

    size_t Renderer::render()
    {
        size_t columns = _state.display_width;

        if (!_cchars.empty())
        {
            if (getcurx(_window) + _state.display_width > getmaxx(_window))
                set_off_screen();

            wadd_wchnstr(_window, _cchars.data(), _cchars.size());
            _cchars.clear();
            _state.display_width = 0;
        }

        return columns;
    }

    void Renderer::add_cut_off_indicator()
    {
        render();

        if (_off_screen)
            mvwaddch(_window, getcury(_window), getmaxx(_window) - 1,
                '$' | _state.attributes | COLOR_PAIR(Color::CutOffIndicator));
    }

    void Renderer::set_off_screen(bool off)
    {
        _off_screen = off;

        if (off)
            setstate(std::ios_base::failbit);
        else
            clear();
    }

    std::ios_base & acs(std::ios_base & ios)
    {
        void * pointer = ios.pword(state_index);
        if (pointer)
            static_cast<State *>(pointer)->attributes |= A_ALTCHARSET;

        return ios;
    }

    std::ios_base & noacs(std::ios_base & ios)
    {
        void * pointer = ios.pword(state_index);
        if (pointer)
            static_cast<State *>(pointer)->attributes &= ~A_ALTCHARSET;

        return ios;
    }

    std::ios_base & clear_attr(std::ios_base & ios)
    {
        void * pointer = ios.pword(state_index);
        if (pointer)
            static_cast<State *>(pointer)->attributes = 0;

        return ios;
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

