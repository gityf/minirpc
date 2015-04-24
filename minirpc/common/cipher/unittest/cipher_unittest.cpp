/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The unit test file of base64.
*/

#include "../base64.h"
#include "../cipher/md5.h"
#include "../cipher/sha1.h"
#include "../cipher/sha256.h"
#include "../cipher/sha512.h"
#include "../cipher/tdes.h"
#include "../cstringutils.h"
#include "gtest/gtest.h"

using namespace std;

void EncodeAndDecode(const std::string& s)
{
    string b;
    EXPECT_TRUE(Base64Encode(s, &b));

    string c;
    EXPECT_TRUE(Base64Decode(b, &c));
    EXPECT_EQ(c, s);
}

TEST(Base64Test, BasicTest)
{
    EncodeAndDecode("a");
    EncodeAndDecode("ab");
    EncodeAndDecode("abc");
    EncodeAndDecode("abcd");
    EncodeAndDecode("abcde");
    EncodeAndDecode("abcdef");
    EncodeAndDecode("abcdefg");
}

TEST(Base64Test, EncodeEmptyBuffer)
{
    std::string output;
    EXPECT_TRUE(Base64Encode("", &output));
}

TEST(Base64Test, DecodeEmptyString)
{
    std::string output;
    EXPECT_TRUE(Base64Decode("", &output));
}

TEST(Base64Test, DecodeWithPadding)
{
    std::string output;
    std::string s = "e===";
    EXPECT_TRUE(!Base64Decode(s, &output));

    s = "";
    EXPECT_TRUE(Base64Decode(s, &output));

    s = "abcdAFCD\r\neF==";
    EXPECT_TRUE(Base64Decode(s, &output));
    EXPECT_EQ((size_t)7, output.size());

    s = "abcdAFCD\r\neF==\r\n\r\n";
    EXPECT_TRUE(Base64Decode(s, &output));
    EXPECT_EQ((size_t)7, output.size());

    s = "abcdAFCD\r\neF=a";
    EXPECT_TRUE(!Base64Decode(s, &output));

    s = "abcdAFCD\r\ne===";
    EXPECT_TRUE(!Base64Decode(s, &output));

    s = "abcdAFFCD\r\ne==";
    EXPECT_TRUE(Base64Decode(s, &output));
    EXPECT_EQ((size_t)7, output.size());

    s = "abcdAFFCD\r\ne=\r\n=";
    EXPECT_TRUE(Base64Decode(s, &output));
    EXPECT_EQ((size_t)7, output.size());

    s = "abcdAF=D\r\nef==";
    EXPECT_TRUE(!Base64Decode(s, &output));

    s = "abcdA";
    EXPECT_TRUE(!Base64Decode(s, &output));
    Base64Encode("admin:123", &output);
    EXPECT_EQ(string("YWRtaW46MTIz"), output);
    Base64Decode(output, &s);
    EXPECT_EQ(string("admin:123"), s);
}

TEST(Md5Test, BasicTest) {
    unsigned char input[] = "1234567890";
    unsigned char output[100] = {0};
    MD5Calc(input, 10, output);
    std::string cryptOut = "e807f1fcf82d132f9bb018ca6738a19f";
    std::string calcOut = wyf::CStrUitls::HexFormatStr((char *)output, 16);

    EXPECT_EQ(cryptOut, calcOut);
}

TEST(Sha1Test, BasicTest) {
    unsigned char input[] = "1234567687890";
    unsigned char output[100] = {0};
    SHA1Calc(input, 13, output);
    std::string cryptOut = "21885c92cf31020ba246bfaa999e7c8e508c8d53";
    std::string calcOut = wyf::CStrUitls::HexFormatStr((char *)output, 20);

    EXPECT_EQ(cryptOut, calcOut);
}

TEST(Sha256Test, BasicTest1)
{
   
    unsigned char input[] = "abc";
    unsigned char output[33] = {0};
    SHA256_Simple(input, 3, output);
    std::string cryptOut = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
    std::string calcOut = wyf::CStrUitls::HexFormatStr((char *)output, 32);
    EXPECT_EQ(cryptOut, calcOut);
}
TEST(Sha256Test, BasicTest2) 
{
    unsigned char input[] = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    unsigned char output[65] = {0};
    SHA256_Simple(input, 56, output);
    std::string cryptOut = "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1";
    std::string calcOut = wyf::CStrUitls::HexFormatStr((char *)output, 32);
    EXPECT_EQ(cryptOut, calcOut);
    
}

TEST(Sha512Test, BasicTest1)
{
    unsigned char input[] = "abc";
    unsigned char output[65] = {0};
    SHA512_Simple(input, 3, output);
    std::string cryptOut = "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f";
    std::string calcOut = wyf::CStrUitls::HexFormatStr((char *)output, 64);
    EXPECT_EQ(cryptOut, calcOut);
}

TEST(Sha512Test, BasicTest2) 
{
     unsigned char input[] = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmn"
                             "hijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu";
     unsigned char output[65] = {0};
     SHA512_Simple(input, 112, output);
     std::string cryptOut = "8e959b75dae313da8cf4f72814fc143f8f7779c6eb9f7fa17299aeadb6889018501d289e4900f7e4331b99dec4b5433ac7d329eeb6dd26545e96e55b874be909";
     std::string calcOut = wyf::CStrUitls::HexFormatStr((char *)output, 64);
     EXPECT_EQ(cryptOut, calcOut);
}
TEST(Sha512Test, BasicTest3) 
{
    unsigned char input[] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    unsigned char output[65] = {0};
    SHA512_State s;
    int n;
    SHA512_Init(&s);
    for (n = 0; n < 1000000 / 40; n++)
        SHA512_Bytes(&s, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 40);
    SHA512_Final(&s, output);
    std::string cryptOut = "e718483d0ce769644e2e42c7bc15b4638e1f98b13b2044285632a803afa973ebde0ff244877ea60a4cb0432ce577c31beb009c5c2c49aa2e4eadb217ad8cc09b";
    std::string calcOut = wyf::CStrUitls::HexFormatStr((char *)output, 64);
    EXPECT_EQ(cryptOut, calcOut);
}

TEST(TdesTest, BasicTest) {
    char key[16]={0x70,0xE9,0xBE,0xA6,0x97,0x72,0x3D,0xF8,
        0x36,0x05,0xEB,0xBC,0xB7,0xC2,0xC7,0xC4};
    char encpt[100];
    char result[100];
    char plain[] = "1234567890";
    int len = strlen(plain);
    cipher2(key, plain, encpt, len);
    decipher2(key, result, encpt, strlen(encpt));
    EXPECT_EQ(std::string(plain), std::string(result));

    cipher3(key, plain, encpt,len);
    decipher3(key, result, encpt, strlen(encpt));
    EXPECT_EQ(std::string(plain), std::string(result));
}