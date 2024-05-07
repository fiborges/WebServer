#include "../includes/librarie.hpp"
#include "../includes/RequestParser.hpp"

void printHttpRequest(const HTTrequestMSG& msg) {
    std::cout << "Method: " << HTTPParser::methodToString(msg.method) << "\n";
    std::cout << "Path: " << msg.path << "\n";
    std::cout << "Query: " << msg.query << "\n";
    std::cout << "Version: " << msg.version << "\n";
    std::cout << "Headers:\n";
    for (std::map<std::string, std::string>::const_iterator it = msg.headers.begin(); it != msg.headers.end(); ++it) {
        std::cout << "  " << it->first << ": " << it->second << "\n\n";
    }
    if (!msg.body.empty()) {
        std::cout << "Body: " << msg.body << "\n";
    }
    if (msg.is_cgi) {
        std::cout << YELLOW << "CGI:" << RESET << "Yes\n\n";
        std::cout << BLUE << "CGI Environment Variables:\n" << RESET;
        for (std::map<std::string, std::string>::const_iterator it = msg.cgi_env.begin(); it != msg.cgi_env.end(); ++it) {
            std::cout << "  " << it->first << ": " << it->second << "\n";
        }
    }
    std::cout << "\n";
}

int main() {
    // Simulação de uma requisição HTTP GET
    std::string getRequest = 
        "GET /index.html HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "Connection: keep-alive\r\n"
        "Accept-Language: en-US,en;q=0.9\r\n\r\n";

    HTTPParser parser;
    HTTrequestMSG msg;

    std::cout << GREEN << "Testa requisição GET:\n" << RESET;
    if (parser.parseRequest(getRequest, msg, 10000)) {
        printHttpRequest(msg);
    } else {
        std::cout << RED << "Erro parser da requisição GET.\n" << RESET;
    }

    return 0;
}
