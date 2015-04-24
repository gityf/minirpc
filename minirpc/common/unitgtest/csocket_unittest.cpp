
#include "common/csingleton.h"
#include "common/csocket.h"
#include "common/colorprint.h"
#include <iostream>
#include "gtest/gtest.h"
using namespace wyf;
using namespace std;
TEST(CSocketTest, BasicTest)
{
    CSocket* sock = new CSocket();
    EXPECT_EQ(true, sock->IsIPv4Addr("10.1.1.1"));
    EXPECT_EQ(true, sock->IsIPv4Addr("0.0.0.0"));
    EXPECT_EQ(true, sock->IsIPv4Addr("255.255.255.255"));
    EXPECT_EQ(true, sock->IsIPv4Addr("127.0.0.1"));
    EXPECT_EQ(true, sock->IsIPv4Addr("192.168.1.1"));
    EXPECT_EQ(true, sock->IsIPv4Addr("224.0.1.1"));
    EXPECT_EQ(true, sock->IsIPv6Addr("0:0:0:0:0:FFFF:204.152.189.116"));
    EXPECT_EQ(true, sock->IsIPv6Addr("0:0:0:0:0:0:0:0"));
    EXPECT_EQ(true, sock->IsIPv6Addr("2606:b400:c00:58:5466:154d:c9d8:6d9b"));
    EXPECT_EQ(true, sock->IsIPv6Addr("FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF"));
    EXPECT_EQ(true, sock->IsIPv6Addr("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"));
    wyf::COLOR_GREEN(cout) << "[ INFO     ] stdout/stderr->"; 
    cout.flush();
    string outStr = "This NetWrite test case display string!!!\n";
    sock->NetWrite(1, outStr.c_str(), outStr.size());
    sock->NetWrite(2, outStr.c_str(), outStr.size());
    char buf[1025] = {0};
    wyf::COLOR_GREEN(cout) << "[ INFO     ] test TcpRecvOnce input:"; 
    sock->TcpRecvOnce(0, buf, 1024);
    wyf::COLOR_GREEN(cout) << "[ INFO     ] in >>" << buf; 
    sock->HostResolve("localhost", buf, sizeof(buf));
    wyf::COLOR_GREEN(cout) << "[ INFO     ] localhost to ip >>" << buf << endl; 
    sock->HostResolve("127.1.2.3", buf, sizeof(buf));
    wyf::COLOR_GREEN(cout) << "[ INFO     ] 127.1.2.3 to ip >>" << buf << endl; 
    sock->Name2IPv4("localhost", buf);
    wyf::COLOR_GREEN(cout) << "[ INFO     ] localhost to ip >>" << buf << endl; 
    delete sock;
}