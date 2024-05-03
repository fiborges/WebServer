#include <gtest/gtest.h>
#include "../includes/URIparser.hpp"

//g++ -std=c++14 -o http_parser_test URIParser.cpp test_URIparser.cpp -lgtest -lgtest_main -pthread
//./http_parser_test

// Teste para a função parseRequest
TEST(HTTPParserTest, ParseRequest_ValidRequest) {
    HTTPParser parser;
    std::string rawRequest = "GET /index.html HTTP/1.1\r\n"
                             "Host: example.com\r\n"
                             "User-Agent: Mozilla/5.0\r\n"
                             "Accept: text/html\r\n"
                             "\r\n";
    HTTrequestMSG msg;
    size_t maxSize = 1024;

    ASSERT_TRUE(parser.parseRequest(rawRequest, msg, maxSize));
    ASSERT_EQ(msg.state, HTTrequestMSG::FINISH);
    ASSERT_EQ(msg.method, HTTrequestMSG::GET);
    ASSERT_EQ(msg.headers.size(), 3);
    ASSERT_EQ(msg.headers["Host"], "example.com");
    ASSERT_EQ(msg.headers["User-Agent"], "Mozilla/5.0");
    ASSERT_EQ(msg.headers["Accept"], "text/html");
    ASSERT_EQ(msg.body, "");
    ASSERT_EQ(msg.process_bytes, 0);
    ASSERT_EQ(msg.content_lenght, 0);
    ASSERT_EQ(msg.error, "");
}

// Teste para a função parseHeader
TEST(HTTPParserTest, ParseHeader_ValidHeader) {
    HTTPParser parser;
    std::string rawHeader = "GET /index.html HTTP/1.1\r\n"
                            "Host: example.com\r\n"
                            "User-Agent: Mozilla/5.0\r\n"
                            "Accept: text/html\r\n"
                            "\r\n";
    HTTrequestMSG msg;

    ASSERT_TRUE(parser.parseHeader(rawHeader, msg));
    ASSERT_EQ(msg.method, HTTrequestMSG::GET);
    ASSERT_EQ(msg.path, "/index.html");
    ASSERT_EQ(msg.version, "HTTP/1.1");
    ASSERT_EQ(msg.headers.size(), 3);
    ASSERT_EQ(msg.headers["Host"], "example.com");
    ASSERT_EQ(msg.headers["User-Agent"], "Mozilla/5.0");
    ASSERT_EQ(msg.headers["Accept"], "text/html");
}

// Teste para a função readRequestLine
TEST(HTTPParserTest, ReadRequestLine_ValidLine) {
    HTTPParser parser;
    std::istringstream stream("GET /index.html HTTP/1.1");
    HTTrequestMSG msg;

    parser.readRequestLine(stream, msg);

    ASSERT_EQ(msg.method, HTTrequestMSG::GET);
    ASSERT_EQ(msg.path, "/index.html");
    ASSERT_EQ(msg.version, "HTTP/1.1");
}

// Teste para a função readHeaders
TEST(HTTPParserTest, ReadHeaders_ValidHeaders) {
    HTTPParser parser;
    std::istringstream stream("Host: example.com\r\n"
                              "User-Agent: Mozilla/5.0\r\n"
                              "Accept: text/html\r\n"
                              "\r\n");
    HTTrequestMSG msg;

    parser.readHeaders(stream, msg);

    ASSERT_EQ(msg.headers.size(), 3);
    ASSERT_EQ(msg.headers["Host"], "example.com");
    ASSERT_EQ(msg.headers["User-Agent"], "Mozilla/5.0");
    ASSERT_EQ(msg.headers["Accept"], "text/html");
}

// Teste para a função setMethod
TEST(HTTPParserTest, SetMethod_ValidMethod) {
    HTTPParser parser;
    HTTrequestMSG msg;

    parser.setMethod("GET", msg);
    ASSERT_EQ(msg.method, HTTrequestMSG::GET);

    parser.setMethod("POST", msg);
    ASSERT_EQ(msg.method, HTTrequestMSG::POST);

    parser.setMethod("DELETE", msg);
    ASSERT_EQ(msg.method, HTTrequestMSG::DELETE);

    parser.setMethod("PUT", msg); // Método desconhecido
    ASSERT_EQ(msg.method, HTTrequestMSG::UNKNOWN);
}

// Teste para a função parseHex
TEST(HTTPParserTest, ParseHex_ValidHex) {
    HTTPParser parser;

    ASSERT_EQ(parser.parseHex("10"), 16);
    ASSERT_EQ(parser.parseHex("A"), 10);
    ASSERT_EQ(parser.parseHex("FF"), 255);
}

// Teste para a função isChunkedTransferEncoding
TEST(HTTPParserTest, IsChunkedTransferEncoding_ValidEncoding) {
    HTTPParser parser;
    HTTrequestMSG msg;

    msg.headers["transfer-encoding"] = "chunked";
    ASSERT_TRUE(parser.isChunkedTransferEncoding(msg));

    msg.headers["transfer-encoding"] = "gzip, chunked";
    ASSERT_TRUE(parser.isChunkedTransferEncoding(msg));

    msg.headers["transfer-encoding"] = "gzip";
    ASSERT_FALSE(parser.isChunkedTransferEncoding(msg));
}

// Teste para a função setContentLength
TEST(HTTPParserTest, SetContentLength_ValidLength) {
    HTTPParser parser;
    HTTrequestMSG msg;

    msg.headers["content-length"] = "100";
    parser.setContentLength(msg);
    ASSERT_EQ(msg.content_lenght, 100);

    msg.headers["content-length"] = "invalid"; // Valor inválido
    parser.setContentLength(msg);
    ASSERT_EQ(msg.content_lenght, 0);
}

// Teste para a função removeCarriageReturn
TEST(HTTPParserTest, RemoveCR_ValidRemoval) {
    HTTPParser parser;
    std::string s = "string with CR\r";
    parser.removeCarriageReturn(s);
    ASSERT_EQ(s, "string with CR");
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}



