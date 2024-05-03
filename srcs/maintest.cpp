#include <iostream>
#include <string>
#include "../includes/URIparser.hpp"

/*
GET é representado pelo valor 0.
POST é representado pelo valor 1.
DELETE é representado pelo valor 2.
*/
int main() 
{
    std::string request = "GET /index.html HTTP/1.1\r\n"
                          "Host: www.example.com\r\n"
                          "User-Agent: Mozilla/5.0\r\n"
                          "Content-Length: 10\r\n"
                          "\r\n";

    HTTPParser parser;
    HTTrequestMSG msg;
    size_t maxSize = 1024;

    bool result = parser.parseRequest(request, msg, maxSize);

    if (result) {
        std::cout << "Request successfully processed!" << std::endl;
        std::cout << "Method Number: " << msg.method << std::endl;
        std::cout << "Method Name: " << parser.methodToString(msg.method) << std::endl;
        std::cout << "Path: " << msg.path << std::endl;
        std::cout << "Version: " << msg.version << std::endl;
        std::cout << "Host: " << msg.headers["Host"] << std::endl;
        std::cout << "User-Agent: " << msg.headers["User-Agent"] << std::endl;
        std::cout << "Content-Length: " << msg.headers["Content-Length"] << std::endl;
    } else {
        std::cerr << "Failed to process request!" << std::endl;
    }

    return 0;
}