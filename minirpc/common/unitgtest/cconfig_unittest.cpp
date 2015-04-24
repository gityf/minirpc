#include "gtest/gtest.h"
#include "common/cconfig.h"
#include "common/csingleton.h"
CConfig *iConfig = CSingleton<CConfig>::Instance();
bool SetGetString()
{
    string key   = "test key";
    string value = "test value";
    iConfig->SetConfig(key, value);
    string retStr = iConfig->GetConfig(key);
    return retStr == value;
}

bool SetGetDefaultString()
{
    string key   = "test key";
    string value = "test value";
    iConfig->SetConfig(key, value);
    string retStr = iConfig->GetCfgStr(key, value);
    return retStr == value;
}

bool SetGetDefaultCharChar()
{
    const char* key   = "test key char";
    const char* value = "test value char char";
    iConfig->SetConfig(key, value);
    string retStr = iConfig->GetCfgStr(key, value);
    return retStr == value;
}

bool SetGetInt()
{
    int key   = 0x1234;
    int value = 0x4321;
    iConfig->SetConfig(key, value);
    int ret = iConfig->GetConfig(key);
    return ret == value;
}

bool SetGetDefaultInt()
{
    int key   = 1234;
    int value = 0x4321;
    iConfig->SetConfig(key, value);
    string keyStr = "1234";
    int ret = iConfig->GetCfgInt(keyStr, value);
    return ret == value;
}

bool SetGetDefaultCharInt()
{
    int key   = 1234;
    int value = 0x4321;
    iConfig->SetConfig(key, value);
    const char* keyPtr = "1234";
    int ret = iConfig->GetCfgInt(keyPtr, value);
    return ret == value;
}

TEST(CConfigTest, SetGetConfig)
{
    EXPECT_EQ(true, SetGetString());
    EXPECT_EQ(true, SetGetInt());  
}

TEST(CConfigTest, SetGetDefaultConfig)
{
    EXPECT_EQ(true, SetGetDefaultString());
    EXPECT_EQ(true, SetGetDefaultInt());
}

TEST(CConfigTest, SetGetDefaultConfigChar)
{
    EXPECT_EQ(true, SetGetDefaultCharChar());
    EXPECT_EQ(true, SetGetDefaultCharInt());
}
