/*
** Copyright (C) 2015 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: source file of IniParser ut.
*/

#include "common/ini_parser.h"
#include "gtest/gtest.h"

TEST(IniParserTest, Empty)
{
    IniParser parser("");
}

TEST(IniParserTest, Basic)
{
    std::string str =
        "; for test                              \r\n"
        " [Section1]    \r\n"
        "key1=value1\r\n"
        "key2 = value2\r\n";
    IniParser parser(str);
    EXPECT_TRUE(parser.Parse());
    std::string value;
    EXPECT_TRUE(parser.ReadItem("Section1", "key1", &value));
    EXPECT_EQ("value1", value);
    EXPECT_TRUE(parser.ReadItem("Section1", "key2", &value));
    EXPECT_EQ("value2", value);
}

TEST(IniParserTest, MultipleSection)
{
    std::string str =
        "; for test                              \r\n"
        " [Section1]    \r\n"
        "key1=value1\r\n"
        "; for test                              \r\n"
        "key2 = value2\r\n"
        " [Section2]    \r\n"
        "key1=value1\r\n"
        "; for test                              \r\n"
        "key2 = value2\r\n";
    IniParser parser(str);
    EXPECT_TRUE(parser.Parse());
    std::string value;
    EXPECT_TRUE(parser.ReadItem("Section2", "key1", &value));
    EXPECT_EQ("value1", value);
    EXPECT_TRUE(parser.ReadItem("Section2", "key2", &value));
    EXPECT_EQ("value2", value);
}

TEST(IniParserTest, Iterator)
{
    std::string str =
        "; for test                              \r\n"
        " [Section1]    \r\n"
        "key1=value1\r\n"
        "; for test                              \r\n"
        "key2 = value2\r\n"
        " [Section2]    \r\n"
        "key1=value1\r\n"
        "; for test                              \r\n"
        "key2 = value2\r\n";
    IniParser parser(str);
    EXPECT_TRUE(parser.Parse());

    std::string section;
    std::string key;
    std::string value;

    // Normally, use iterators in this way.
    IniParser::SectionIterator it1 = parser.CreateSectionIterator();
    while (it1.Valid()) {
        it1.Name(&section);
        IniParser::ItemIterator it2 = it1.CreateItemIterator();
        while (it2.Valid()) {
            it2.Item(&key, &value);
            it2.Next();
        }
        it1.Next();
    }

    // For test only.
    it1 = parser.CreateSectionIterator();
    EXPECT_TRUE(it1.Valid());
    it1.Name(&section);
    EXPECT_EQ("Section1", section);
    IniParser::ItemIterator it2 = it1.CreateItemIterator();
    EXPECT_TRUE(it2.Valid());
    it2.Item(&key, &value);
    EXPECT_EQ("key1", key);
    EXPECT_EQ("value1", value);
    it2.Next();
    EXPECT_TRUE(it2.Valid());
    it2.Item(&key, &value);
    EXPECT_EQ("key2", key);
    EXPECT_EQ("value2", value);
    it2.Next();
    EXPECT_TRUE(!it2.Valid());
    // next section
    it1.Next();
    EXPECT_TRUE(it1.Valid());
    it1.Name(&section);
    EXPECT_EQ("Section2", section);
    it2 = it1.CreateItemIterator();
    EXPECT_TRUE(it2.Valid());
    it2.Item(&key, &value);
    EXPECT_EQ("key1", key);
    EXPECT_EQ("value1", value);
    it2.Next();
    EXPECT_TRUE(it2.Valid());
    it2.Item(&key, &value);
    EXPECT_EQ("key2", key);
    EXPECT_EQ("value2", value);
    it2.Next();
    EXPECT_TRUE(!it2.Valid());
    // next section
    it1.Next();
    EXPECT_TRUE(!it1.Valid());
}
