/* ner: notmuch/tree.hh
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

#ifndef NER_NOTMUCH_TREE_H
#define NER_NOTMUCH_TREE_H 1

#include <vector>
#include <cstddef>
#include <notmuch.h>

#include "notmuch/util.hh"

namespace Notmuch
{
    template <typename T, bool Const>
    class TreeIterator;

    template <typename T>
    class Tree
    {
        public:
            struct Node
            {
                T value;
                Tree<T> branch;
            };

            std::vector<Node> children;

            typedef TreeIterator<T, false> iterator;
            typedef TreeIterator<T, true> const_iterator;

            iterator begin();
            iterator end();

            const_iterator cbegin() const;
            const_iterator cend() const;
    };

    template <typename T, bool Const>
    class TreeIterator
    {
        public:
            typedef typename std::conditional<Const, typename std::add_const<T>::type,
                                              T>::type value_type;
            typedef ptrdiff_t difference_type;
            typedef value_type * pointer;
            typedef value_type & reference;
            typedef std::forward_iterator_tag iterator_category;

            typedef typename std::conditional<Const, const Tree<T>, Tree<T>>::type tree_type;

            explicit TreeIterator(tree_type & tree)
                : _branches{ std::make_pair(tree.children.begin(), tree.children.end()) }
            {
            }

            TreeIterator()
            {
            }

            reference operator*()
            {
                return _branches.back().first->value;
            }

            pointer operator->()
            {
                return &operator*();
            }

            TreeIterator<T, Const> & operator++()
            {
                auto & node = *_branches.back().first;
                ++_branches.back().first;

                _branches.push_back(std::make_pair(node.branch.children.begin(),
                    node.branch.children.end()));

                /* Remove all completed branches. */
                while (!_branches.empty() && _branches.back().first == _branches.back().second)
                    _branches.pop_back();

                _last_reply = _branches.back().first + 1 == _branches.back().second;

                return *this;
            }

            bool operator==(const TreeIterator<T, Const> & other) const
            {
                return _branches.empty() == other._branches.empty();
            }

            bool operator!=(const TreeIterator<T, Const> & other) const
            {
                return !operator==(other);
            }

            bool last_in_branch() const
            {
                return _last_reply;
            }

            int level() const
            {
                return _branches.size();
            }

        private:
#if defined __GNUC__ && !__GNUC_PREREQ(4, 7)
            /* Inflexible if the type for Tree<T>::children happens to change. */
            typedef std::vector<typename tree_type::Node> children_type;
#else
            typedef decltype(tree_type::children) children_type;
#endif
            typedef typename std::conditional<Const, typename children_type::const_iterator,
                typename children_type::iterator>::type Iterator;
            std::vector<std::pair<Iterator, Iterator>> _branches;
            bool _last_reply;
    };

    template <typename T>
    typename Tree<T>::iterator Tree<T>::begin()
    {
        return Tree<T>::iterator(*this);
    }

    template <typename T>
    typename Tree<T>::iterator Tree<T>::end()
    {
        return Tree<T>::iterator();
    }

    template <typename T>
    typename Tree<T>::const_iterator Tree<T>::cbegin() const
    {
        return Tree<T>::const_iterator(*this);
    }

    template <typename T>
    typename Tree<T>::const_iterator Tree<T>::cend() const
    {
        return Tree<T>::const_iterator();
    }
}

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

