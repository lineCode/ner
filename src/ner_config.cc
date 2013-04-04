/* ner: src/ner_config.cc
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

#include <fstream>
#include <yaml-cpp/yaml.h>

#include "ner_config.hh"
#include "identity_manager.hh"
#include "colors.hh"
#include "ncurses.hh"

const std::string nerConfigFile(".ner.yaml");

namespace YAML
{

template<>
struct convert<ColorPair>
{
    static bool decode(const YAML::Node & node, ColorPair & color_pair)
    {
        static const std::map<std::string, int> ncursesColors = {
            { "black",      COLOR_BLACK },
            { "red",        COLOR_RED },
            { "green",      COLOR_GREEN },
            { "yellow",     COLOR_YELLOW },
            { "blue",       COLOR_BLUE },
            { "magenta",    COLOR_MAGENTA },
            { "cyan",       COLOR_CYAN },
            { "white",      COLOR_WHITE }
        };

        std::string foreground = node["fg"].as<std::string>();
        std::string background = node["bg"].as<std::string>();

        color_pair.foreground = ncursesColors.at(foreground);
        color_pair.background = ncursesColors.at(background);
        return true;
    }
};

template<>
struct convert<Search>
{
    static bool decode(const YAML::Node & node, Search & search)
    {
        search.name = node["name"].as<std::string>();
        search.query = node["query"].as<std::string>();
        return true;
    }
};

}

const NerConfig * NerConfig::_instance = nullptr;

const NerConfig & NerConfig::instance()
{
    return *_instance;
}

NerConfig::NerConfig()
{
    _instance = this;
}

void NerConfig::load()
{
    /* Reset configuration to default values. */
    sort_mode = Notmuch::SortMode::NewestFirst;
    refresh_view = true;
    add_signature_dashes = true;
    commands = {
        { "send",   "/usr/sbin/sendmail -t" },
        { "edit",   "vim +" },
        { "html",   "elinks -dump" }
    };
    color_map = defaultColorMap;

    std::string configPath(std::string(getenv("HOME")) + "/" + nerConfigFile);
    YAML::Node document = YAML::LoadFile(configPath);

    if (document)
    {
        /* Identities */
        IdentityManager::instance().load(document["identities"]);

        if (auto defaultIdentity = document["default_identity"])
            IdentityManager::instance().setDefaultIdentity(defaultIdentity.as<std::string>());

        /* General stuff */
        if (auto general = document["general"])
        {
            auto sortModeNode = general["sort_mode"];

            if (sortModeNode)
            {
                std::string sortModeStr = sortModeNode.as<std::string>();

                if (sortModeStr == std::string("oldest_first"))
                    sort_mode = Notmuch::SortMode::OldestFirst;
                else if (sortModeStr == std::string("message_id"))
                    sort_mode = Notmuch::SortMode::MessageID;
                else if (sortModeStr == std::string("newest_first"))
                    sort_mode = Notmuch::SortMode::NewestFirst;
                else
                {
                    /* FIXME: throw? */
                }
            }

            ;

            if (auto refreshViewNode = general["refresh_view"])
                refresh_view = refreshViewNode.as<bool>();

            if (auto addSigDashesNode = general["add_sig_dashes"])
                add_signature_dashes = addSigDashesNode.as<bool>();
        }

        /* Commands */
        if (auto commands_node = document["commands"])
            commands = commands_node.as<decltype(commands)>();

        /* Saved Searches */
        if (auto searches_node = document["searches"])
            searches = searches_node.as<decltype(searches)>();
        else
            searches = {
                { "New", "tag:inbox and tag:unread" },
                { "Unread", "tag:unread" },
                { "Inbox", "tag:inbox" }
            };

        /* Colors */
        if (auto colors = document["colors"])
        {
            std::map<std::string, Color> colorNames = {
                /* General */
                { "cut_off_indicator",      Color::CutOffIndicator },
                { "more_less_indicator",    Color::MoreLessIndicator },
                { "empty_space_indicator",  Color::EmptySpaceIndicator },
                { "line_wrap_indicator",    Color::LineWrapIndicator },

                /* Status Bar */
                { "status_bar_status",          Color::StatusBarStatus },
                { "status_bar_status_divider",  Color::StatusBarStatusDivider },
                { "status_bar_message",         Color::StatusBarMessage },
                { "status_bar_prompt",          Color::StatusBarPrompt },

                /* Search View */
                { "search_view_date",                   Color::SearchViewDate },
                { "search_view_message_count_complete", Color::SearchViewMessageCountComplete },
                { "search_view_message_count_partial",  Color::SearchViewMessageCountPartial },
                { "search_view_authors",                Color::SearchViewAuthors },
                { "search_view_subject",                Color::SearchViewSubject },
                { "search_view_tags",                   Color::SearchViewTags },

                /* Thread View */
                { "thread_view_arrow",  Color::ThreadViewArrow },
                { "thread_view_date",   Color::ThreadViewDate },
                { "thread_view_tags",   Color::ThreadViewTags },

                /* Email View */
                { "email_view_header",  Color::EmailViewHeader },

                /* View View */
                { "view_view_number",   Color::ViewViewNumber },
                { "view_view_name",     Color::ViewViewName },
                { "view_view_status",   Color::ViewViewStatus },

                /* Search List View */
                { "search_list_view_name",      Color::SearchListViewName },
                { "search_list_view_terms",     Color::SearchListViewTerms },
                { "search_list_view_results",   Color::SearchListViewTerms },

                /* Message Parts */
                { "attachment_filename",        Color::AttachmentFilename },
                { "attachment_mimetype",        Color::AttachmentMimeType },
                { "attachment_filesize",        Color::AttachmentFilesize },

                /* Citation levels */
                { "citation_level_1",           Color::CitationLevel1 },
                { "citation_level_2",           Color::CitationLevel2 },
                { "citation_level_3",           Color::CitationLevel3 },
                { "citation_level_4",           Color::CitationLevel4 }
            };

            for (auto name = colors.begin(), e = colors.end(); name != e; ++name)
                color_map[colorNames.at(name->first.as<std::string>())] = name->second.as<ColorPair>();
        }
    }
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

