/* ner: notmuch/util.hh
 *
 * Copyright (c) 2012 Michael Forney
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

#ifndef NER_NOTMUCH_UTIL_H
#define NER_NOTMUCH_UTIL_H 1

#include <string>
#include <memory>
#include <cassert>

namespace Notmuch
{
    /* Notmuch Pointers */

    template <typename T>
    struct Deleter
    {
        constexpr Deleter() = default;

        inline void operator()(T * object)
        {
            del(object);
        }

        typedef void (* DeleteFunction)(T * object);
        static DeleteFunction del;
    };

    template <typename T>
    using Pointer = std::unique_ptr<T, Deleter<T>>;

    template <typename T>
    Pointer<T> ptr(T * object)
    {
        return Pointer<T>(object);
    }

    /* Case-Insensitive Strings */
    template <typename CharT>
    struct CaseInsensitiveCharTraits : public std::char_traits<CharT>
    {
        static int compare(const CharT * s1, const CharT * s2, size_t n)
        {
            std::locale locale;
            assert(std::has_facet<std::ctype<CharT>>(locale));
            auto ctype = &std::use_facet<std::ctype<CharT>>(locale);
            CharT c1, c2;

            for (size_t i = 0; i < n; ++i)
            {
                c1 = ctype->tolower(s1[i]);
                c2 = ctype->tolower(s2[i]);

                if (std::char_traits<CharT>::lt(c1, c2))
                    return -1;
                else if (std::char_traits<CharT>::lt(c2, c1))
                    return 1;
            }

            return 0;
        }
    };

    typedef std::basic_string<char, CaseInsensitiveCharTraits<char>>
        CaseInsensitiveString;
}

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

