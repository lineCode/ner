/* ner: src/notmuch.cc
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

#include <stdexcept>
#include <glib-object.h>

#include "notmuch.hh"

GKeyFile * _config = NULL;

namespace Notmuch
{
    notmuch_database_t * Database::_shared_read_only_database = nullptr;

    Database::Database(notmuch_database_mode_t mode)
        : _owner(true)
    {
        _database = open(mode);
    }

    Database::Database()
        : _owner(false)
    {
        if (_shared_read_only_database == nullptr)
            _shared_read_only_database = open(NOTMUCH_DATABASE_MODE_READ_ONLY);

        _database = _shared_read_only_database;
    }

    Database::~Database()
    {
        if (_owner)
            notmuch_database_close(_database);
    }

    Database::operator notmuch_database_t *() const
    {
        return _database;
    }

    notmuch_database_t * Database::open(notmuch_database_mode_t mode)
    {
        notmuch_database_t * database;
        notmuch_status_t status = notmuch_database_open(path(), mode, &database);

        if (status != NOTMUCH_STATUS_SUCCESS)
            throw std::runtime_error("Open database failed: "
                + std::string(notmuch_status_to_string(status)));

        return database;
    }

    InvalidThreadException::InvalidThreadException(const std::string & threadId)
        : _id(threadId)
    {
    }

    InvalidThreadException::~InvalidThreadException() throw()
    {
    }

    const char * InvalidThreadException::what() const throw()
    {
        return ("Cannot find thread with ID: " + _id).c_str();
    }

    InvalidMessageException::InvalidMessageException(const std::string & messageId)
        : _id(messageId)
    {
    }

    InvalidMessageException::~InvalidMessageException() throw()
    {
    }

    const char * InvalidMessageException::what() const throw()
    {
        return ("Cannot find message with ID: " + _id).c_str();
    }

    Thread::Thread(notmuch_thread_t * thread)
        : id(notmuch_thread_get_thread_id(thread)),
            subject(notmuch_thread_get_subject(thread) ? : "(null)"),
            authors(notmuch_thread_get_authors(thread) ? : "(null)"),
            totalMessages(notmuch_thread_get_total_messages(thread)),
            matchedMessages(notmuch_thread_get_matched_messages(thread)),
            newestDate(notmuch_thread_get_newest_date(thread)),
            oldestDate(notmuch_thread_get_oldest_date(thread))
    {
        notmuch_tags_t * tagIterator;

        for (tagIterator = notmuch_thread_get_tags(thread);
            notmuch_tags_valid(tagIterator);
            notmuch_tags_move_to_next(tagIterator))
        {
            tags.insert(notmuch_tags_get(tagIterator));
        }

        notmuch_tags_destroy(tagIterator);
    }

    Message::Message(notmuch_message_t * message)
        : id(notmuch_message_get_message_id(message)),
            filename(notmuch_message_get_filename(message)),
            date(notmuch_message_get_date(message)),
            matched(notmuch_message_get_flag(message, NOTMUCH_MESSAGE_FLAG_MATCH)),
            headers{
                {"From",    notmuch_message_get_header(message, "From")     ? : "(null)"},
                {"To",      notmuch_message_get_header(message, "To")       ? : "(null)"},
                {"Subject", notmuch_message_get_header(message, "Subject")  ? : "(null)"},
            }
    {
        /* Tags */
        notmuch_tags_t * tagIterator;

        for (tagIterator = notmuch_message_get_tags(message);
            notmuch_tags_valid(tagIterator);
            notmuch_tags_move_to_next(tagIterator))
        {
            tags.insert(notmuch_tags_get(tagIterator));
        }

        notmuch_tags_destroy(tagIterator);

        /* Replies */
        notmuch_messages_t * messages;

        for (messages = notmuch_message_get_replies(message);
            notmuch_messages_valid(messages);
            notmuch_messages_move_to_next(messages))
        {
            replies.push_back(Message(notmuch_messages_get(messages)));
        }

        notmuch_messages_destroy(messages);
    }

    GKeyFile * config()
    {
        return _config;
    }

    void setConfig(const std::string & path)
    {
        if (_config)
            g_object_unref(_config);

        _config = g_key_file_new();

        if (!g_key_file_load_from_file(_config, path.c_str(), G_KEY_FILE_NONE, NULL))
            _config = NULL;
    }

    const char * path()
    {
        return g_key_file_get_string(_config, "database", "path", NULL);
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

