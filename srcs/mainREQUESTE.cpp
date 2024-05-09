#include "../includes/librarie.hpp"
#include "../includes/RequestParser.hpp"

void printHttpRequest(const HTTrequestMSG& msg) {
    // Impressão dos headers
    std::cout << YELLOW << "Headers:\n" << RESET;
    for (std::map<std::string, std::string>::const_iterator it = msg.headers.begin(); it != msg.headers.end(); ++it) {
        std::cout << "  " << it->first << YELLOW << "-----------> " << RESET << it->second << "\n";
    }
    // Impressão das variáveis de ambiente CGI
    if (msg.is_cgi) {
        std::cout << BLUE << "\n\nCGI Environment Variables:\n\n" << RESET;
        for (std::map<std::string, std::string>::const_iterator it = msg.cgi_env.begin(); it != msg.cgi_env.end(); ++it) {
            std::cout << "  " << it->first << ": " << it->second << "\n";
        }
    }
    std::cout << "\n";
}

int main() 
{
    HTTPParser parser;
    HTTrequestMSG msg;

    // Exemplo de uma requisição GET com CGI
    std::string getRequestCGI =
        "GET /cgi-bin/test HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:125.0) Gecko/20100101 Firefox/125.0\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\n"
        "Accept-Language: en-US,en;q=0.5\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 0\r\n"
        "Connection: keep-alive\r\n"
        "Referer: http://localhost:8080/index.html\r\n"
        "Upgrade-Insecure-Requests: 1\r\n"
        "Sec-Fetch-Dest: document\r\n"
        "Sec-Fetch-Mode: navigate\r\n"
        "Sec-Fetch-Site: same-origin\r\n"
        "Sec-Fetch-User: ?1\r\n"
        "\r\n";

    // Exemplo de uma requisição GET sem CGI
    std::string getRequest =
        "GET /index.html HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:125.0) Gecko/20100101 Firefox/125.0\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\n"
        "Accept-Language: en-US,en;q=0.5\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 0\r\n"
        "Connection: keep-alive\r\n"
        "Referer: http://localhost:8080/index.html\r\n"
        "Upgrade-Insecure-Requests: 1\r\n"
        "Sec-Fetch-Dest: document\r\n"
        "Sec-Fetch-Mode: navigate\r\n"
        "Sec-Fetch-Site: same-origin\r\n"
        "Sec-Fetch-User: ?1\r\n"
        "\r\n";

    // Exemplo de uma requisição DELETE sem CGI
    std::string deleteRequest =
        "DELETE /resource/12345 HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:125.0) Gecko/20100101 Firefox/125.0\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\n"
        "Accept-Language: en-US,en;q=0.5\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Content-Length: 0\r\n"
        "Connection: keep-alive\r\n"
        "Referer: http://localhost:8080/index.html\r\n"
        "Upgrade-Insecure-Requests: 1\r\n"
        "Sec-Fetch-Dest: document\r\n"
        "Sec-Fetch-Mode: navigate\r\n"
        "Sec-Fetch-Site: same-origin\r\n"
        "Sec-Fetch-User: ?1\r\n"
        "\r\n";

         std::string postRequest =
        "POST /submit-form HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:125.0) Gecko/20100101 Firefox/125.0\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\n"
        "Accept-Language: en-US,en;q=0.5\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Content-Type: multipart/form-data; boundary=---------------------------1234567890\r\n"
        "Content-Length: 329\r\n"
        "Connection: keep-alive\r\n"
        "Referer: http://localhost:8080/index.html\r\n"
        "Upgrade-Insecure-Requests: 1\r\n"
        "Sec-Fetch-Dest: document\r\n"
        "Sec-Fetch-Mode: navigate\r\n"
        "Sec-Fetch-Site: same-origin\r\n"
        "Sec-Fetch-User: ?1\r\n"
        "\r\n"
        "-----------------------------1234567890\r\n"
        "Content-Disposition: form-data; name=\"text\"\r\n"
        "\r\n"
        "This is some text content.\r\n"
        "-----------------------------1234567890\r\n"
        "Content-Disposition: form-data; name=\"file\"; filename=\"example.txt\"\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "This is the content of the file.\r\n"
        "-----------------------------1234567890--\r\n";


         std::string postRequestCGI= 
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

     std::cout << GREEN << "Testa requisição CGI GET:\n" << RESET;
    if (parser.parseRequest(getRequestCGI, msg, 10000)) {
        printHttpRequest(msg);
    } else {
        std::cout << RED << "Erro parser da requisição CGI GET.\n" << RESET;
    }

    msg = HTTrequestMSG();

    std::cout << GREEN << "Testa requisição CGI post:\n" << RESET;
    if (parser.parseRequest(postRequestCGI, msg, 10000)) {
        printHttpRequest(msg);
    } else {
        std::cout << RED << "Erro parser da requisição CGI POST.\n" << RESET;
    }

    return 0;
}