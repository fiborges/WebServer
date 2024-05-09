#include <gtest/gtest.h>
#include <fstream>
#include "../includes/get.hpp"

//  g++ get_test.cpp get.cpp -lgtest -lgtest_main -pthread -lstdc++

// Test case for handling a valid GET request with existing file
TEST(ServerInfoTest, HandleGetRequest_ValidFile) {
    ServerInfo server;
    std::string path = "/index.html";

    // Read the actual content of the file
    std::ifstream file("../resources/website" + path);
    std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Use the actual content of the file in the expected response
    std::string expectedResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + fileContent;

    server.handleGetRequest(path, server);

    ASSERT_EQ(server.getResponse(), expectedResponse);
}

// Test case for handling a valid GET request with non-existing file
TEST(ServerInfoTest, HandleGetRequest_NonExistingFile) {
    ServerInfo server;
    std::string path = "/non_existing.html";
    std::string expectedResponse = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n";

    server.handleGetRequest(path, server);

    ASSERT_EQ(server.getResponse(), expectedResponse);
}

// Test case for handling a GET request with a directory path
TEST(ServerInfoTest, HandleGetRequest_DirectoryPath) {
    ServerInfo server;
    std::string path = "/directory/";
    std::string expectedResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nDirectory content";

    server.handleGetRequest(path, server);

    ASSERT_EQ(server.getResponse(), expectedResponse);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}