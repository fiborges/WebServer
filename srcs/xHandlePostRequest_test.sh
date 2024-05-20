
#!/bin/bash

echo

/bin/echo -e "\033[1;33m ==> HANDLE POST RESQUEST TEST\n\033[0m"

# Criar o diretório para o teste
mkdir -p resources/website/path

# Criar o arquivo xHandlePostRequest_test.cpp e inserir o código de teste
cat <<- 'EOF' > xHandlePostRequest_test.cpp
#include <gtest/gtest.h>
#include "../includes/get.hpp"

TEST(ServerInfoTest, HandlePostRequestTest) {
     // Create a ServerInfo object
     ServerInfo server;

     // Create a mock HTTrequestMSG object
     HTTrequestMSG request;
     request.method = HTTrequestMSG::POST;
     request.body = "key1=value1&key2=value2&key3=value3";

     // Call the handlePostRequest function
     server.handlePostRequest("/path", request, server);

     // Check the response
     std::string expectedResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
     expectedResponse += "Received POST data:\nkey1=value1\nkey2=value2\nkey3=value3";

     ASSERT_EQ(server.getResponse(), expectedResponse);
}

TEST(ServerInfoTest, HandlePostRequestTest_NoBody) {
     // Create a ServerInfo object
     ServerInfo server;

     // Create a mock HTTrequestMSG object with no body
     HTTrequestMSG request;
     request.method = HTTrequestMSG::POST;

     // Call the handlePostRequest function
     server.handlePostRequest("/path", request, server);

     // Check the response
     // Replace with the expected response when there is no body
     std::string expectedResponse = "Your expected response here";
     ASSERT_EQ(server.getResponse(), expectedResponse);
}

TEST(ServerInfoTest, HandlePostRequestTest_BadFormat) {
     // Create a ServerInfo object
     ServerInfo server;

     // Create a mock HTTrequestMSG object with a body that is not in the correct format
     HTTrequestMSG request;
     request.method = HTTrequestMSG::POST;
     request.body = "This is not in the correct format";

     // Call the handlePostRequest function
     server.handlePostRequest("/path", request, server);

     // Check the response
     // Replace with the expected response when the body is not in the correct format
     std::string expectedResponse = "Your expected response here";
     ASSERT_EQ(server.getResponse(), expectedResponse);
}

TEST(ServerInfoTest, HandlePostRequestTest_NonexistentPath) {
     // Create a ServerInfo object
     ServerInfo server;

     // Create a mock HTTrequestMSG object
     HTTrequestMSG request;
     request.method = HTTrequestMSG::POST;
     request.body = "key1=value1&key2=value2&key3=value3";

     // Call the handlePostRequest function with a non-existent path
     server.handlePostRequest("/nonexistentpath", request, server);

     // Check the response
     std::string expectedResponse = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n";
     expectedResponse += "The requested resource could not be found.";
     ASSERT_EQ(server.getResponse(), expectedResponse);
}

EOF

# Compilar e executar o teste
g++ -o a.out xHandlePostRequest_test.cpp get.cpp -lgtest -lgtest_main -pthread
./a.out

# Remover o arquivo copiado
rm xHandlePostRequest_test.cpp
rm a.out
# Remover o diretório após o teste
rm -r resources/website/path

echo