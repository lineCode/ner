/* ner: src/identity_manager.cc
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

#include "identity_manager.hh"

#include "maildir.hh"
#include "notmuch/config.hh"

namespace YAML
{

template<>
struct convert<Identity>
{
    static bool decode(const YAML::Node & node, Identity & identity)
    {
         identity.name = node["name"].as<std::string>();
         identity.email = node["email"].as<std::string>();

        if (node["signature"])
            identity.signaturePath = node["signature"].as<std::string>();
        else
            identity.signaturePath.clear();

        if (node["send"])
            identity.sendCommand = node["send"].as<std::string>();

        std::string tagPrefix = "tag:the-ner.org,2010:";

        identity.sentMail.reset();
        auto& sentMailNode = node["sent_mail"];
        if (sentMailNode and sentMailNode.Tag() == tagPrefix + "maildir")
        {
            std::string sentMailPath = sentMailNode.as<std::string>();
            identity.sentMail = std::make_shared<Maildir>(sentMailPath);
        }
        return true;
    }
};

}

IdentityManager & IdentityManager::instance()
{
    static IdentityManager * manager = NULL;

    if (!manager)
        manager = new IdentityManager();

    return *manager;
}

IdentityManager::IdentityManager()
{
}

IdentityManager::~IdentityManager()
{
}

void IdentityManager::load(const YAML::Node & node)
{
    _identities.clear();

    if (node)
        _identities = node.as<decltype(_identities)>();
    /* Otherwise, guess identities from notmuch config */
    else
    {
        Identity identity;
        identity.name = Notmuch::Config::instance().user.name;
        identity.email = Notmuch::Config::instance().user.primary_email;
        _identities.insert(std::make_pair(identity.email, identity));

        for (auto & address : Notmuch::Config::instance().user.other_email)
        {
            identity.email = address;
            _identities.insert(std::make_pair(identity.email, identity));
        }
    }
}

void IdentityManager::setDefaultIdentity(const std::string & identity)
{
    _defaultIdentity = identity;
}

const Identity * IdentityManager::defaultIdentity() const
{
    auto identity = _identities.find(_defaultIdentity);

    if (identity == _identities.end())
        /* We couldn't find it, just use the first one */
        identity = _identities.begin();

    return &identity->second;
}

const Identity * IdentityManager::findIdentity(InternetAddress * address)
{
    std::string email = internet_address_mailbox_get_addr(INTERNET_ADDRESS_MAILBOX(address));

    for (auto identity = _identities.begin(); identity != _identities.end(); ++identity)
    {
        if (identity->second.email == email)
            return &identity->second;
    }

    return 0;
}

const Identity * IdentityManager::findIdentity(const std::string & name)
{
    auto identity = _identities.find(name);

    if (identity != _identities.end())
        return &identity->second;

    return 0;
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

