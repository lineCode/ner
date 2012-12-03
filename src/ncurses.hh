/* ner: src/ncurses.hh
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

#ifndef NER_NCURSES_H
#define NER_NCURSES_H 1

#include "config.h"

#include <limits>
#include <algorithm>
#include <locale>
#include <vector>
#include <iostream>
#include <cstring>

#if HAVE_NCURSESW_NCURSES_H
#   include <ncursesw/ncurses.h>
#elif HAVE_NCURSES_NCURSES_H
#   include <ncurses/ncurses.h>
#elif HAVE_NCURSES_H
#   include <ncurses.h>
#else
#   include <curses.h>
#endif

#include "colors.hh"

/**
 * NCurses utility functions
 */
namespace NCurses
{
    extern const int state_index;

    /**
     * Initialize the screen for ncurses.
     */
    void initialize(const ColorMap & color_map);

    /**
     * Return screen to normal state.
     */
    void cleanup();

    /**
     * Get the character part of a ncurses character.
     */
    inline char chchar(chtype c)
    {
        return static_cast<char>(c & A_CHARTEXT);
    }

    /**
     * Get the attribute part of a ncurses character.
     */
    inline attr_t chattr(chtype c)
    {
        return static_cast<attr_t>(c & A_ATTRIBUTES);
    }

    /**
     * The current of the render buffer.
     */
    struct State
    {
        Color color = Color::None;
        attr_t attributes = 0;

        size_t max_width = std::numeric_limits<size_t>::max();
        size_t display_width = 0;
    };

    /**
     * An ncurses stream buffer.
     *
     * This class converts its input into complex characters for rendering to
     * the screen with wadd_wchnstr.
     */
    template <typename CharT, typename Traits = std::char_traits<CharT>,
        typename Allocator = std::allocator<CharT>>
    class Buffer : public std::basic_streambuf<CharT, Traits>
    {
        public:
            Buffer(std::vector<cchar_t> & cchar_buffer, State & render_state);

        protected:
            virtual std::streamsize xsputn(const CharT * s, std::streamsize count)
            {
                const CharT * end = s + count;

                wchar_t wcharacters[CCHARW_MAX + 1];
                int wc_pos = 0;

                wchar_t wc;
                cchar_t cchar;

                /* Unused, but we need it as an argument to codecvt::in. */
                wchar_t * wc_next;

                while (s < end)
                {
                    if (!get_wc(s, end, wc))
                        break;

                    int width = wcwidth(wc);

                    if (width > 0)
                        _state.display_width += width;

                    if (_state.display_width > _state.max_width)
                        break;

                    /* We found a new spacing character, set the next cchar_t. */
                    if ((width > 0 && wc_pos > 0) || wc_pos == CCHARW_MAX)
                    {
                        wcharacters[wc_pos] = L'\0';
                        setcchar(&cchar, wcharacters, _state.attributes, _state.color, NULL);
                        _cchars.push_back(cchar);

                        /* Start the next display character */
                        wc_pos = 0;
                    }
                    /* setcchar requires that the first character of our string is a
                     * spacing character. */
                    else if (width == 0 && wc_pos == 0)
                        wcharacters[wc_pos++] = L' ';
                    /* setcchar can't deal with negative spacing characters. */
                    else if (width < 0)
                        break;

                    wcharacters[wc_pos++] = wc;
                }

                /* Finish up the last cchar_t. */
                if (wc_pos > 0)
                {
                    wcharacters[wc_pos] = L'\0';
                    setcchar(&cchar, wcharacters, _state.attributes, _state.color, NULL);
                    _cchars.push_back(cchar);
                }

                return count - (end - s);
            }

        private:
            bool get_wc(const CharT *& from, const CharT * from_end, wchar_t & wc);

            const std::codecvt<wchar_t, CharT, std::mbstate_t> * _codec;
            std::mbstate_t _conversion_state;

            std::vector<cchar_t> & _cchars;
            State & _state;
    };

    /**
     * An ncurses output stream.
     *
     * This stream is just a normal output stream that uses an NCurses::Buffer
     * as its buffer.
     */
    template <typename CharT, typename Traits = std::char_traits<CharT>>
    class Stream : public std::basic_ostream<CharT, Traits>
    {
        public:
            Stream(std::vector<cchar_t> & cchar_buffer, State & render_state)
                : std::basic_ostream<CharT, Traits>(&_buffer), _buffer(cchar_buffer, render_state)
            {
                /* Add State object to ios_base internal storage. */
                this->pword(state_index) = &render_state;
            }

        private:
            Buffer<CharT, Traits> _buffer;
    };

    /**
     * Provides a simple interface for writing to the window using ncurses.
     *
     * This class makes it easy to navigate around the window and print
     * formatted output in certain areas.
     */
    class Renderer : public Stream<char>
    {
        public:
            /**
             * Constructs a new Renderer which works on window.
             */
            Renderer(WINDOW * window, bool erase = true);
            ~Renderer();

            /**
             * Sets the attributes for the renderer output, and every character
             * left on the line.
             */
            void set_line_attributes(attr_t attributes);
            void set_color(Color color = Color::None);
            void set_max_width(size_t width = std::numeric_limits<size_t>::max());

            /**
             * Moves the renderer to the specified coordinates.
             */
            void move(int y, int x);

            /**
             * Advances the renderer some column amount from the last movement
             * (call to move or advance) location.
             */
            void advance(size_t amount);

            /**
             * Skips some columns from the end of the last output.
             */
            void skip(size_t amount = 1);

            /**
             * Advances the renderer to the next line of the window.
             */
            void next_line();

            /**
             * Checks whether we tried to render some text that would not fit on the screen.
             */
            bool off_screen() const;

            /**
             * Returns the current row in the window that the Renderer is on.
             */
            int row() const;

            /**
             * Renders the current output to the screen, flushing the buffer.
             */
            size_t render();

            /**
             * Adds a cut off indicator to the rightmost column of the current
             * line if some output would not fit on the screen.
             */
            void add_cut_off_indicator();

        private:
            void set_off_screen(bool off = true);

            std::vector<cchar_t> _cchars;
            State _state;

            WINDOW * _window;
            bool _off_screen;
            int _row;
    };

    /* State Manipulators */
    struct SetColor { Color color; };
    struct EnableAttr { attr_t attributes; };
    struct DisableAttr { attr_t attributes; };
    struct Ch { chtype c; };

    template <typename V>
    struct Styled
    {
        const V & value;
        Color color;
        attr_t attributes;
    };

    /**
     * Change the color in the subsequent output.
     */
    inline SetColor set_color(Color color = Color::None)
    {
        return { color };
    }

    /**
     * Enable attributes in the subsequent output.
     */
    inline EnableAttr enable_attr(attr_t attributes)
    {
        return { attributes };
    }

    /**
     * Disable attributes in the subsequent output.
     */
    inline DisableAttr disable_attr(attr_t attributes)
    {
        return { attributes };
    }

    /**
     * Render a preformatted character.
     */
    inline Ch ch(chtype c)
    {
        return { c };
    }

    /**
     * Render a value with the specified color and attributes.
     */
    template <typename V>
    inline Styled<V> styled(const V & value, Color color, attr_t attributes = 0)
    {
        return Styled<V>{ value, color, attributes };
    }

    /**
     * Enable the attribute A_ALTCHARSET in the subsequent output.
     *
     * This is useful for outputting ACS_* characters.
     */
    std::ios_base & acs(std::ios_base & ios);

    /**
     * Disable the attribute A_ALTCHARSET in the subsequent output.
     */
    std::ios_base & noacs(std::ios_base & ios);

    /**
     * Clears all attributes in the subsequent output.
     */
    std::ios_base & clear_attr(std::ios_base & ios);

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> & ios, SetColor s)
    {
        void * pointer = ios.pword(state_index);
        if (pointer)
            static_cast<State *>(pointer)->color = s.color;
        return ios;
    }

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> & ios, EnableAttr e)
    {
        void * pointer = ios.pword(state_index);
        if (pointer)
            static_cast<State *>(pointer)->attributes |= e.attributes;
        return ios;
    }

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> & ios, DisableAttr d)
    {
        void * pointer = ios.pword(state_index);
        if (pointer)
            static_cast<State *>(pointer)->attributes &= ~d.attributes;
        return ios;
    }

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> & ios, Ch c)
    {
        return ios << enable_attr(chattr(c.c)) << chchar(c.c) << disable_attr(chattr(c.c));
    }

    template <typename CharT, typename Traits, typename V>
    std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> & ios, const Styled<V> & s)
    {
        return ios << enable_attr(s.attributes) << set_color(s.color)
            << s.value << disable_attr(s.attributes) << set_color();
    }
}

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

