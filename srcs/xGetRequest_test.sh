#!/bin/bash

echo

/bin/echo -e "\033[1;33m ==> HANDLE GET RESQUEST TEST\n\033[0m"

# Criar o arquivo xHandleGetRequest_test.cpp e inserir o código de teste
cat <<- 'EOF' > xHandleGetRequest_test.cpp
#include <gtest/gtest.h>
#include "includes/get.hpp"
#include "includes/RequestParser.hpp"

TEST(ServerInfoTest, HandleGetRequestTest_RootDirectory) {
    ServerInfo server;
    std::string path = "/";
    server.handleGetRequest(path, server);
    std::string expectedResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    expectedResponse += readFileContent("resources/website/index.html");
    EXPECT_EQ(server.getResponse(), expectedResponse);
}

TEST(ServerInfoTest, HandleGetRequestTest_IndexFile) {
    ServerInfo server;
    std::string path = "/index.html";
    server.handleGetRequest(path, server);
    std::string expectedResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + readFileContent("resources/website/index.html");
    EXPECT_EQ(server.getResponse(), expectedResponse);
}

TEST(ServerInfoTest, HandleGetRequestTest_NonexistentFile) {
    ServerInfo server;
    std::string path = "/nonexistent.html";
    server.handleGetRequest(path, server);
    std::string expectedResponse = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n";
    EXPECT_EQ(server.getResponse(), expectedResponse);
}
EOF

# Copiar o arquivo
cp xHandleGetRequest_test.cpp ../

# Mudar para o diretório pai
cd ..

# Compilar e executar o teste
g++ xHandleGetRequest_test.cpp srcs/get.cpp srcs/RequestParser.cpp -lgtest -lgtest_main -pthread
./a.out

# Remover o arquivo copiado
rm xHandleGetRequest_test.cpp
cd - > /dev/null
rm xHandleGetRequest_test.cpp
cd ..
rm a.out

echo