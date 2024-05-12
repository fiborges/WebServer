#!/bin/bash

# Print an empty line
echo

# Print "SINGLE SERVER TEST" in yellow
/bin/echo -e "\033[1;33m ==> SINGLE SERVER TEST\n\033[0m"

# Create single_server_test.cpp
echo '
#include <gtest/gtest.h>
#include "../includes/get.hpp"

TEST(ServerInfoTestSingle, SetupServerTest) {
    ServerInfo server;
    conf_File_Info config;
    config.portListen = 8080;
    config.RootDirectory = "resources/TESTE_1";
    server.setRootUrl("resources");
    setupServer(server, config);
    EXPECT_GE(server.getSocketFD(), 0);
    EXPECT_EQ(server.getRootUrl(), "resources");
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
' > single_server_test.cpp

# Compile and run single_server_test.cpp
g++ single_server_test.cpp get.cpp -lgtest -lgtest_main -pthread -lstdc++ -o single_test
./single_test

/bin/echo -e "\033[1;33m\n\n ==> MULTISERVER TEST\n\033[0m"

# Create multi_server_test.cpp
echo '
#include <gtest/gtest.h>
#include "../includes/get.hpp"

class ServerInfoTestMulti : public ::testing::TestWithParam<int> {
};

TEST_P(ServerInfoTestMulti, SetupServerTest) {
    ServerInfo server;
    conf_File_Info config;
    config.portListen = GetParam();
    config.RootDirectory = "resources/TESTE_2";
    server.setRootUrl("resources");
    setupServer(server, config);
    EXPECT_GE(server.getSocketFD(), 0);
    EXPECT_EQ(server.getRootUrl(), "resources");
}

INSTANTIATE_TEST_SUITE_P(PortTests, ServerInfoTestMulti, ::testing::Values(8080, 8081, 8082, 8083));

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
' > multi_server_test.cpp

# Compile and run multi_server_test.cpp
g++ multi_server_test.cpp get.cpp -lgtest -lgtest_main -pthread -lstdc++ -o multi_test
./multi_test

# Remove test files
rm single_server_test.cpp multi_server_test.cpp single_test multi_test

# Print an empty line
echo

/bin/echo -e "\033[1;31m ==> Preparing to delete the directory created: /resources \n\033[0m"

# Pause for 5 seconds
sleep 5

# Remove directories
rm -r resources/TESTE_1 resources/TESTE_2 resources

/bin/echo -e "\033[1;32m ==> Done! \n\033[0m"