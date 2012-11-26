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

void operator>>(const YAML::Node & node, ColorPair & color_pair)
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

    std::string foreground = node.FindValue("fg")->to<std::string>();
    std::string background = node.FindValue("bg")->to<std::string>();

    color_pair.foreground = ncursesColors.at(foreground);
    color_pair.background = ncursesColors.at(background);
}

void operator>>(const YAML::Node & node, Search & search)
{
    node.FindValue("name")->Read(search.name);
    node.FindValue("query")->Read(search.query);
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

NerConfig::~NerConfig()
{
}

void NerConfig::load()
{
    /* Reset configuration to default values. */
    sort_mode = NOTMUCH_SORT_NEWEST_FIRST;
    refresh_view = true;
    add_signature_dashes = true;
    commands = {
        { "send",   "/usr/sbin/sendmail -t" },
        { "edit",   "vim +" },
        { "html",   "elinks -dump" }
    };

    ColorMap colorMap = defaultColorMap;

    std::string configPath(std::string(getenv("HOME")) + "/" + nerConfigFile);
    std::ifstream configFile(configPath.c_str());

    YAML::Parser parser(configFile);

    YAML::Node document;
    parser.GetNextDocument(document);

    if (document.Type() != YAML::NodeType::Null)
    {
        /* Identities */
        IdentityManager::instance().load(document.FindValue("identities"));

        const YAML::Node * defaultIdentity = document.FindValue("default_identity");
        if (defaultIdentity)
            IdentityManager::instance().setDefaultIdentity(defaultIdentity->to<std::string>());

        /* General stuff */
        auto general = document.FindValue("general");

        if (general)
        {
            auto sortModeNode = general->FindValue("sort_mode");

            if (sortModeNode)
            {
                std::string sortModeStr;
                *sortModeNode >> sortModeStr;

                if (sortModeStr == std::string("oldest_first"))
                    sort_mode = NOTMUCH_SORT_OLDEST_FIRST;
                else if (sortModeStr == std::string("message_id"))
                    sort_mode = NOTMUCH_SORT_MESSAGE_ID;
                else if (sortModeStr != std::string("newest_first"))
                {
                    /* FIXME: throw? */
                }
            }

            auto refreshViewNode = general->FindValue("refresh_view");

            if (refreshViewNode)
                *refreshViewNode >> refresh_view;

            auto addSigDashesNode = general->FindValue("add_sig_dashes");

            if (addSigDashesNode)
                *addSigDashesNode >> add_signature_dashes;
        }

        /* Commands */
        const YAML::Node * commands_node = document.FindValue("commands");
        if (commands_node)
            commands_node->Read(commands);

        /* Saved Searches */
        const YAML::Node * searches_node = document.FindValue("searches");
        if (searches_node)
            searches_node->Read(searches);
        else
            searches = {
                { "New", "tag:inbox and tag:unread" },
                { "Unread", "tag:unread" },
                { "Inbox", "tag:inbox" }
            };

        /* Colors */
        const YAML::Node * colors = document.FindValue("colors");
        if (colors)
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

            for (auto name = colors->begin(), e = colors->end(); name != e; ++name)
                colorMap[colorNames.at(name.first().to<std::string>())] = name.second().to<ColorPair>();
        }
    }

    /* Initialize colors from color map. */
    for (auto color = colorMap.begin(), e = colorMap.end(); color != e; ++color)
        init_pair(color->first, color->second.foreground, color->second.background);
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

