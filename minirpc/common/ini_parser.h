/*
** Copyright (C) 2015 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: Header file of class IniParser.
*/

#ifndef _COMMON_INI_PARSER_H
#define _COMMON_INI_PARSER_H

#include <list>
#include <string>
#include <utility>

class IniParser
{
private:
    struct Section
    {
        std::string name;
        std::list<std::pair<std::string, std::string> > items;
    };

public:
    class ItemIterator
    {
    public:
        void Next();
        bool Valid();
        void Item(std::string* key, std::string* value);
    private:
        explicit ItemIterator(const Section& section);
    private:
        friend class IniParser;
        const Section* mSection;
        std::list<std::pair<std::string, std::string> >::const_iterator
            mIterator;
    };

    class SectionIterator
    {
    public:
        void Next();
        bool Valid();
        void Name(std::string* name);
        ItemIterator CreateItemIterator() const;
    private:
        explicit SectionIterator(const std::list<Section>& sections);
    private:
        friend class IniParser;
        const std::list<Section>* mSections;
        std::list<Section>::const_iterator mIterator;
    };

public:
    explicit IniParser(const std::string& buffer);
    bool Parse();
    bool ReadItem(const std::string& section, const std::string& key,
                  std::string* value);
    SectionIterator CreateSectionIterator();

private:
    static bool IniParse(const std::string& ini, std::list<Section>* sections);
    static bool ParseSection(std::list<std::string>* lines, Section* section);
    static bool IsNullLine(const std::string& line);
    static bool ParseSectionName(const std::string& line, std::string* name);
    static bool ParseItem(const std::string& line,
                          std::string* key,
                          std::string* value);

private:
    std::string mContent;
    bool mParsed;
    std::list<Section> mSections;
};

#endif // _COMMON_INI_PARSER_H
