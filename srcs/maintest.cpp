#include "../includes/librarie.hpp"
#include "../includes/RequestParser.hpp"
#include "../includes/get.hpp"

void printHttpRequest(const HTTrequestMSG& msg) {
    std::cout << "Method: " << HTTPParser::methodToString(msg.method) << "\n";
    std::cout << "Path: " << msg.path << "\n";
    std::cout << "Query: " << msg.query << "\n";
    std::cout << "Version: " << msg.version << "\n";
    std::cout << "Headers:\n";
    for (std::map<std::string, std::string>::const_iterator it = msg.headers.begin(); it != msg.headers.end(); ++it) {
        std::cout << "  " << it->first << ": " << it->second << "\n";
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

    // Simulação de uma requisição HTTP POST
    std::string postRequest = 
        "POST /submit-form HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Content-Length: 27\r\n"
        "\r\n"
        "name=John+Doe&age=23";

    // Simulação de uma requisição HTTP DELETE
    std::string deleteRequest = 
        "DELETE /resource/12345 HTTP/1.1\r\n"
        "Host: www.example.com\r\n\r\n";

    std::string cgiRequest = 
    "POST /bin-cgi HTTP/1.1\r\n"
    "Host: localhost:8080\r\n"
    "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:125.0) Gecko/20100101 Firefox/125.0\r\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\n"
    "Accept-Language: en-US,en;q=0.5\r\n"
    "Accept-Encoding: gzip, deflate, br\r\n"
    "Content-Type: multipart/form-data; boundary=---------------------------269449056911015867562387764176\r\n"
    "Content-Length: 1276\r\n"
    "Origin: http://localhost:8080\r\n"
    "Connection: keep-alive\r\n"
    "Referer: http://localhost:8080/index.html\r\n"
    "Upgrade-Insecure-Requests: 1\r\n"
    "Sec-Fetch-Dest: document\r\n"
    "Sec-Fetch-Mode: navigate\r\n"
    "Sec-Fetch-Site: same-origin\r\n"
    "Sec-Fetch-User: ?1\r\n"
    "\r\n"
    "-----------------------------269449056911015867562387764176\r\n"
    "Content-Disposition: form-data; name=\"file\"; filename=\"main.cpp\"\r\n"
    "Content-Type: text/x-c++src\r\n"
    "\r\n"
    "Conteúdo do arquivo aqui\r\n"
    "-----------------------------269449056911015867562387764176--\r\n";


    HTTPParser parser;
    HTTrequestMSG msg;

    std::cout << GREEN << "Testa requisição GET:\n" << RESET;
    if (parser.parseRequest(getRequest, msg, 10000)) {
        printHttpRequest(msg);
    } else {
        std::cout << RED << "Erro parser da requisição GET.\n" << RESET;
    }

    // Limpa msg para o próximo teste
    msg = HTTrequestMSG();

    std::cout << GREEN << "Testa requisição POST:\n" << RESET;
    if (parser.parseRequest(postRequest, msg, 10000)) {
        printHttpRequest(msg);
    } else {
        std::cout << RED << "Erro parser da requisição POST.\n" << RESET;
    }

    // Limpa msg para o próximo teste
    msg = HTTrequestMSG();

    std::cout << GREEN << "Testa requisição DELETE:\n" << RESET;
    if (parser.parseRequest(deleteRequest, msg, 10000)) {
        printHttpRequest(msg);
    } else {
        std::cout << RED << "Erro parser da requisição DELETE.\n" << RESET;
    }

    // Limpa msg para o próximo teste
    msg = HTTrequestMSG();

    // Testar a requisição CGI
   std::cout << GREEN << "Testa requisição CGI:\n" << RESET;
    if (parser.parseRequest(cgiRequest, msg, 10000)) {
        printHttpRequest(msg);
        if (msg.is_cgi) {
            std::cout << BLUE << "CGI Environment Variables:\n" << RESET;
            for (std::map<std::string, std::string>::size_type i = 0; i < msg.cgi_env.size(); ++i) {
                std::map<std::string, std::string>::const_iterator it = msg.cgi_env.begin();
                std::advance(it, i);
                std::cout << "  " << it->first << ": " << it->second << "\n";
            }
        }
    } else {
        std::cout << RED << "Erro parser da requisição CGI.\n" << RESET;
    }

    return 0;
}
