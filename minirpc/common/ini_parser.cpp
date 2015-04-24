/*
** Copyright (C) 2015 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: Source file of class IniParser.
*/

#include "common/ini_parser.h"
#include <assert.h>
#include <vector>
#include "common/cstringutils.h"
using namespace wyf;
IniParser::IniParser(const std::string& buffer)
  : mContent(buffer),
    mParsed(false)
{
}

bool IniParser::Parse()
{
    if (mParsed) {
        return true;
    }
    if (IniParse(mContent, &mSections)) {
        mParsed = true;
    }
    return mParsed;
}

bool IniParser::ReadItem(const std::string& section, const std::string& key,
                         std::string* value)
{
    assert(mParsed);

    SectionIterator it1 = SectionIterator(mSections);
    while (it1.Valid()) {
        std::string s;
        it1.Name(&s);
        if (s == section) {
            break;
        }
        it1.Next();
    }

    if (it1.Valid()) {
        ItemIterator it2 = it1.CreateItemIterator();
        while (it2.Valid()) {
            std::string k;
            std::string v;
            it2.Item(&k, &v);
            if (k == key) {
                value->swap(v);
                return true;
            }
            it2.Next();
        }
    }

    return false;
}

IniParser::SectionIterator IniParser::CreateSectionIterator()
{
    SectionIterator iterator(mSections);
    return iterator;
}

// SectionIterator
IniParser::SectionIterator::SectionIterator(const std::list<Section>& sections)
  : mSections(&sections),
    mIterator(sections.begin())
{
}

void IniParser::SectionIterator::Next()
{
    ++mIterator;
}

bool IniParser::SectionIterator::Valid()
{
    return mIterator != mSections->end();
}

void IniParser::SectionIterator::Name(std::string* name)
{
    const Section& s = *mIterator;
    *name = s.name;
}

IniParser::ItemIterator IniParser::SectionIterator::CreateItemIterator() const
{
    const Section& s = *mIterator;
    IniParser::ItemIterator it(s);
    return it;
}

// ItemIterator
IniParser::ItemIterator::ItemIterator(const Section& section)
  : mSection(&section),
    mIterator(section.items.begin())
{
}

void IniParser::ItemIterator::Next()
{
    ++mIterator;
}

bool IniParser::ItemIterator::Valid()
{
    return mIterator != mSection->items.end();
}

void IniParser::ItemIterator::Item(std::string* key, std::string* value)
{
    const std::pair<std::string, std::string>& p = *mIterator;
    *key = p.first;
    *value = p.second;
}

bool IniParser::IniParse(const std::string& ini, std::list<Section>* sections)
{
    sections->clear();

    std::vector<std::string> lines;
    CStrUitls::SplitString(ini, "\n", &lines);

    std::list<std::string> lineList(lines.begin(), lines.end());
    while (true) {
        Section section;
        if (ParseSection(&lineList, &section)) {
            sections->push_back(section);
        } else {
            break;
        }
    }

    return !sections->empty() && lineList.empty();
}

bool IniParser::ParseSection(std::list<std::string>* lines, Section* section)
{
    std::string name;
    while (!lines->empty()) {
        std::string line = lines->front();
        CStrUitls::TrimWhitespace(line, kTrimAll, &line);
        if (!IsNullLine(line)) {
            if (ParseSectionName(line, &name)) {
                lines->pop_front();
                break;
            } else {
                return false;
            }
        }
        lines->pop_front();
    }

    if (name.empty()) {
        return false;
    }
    section->name = name;

    while (!lines->empty()) {
        std::string line = lines->front();
        CStrUitls::TrimWhitespace(line, kTrimAll, &line);
        if (!IsNullLine(line)) {
            if (ParseSectionName(line, &name)) {
                break;
            }
            std::string key;
            std::string value;
            if (ParseItem(line, &key, &value)) {
                section->items.push_back(std::make_pair(key, value));
            } else {
                return false;
            }
        }
        lines->pop_front();
    }

    return true;
}

bool IniParser::IsNullLine(const std::string& line)
{
    std::string output;
    CStrUitls::TrimWhitespace(line, kTrimAll, &output);
    if (line.empty()) {
        return true;
    }
    if (line[0] == ';') {
        return true;
    }
    return false;
}

bool IniParser::ParseSectionName(const std::string& line, std::string* name)
{
    assert(!line.empty());
    name->clear();
    std::string::size_type size = line.size();
    if (line[0] == '[' && line[size - 1] == ']') {
        *name = line.substr(1, size - 2);
    }
    return !name->empty();
}

bool IniParser::ParseItem(const std::string& line,
                          std::string* key,
                          std::string* value)
{
    std::string::size_type pos = line.find_first_of('=');
    if (pos == std::string::npos) {
        return false;
    }

    std::string k = line.substr(0, pos);
    std::string v = line.substr(pos + 1);

    CStrUitls::TrimWhitespace(k, kTrimAll, key);
    CStrUitls::TrimWhitespace(v, kTrimAll, value);

    return true;
}

