#include "../includes/librarie.hpp"
#include "../includes/URIparser.hpp"


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

    // Simulação de uma requisição CGI
    std::string cgiRequest = 
        "POST /cgi-bin/script.cgi?name=value HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "data=hello+world";

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
    } else {
        std::cout << RED << "Erro parser da requisição CGI.\n" << RESET;
    }

    return 0;
}




/*int main() {
    // Simulação de uma requisição HTTP normal
    std::string httpRequest = 
        "GET /index.html HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "Connection: keep-alive\r\n"
        "Accept-Language: en-US,en;q=0.9\r\n"
        "Content-Length: 0\r\n\r\n";

    // Simulação de uma requisição CGI
    std::string cgiRequest = 
        "POST /cgi-bin/script.cgi?name=value HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "data=hello+world";

    HTTPParser parser;
    HTTrequestMSG msg;

    std::cout << GREEN << "Teste da requisição HTTP normal:\n" << RESET;
    if (parser.parseRequest(httpRequest, msg, 10000)) {
        std::cout << "Method: " << parser.methodToString(msg.method) << std::endl;
        std::cout << "Path: " << msg.path << std::endl;
        std::cout << "Query: " << msg.query << std::endl;
        std::cout << "Version: " << msg.version << std::endl;
        std::cout << "Headers:\n";
        for (std::map<std::string, std::string>::iterator it = msg.headers.begin(); it != msg.headers.end(); ++it) {
            std::cout << "  " << it->first << ": " << it->second << std::endl;
        }
        std::cout << "Body: " << msg.body << "\n\n";
    } else {
        std::cout << "Erro ao parsear a requisição HTTP normal.\n";
    }

    // LIMPA msg para o próximo teste
    msg = HTTrequestMSG();

    std::cout << GREEN << "Teste da requisição CGI:\n" << RESET;
    if (parser.parseRequest(cgiRequest, msg, 10000)) {
        std::cout << "Method: " << parser.methodToString(msg.method) << std::endl;
        std::cout << "Path: " << msg.path << std::endl;
        std::cout << "Query: " << msg.query << std::endl;
        std::cout << "Version: " << msg.version << std::endl;
        std::cout << "CGI: " << (msg.is_cgi ? "Yes" : "No") << std::endl;
        std::cout << "CGI Environment Variables:\n";
        for (std::map<std::string, std::string>::iterator it = msg.cgi_env.begin(); it != msg.cgi_env.end(); ++it) {
            std::cout << "  " << it->first << ": " << it->second << std::endl;
        }
        std::cout << "Headers:\n";
        for (std::map<std::string, std::string>::iterator it = msg.headers.begin(); it != msg.headers.end(); ++it) {
            std::cout << "  " << it->first << ": " << it->second << std::endl;
        }
        std::cout << "Body: " << msg.body << "\n\n";
    } else {
        std::cout << "Erro ao parsear a requisição CGI.\n";
    }

    return 0;
}*/