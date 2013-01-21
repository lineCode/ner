/* ner: src/compose_view.cc
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

#include <iostream>
#include <fstream>
#include <cstring>
#include <gmime/gmime.h>

#include "compose_view.hh"
#include "ner_config.hh"

bool ComposeFields::prompt(ComposeFields & fields)
{
    StatusBar & s = StatusBar::instance();
    return s.prompt(fields.to, "To: ", "compose-to")
        && s.prompt(fields.cc, "Cc: ", "compose-cc")
        && s.prompt(fields.bcc, "Bcc: ", "compose-bcc")
        && s.prompt(fields.subject, "Subject: ", "compose-subject")
        && s.prompt(fields.identity, "Identity: ", "compose-identity");
}

ComposeView::ComposeView(const ComposeFields & fields, const View::Geometry & geometry)
    : EmailEditView(geometry)
{
    GMimeMessage * message = g_mime_message_new(true);

    if (!fields.identity.empty())
        setIdentity(fields.identity);

    InternetAddress * from = internet_address_mailbox_new(_identity->name.c_str(),
        _identity->email.c_str());
    g_mime_message_set_sender(message, internet_address_to_string(from, true));
    g_mime_object_set_header(GMIME_OBJECT(message), "To", fields.to.c_str());
    g_mime_object_set_header(GMIME_OBJECT(message), "Cc", fields.cc.c_str());
    g_mime_object_set_header(GMIME_OBJECT(message), "Bcc", fields.bcc.c_str());
    g_mime_message_set_subject(message, fields.subject.c_str());

    std::ostringstream messageContentStream;

    /* Read the user's signature */
    if (!_identity->signaturePath.empty())
    {
        if (NerConfig::instance().add_signature_dashes)
            messageContentStream << std::endl << "-- ";

        messageContentStream << std::endl;

        std::ifstream signatureFile(_identity->signaturePath.c_str());
        messageContentStream << signatureFile.rdbuf();
    }

    std::string messageContent(messageContentStream.str());

    GMimeStream * contentStream = g_mime_stream_mem_new_with_buffer(messageContent.c_str(), messageContent.size());
    GMimePart * messagePart = g_mime_part_new_with_type("text", "plain");
    GMimeDataWrapper * contentWrapper = g_mime_data_wrapper_new_with_stream(contentStream, GMIME_CONTENT_ENCODING_DEFAULT);
    g_mime_part_set_content_object(messagePart, contentWrapper);
    g_mime_message_set_mime_part(message, GMIME_OBJECT(messagePart));

    g_object_unref(messagePart);
    g_object_unref(contentWrapper);
    g_object_unref(contentStream);

    createMessage(message);

    edit();
}

ComposeView::~ComposeView()
{
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

