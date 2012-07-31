/* ner: src/email_edit_view.cc
 *
 * Copyright (c) 2010 Michael Forney
 * Copyright (c) 2011 Maxime Coste
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
#include <cstring>
#include <cstdlib>
#include <sys/time.h>
#include <gio/gio.h>
#include <gmime/gmime.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "email_edit_view.hh"
#include "view_manager.hh"
#include "maildir.hh"
#include "ner_config.hh"
#include "util.hh"

EmailEditView::EmailEditView(const View::Geometry & geometry)
    : EmailView(geometry),
        _identity(IdentityManager::instance().defaultIdentity())
{
    setVisibleHeaders(std::vector<std::string>{
        "From",
        "To",
        "Cc",
        "Bcc",
        "Subject"
    });

    /* Key Sequences */
    addHandledSequence("e", std::bind(&EmailEditView::edit, this));
    addHandledSequence("a", std::bind(&EmailEditView::attach, this));
    addHandledSequence("d", std::bind(&EmailEditView::removeSelectedAttachment, this));
    addHandledSequence("y", std::bind(&EmailEditView::send, this));
    addHandledSequence("f", std::bind(&EmailEditView::toggleSelectedPartFolding, this));
}

EmailEditView::~EmailEditView()
{
}

void EmailEditView::edit()
{
    endwin();

    std::string command(NerConfig::instance().commands.at("edit"));
    command.push_back(' ');
    command.append(_messageFile);
    std::system(command.c_str());

    PartList partsBackup;
    partsBackup.swap(_parts); // parts will be cleared anyway

    setEmail(_messageFile);

    std::copy_if(partsBackup.begin(), partsBackup.end(),
                 std::back_inserter(_parts),
                 [] (std::shared_ptr<MessagePart>& part) -> bool { return dynamic_cast<Attachment*>(part.get()); });
}

void EmailEditView::createMessage(GMimeMessage * message)
{
    char * temporaryFilePath = strdup("/tmp/ner-compose-XXXXXX");
    int fd = mkstemp(temporaryFilePath);
    _messageFile = temporaryFilePath;
    free(temporaryFilePath);

    GMimeStream * stream = g_mime_stream_fs_new(fd);
    g_mime_object_write_to_stream(GMIME_OBJECT(message), stream);

    g_object_unref(stream);
    g_object_unref(message);
}

static std::string detectCharset(const std::string& filename)
{
    int readPipes[2];
    int writePipes[2];

    pipe(readPipes);
    pipe(writePipes);

    if (pid_t pid = fork())
    {
        close(writePipes[0]);
        close(writePipes[1]);
        close(readPipes[1]);

        char buffer[1024];
        ssize_t count = read(readPipes[0], buffer, 1024);
        close(readPipes[0]);

        int status;
        waitpid(pid, &status, 0);

        // remove EOL
        if (count > 0 and buffer[count-1] == '\n')
            --count;

        return std::string(buffer, buffer + count);
    }
    else
    {
        close(readPipes[0]);
        close(writePipes[1]);

        dup2(readPipes[1], 1);
        dup2(writePipes[0], 0);

        execlp("file", "file", "-b", "--mime-encoding", filename.c_str(), NULL);
        exit(0);
    }
}

void EmailEditView::send()
{
    std::string charset = detectCharset(_messageFile);

    /* Add the date to the message */
    FILE * file = fopen(_messageFile.c_str(), "r");
    GMimeStream * stream = g_mime_stream_file_new(file);
    GMimeParser * parser = g_mime_parser_new_with_stream(stream);
    GMimeMessage * message = g_mime_parser_construct_message(parser);
    g_object_unref(parser);
    g_object_unref(stream);

    g_mime_object_set_content_type_parameter((GMimeObject*)message->mime_part, "charset", charset.c_str());

    struct timeval timeValue;
    struct timezone timeZone;

    gettimeofday(&timeValue, &timeZone);

    g_mime_message_set_date(message, timeValue.tv_sec, -100 * timeZone.tz_minuteswest / 60);

    /* Give the message an ID */
    char hostname[256];
    gethostname(hostname, sizeof(hostname));

    std::ostringstream id;
    id << timeValue.tv_sec << '-' << "ner" << '-' << rand() % 10000 << '@' << hostname;

    g_mime_message_set_message_id(message, id.str().c_str());

    if (_parts.size() > 1)
    {
        GMimeMultipart* multipart = g_mime_multipart_new_with_subtype("mixed");
        g_mime_multipart_add(multipart, (GMimeObject*)message->mime_part);
        g_mime_message_set_mime_part(message, (GMimeObject*) multipart);

        for (auto i = _parts.begin(); i != _parts.end(); ++i)
        {
            if (not dynamic_cast<Attachment*>(i->get()))
                continue;

            Attachment& attachment = *dynamic_cast<Attachment*>(i->get());

            GMimeContentType* contentType = g_mime_content_type_new_from_string(attachment.contentType.c_str());

            GMimePart* part = g_mime_part_new_with_type(g_mime_content_type_get_media_type(contentType),
                                                        g_mime_content_type_get_media_subtype(contentType));
            g_mime_part_set_content_object(part, attachment.data);
            g_mime_part_set_content_encoding(part, GMIME_CONTENT_ENCODING_BASE64);
            g_mime_part_set_filename(part, attachment.filename.c_str());

            g_mime_multipart_add(multipart, (GMimeObject*) part);
            g_object_unref(part);
            g_object_unref(contentType);
        }
        g_object_unref(multipart);
    }

    /* Send the message */
    std::string sendCommand = _identity->sendCommand.empty() ?
        NerConfig::instance().commands.at("send") : _identity->sendCommand;
    FILE * sendMailPipe = popen(sendCommand.c_str(), "w");
    GMimeStream * sendMailStream = g_mime_stream_file_new(sendMailPipe);
    g_mime_stream_file_set_owner(GMIME_STREAM_FILE(sendMailStream), false);
    g_mime_object_write_to_stream(GMIME_OBJECT(message), sendMailStream);
    g_object_unref(sendMailStream);

    int status = pclose(sendMailPipe);

    if (status == 0)
    {
        StatusBar::instance().displayMessage("Message sent successfully");

        if (_identity->sentMail)
            if (!_identity->sentMail->addMessage(message))
                StatusBar::instance().displayMessage("Could not add message to configured mail store");

        unlink(_messageFile.c_str());
        ViewManager::instance().closeActiveView();
    }
    else
        StatusBar::instance().displayMessage("Could not send the message");

    g_object_unref(message);
}

void EmailEditView::attach()
{
    std::string filename;

    if (!StatusBar::instance().prompt(filename, "Filename: ", "attachment-file")
        || filename.empty())
    {
        return;
    }

    GError * error = NULL;
    GFile * file = g_file_new_for_path(filename.c_str());
    GFileInfo * file_info = g_file_query_info(file,
        G_FILE_ATTRIBUTE_STANDARD_TYPE "," G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
        G_FILE_QUERY_INFO_NONE, NULL, &error);

    /* Obtain unique pointer to file and file_info so they are destroyed when
     * scope ends. */
    GLibPointer file_pointer(file), file_info_pointer(file_info);

    if (error)
    {
        StatusBar::instance().displayMessage("Could not query file information");
        return;
    }
    if (g_file_info_get_file_type(file_info) != G_FILE_TYPE_REGULAR)
    {
        StatusBar::instance().displayMessage("File is not a regular file");
        return;
    }

    GMimeStream * file_stream = g_mime_stream_file_new(fopen(filename.c_str(), "r"));
    GMimeDataWrapper * data = g_mime_data_wrapper_new_with_stream(file_stream,
        GMIME_CONTENT_ENCODING_DEFAULT);

    GLibPointer file_stream_pointer(file_stream), data_pointer(data);

    _parts.push_back(std::make_shared<Attachment>(data, g_file_get_basename(file),
        g_file_info_get_content_type(file_info), g_mime_stream_length(file_stream)));
}

void EmailEditView::removeSelectedAttachment()
{
    PartList::iterator selection = selectedPart();
    if (dynamic_cast<Attachment *>(selection->get()))
        _parts.erase(selection);
}

void EmailEditView::setIdentity(const std::string & name)
{
    const Identity * identity = IdentityManager::instance().findIdentity(name);

    if (identity)
        _identity = identity;
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

