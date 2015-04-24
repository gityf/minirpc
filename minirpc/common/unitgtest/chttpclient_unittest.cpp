
#include "common/csingleton.h"
#include "common/chttpclient.h"
#include "common/chttputils.h"
#include "common/colorprint.h"
#include <iostream>
#include "gtest/gtest.h"
using namespace wyf;
TEST(CHttpClientTest, BasicTest)
{
    CHttpClient* httpCln = new CHttpClient();
    bool ret = httpCln->IsUrlExist("http://127.0.0.1:15218");
    if (ret) {
        ret = httpCln->Request("http://127.0.0.1:15218");
        wyf::COLOR_GREEN(cout) << "[ INFO     ] errorcode:" << httpCln->ErrCode() << endl;
        EXPECT_EQ(true, ret);
        wyf::COLOR_GREEN(cout) << "[ INFO     ] Header:\n" << httpCln->iHttpUtils->DumpHeader() << endl;
        wyf::COLOR_GREEN(cout) << "[ INFO     ] Body:\n" << httpCln->iHttpUtils->Body() << endl;
    }
}

TEST(CHttpUtilsTest, BasicTest)
{
    string httpRespStr = "HTTP/1.1 200 OK\r\n"
                         "Date: Thu, 26 Feb 2015 01:53:49 GMT\r\n"
                         "Server: nginx/1.0.12\r\n"
                         "Content-Type: text/html\r\n"
                         "Accept-Ranges: bytes\r\n"
                         "Last-Modified: Tue, 18 Jun 2013 07:44:44 GMT\r\n"
                         "Content-Length: 10\r\n"
                         "Proxy-Connection: Keep-Alive\r\n\r\n1234567890";
    CHttpUtils* httpUtils = new CHttpUtils();
    httpUtils->ParseResponse(httpRespStr);

    EXPECT_EQ("1234567890", httpUtils->Body());
    EXPECT_EQ("200", httpUtils->Status());
    EXPECT_EQ("Thu, 26 Feb 2015 01:53:49 GMT", httpUtils->GetHeader("Date"));
    EXPECT_EQ("nginx/1.0.12", httpUtils->GetHeader("Server"));
    EXPECT_EQ("text/html", httpUtils->GetHeader("Content-Type"));
    EXPECT_EQ("bytes", httpUtils->GetHeader("Accept-Ranges"));
    EXPECT_EQ("Tue, 18 Jun 2013 07:44:44 GMT", httpUtils->GetHeader("Last-Modified"));
    EXPECT_EQ("10", httpUtils->GetHeader("Content-Length"));
    EXPECT_EQ("10", httpUtils->GetHeader("Content-Length"));
    EXPECT_EQ("Keep-Alive", httpUtils->GetHeader("Proxy-Connection"));

    SUrlInfo url;
    httpUtils->ParseUrl("http://127.0.0.1:15218/name?k=vv&k2=v2", url);
    EXPECT_EQ(url.iHost, "127.0.0.1");
    EXPECT_EQ(url.iPort, 15218);
    EXPECT_EQ(url.iUrl, "/name");
    EXPECT_EQ(url.iUrlParams, "k=vv&k2=v2");
    delete httpUtils;
}