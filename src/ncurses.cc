/* ner: src/ncurses.cc
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

#include "ncurses.hh"

namespace NCurses
{
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

    CutOffException::~CutOffException() throw ()
    {
    }

    const char * CutOffException::what() throw ()
    {
        return "Line cut off past screen";
    }

    void checkMove(WINDOW * window, int x)
    {
        if (wmove(window, getcury(window), x) == ERR)
            throw CutOffException();
    }

    void addCutOffIndicator(WINDOW * window, attr_t attributes)
    {
        wmove(window, getcury(window), getmaxx(window) - 1);
        waddch(window, '$' | attributes | COLOR_PAIR(Color::CutOffIndicator));
    }

    int addPlainString(WINDOW * window, const std::string & string,
        attr_t attributes, Color color, int maxLength)
    {
        return addPlainString(window, string.begin(), string.end(), attributes, color, maxLength);
    }

    int addPlainString(WINDOW * window, const char * string,
        attr_t attributes, Color color, int maxLength)
    {
        return addPlainString(window, string, string + std::strlen(string), attributes, color, maxLength);
    }

    int addUtf8String(WINDOW * window, const char * string,
        attr_t attributes, Color color, int maxLength)
    {
        mbstate_t state = { 0 };

        int length = strlen(string);

        cchar_t displayCharacters[length + 1];
        int displayIndex = 0;
        int displayLength = 0;

        wchar_t wideCharacters[CCHARW_MAX + 1];
        wchar_t wideCharacter;
        int wideIndex = 0;

        for (int position = 0; position < length;)
        {
            int bytesRead = std::mbrtowc(&wideCharacter,
                string + position, length - position, &state);

            position += bytesRead;

            if (bytesRead < 0)
                break;

            int width = wcwidth(wideCharacter);

            if (width > 0)
                displayLength += width;

            if (displayLength > maxLength)
                break;

            /* We found a new spacing character, set the next cchar_t */
            if ((width > 0 && wideIndex > 0) || wideIndex == CCHARW_MAX)
            {
                wideCharacters[wideIndex] = L'\0';
                setcchar(&displayCharacters[displayIndex++], wideCharacters,
                    attributes, color, NULL);

                /* Start the next display character */
                wideIndex = 0;
            }
            else if (width == 0 && wideIndex == 0)
                wideCharacters[wideIndex++] = L' ';
            else if (width < 0)
                break;

            wideCharacters[wideIndex++] = wideCharacter;
        }

        if (wideIndex > 0)
        {
            wideCharacters[wideIndex] = L'\0';
            setcchar(&displayCharacters[displayIndex++], wideCharacters,
                attributes, color, NULL);
        }

        /* Set the NULL cchar_t */
        wideCharacters[0] = L'\0';
        setcchar(&displayCharacters[displayIndex], wideCharacters, 0, 0, NULL);

        wadd_wchnstr(window, displayCharacters, displayIndex);

        return displayLength;
    }

    int addChar(WINDOW * window, chtype character, int attributes, Color color)
    {
        character |= attributes | COLOR_PAIR(color);
        waddchnstr(window, &character, 1);
        return 1;
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

