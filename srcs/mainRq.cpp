#include "../includes/librarie.hpp"
#include "../includes/RequestParser.hpp"
#include "../includes/erros.hpp"

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

    // Impressão das outras variáveis
    std::cout << GREEN << "\n\nOther Variables:\n\n" << RESET;
    std::cout << "Method: " << (msg.method == HTTrequestMSG::GET ? "GET" : 
                                  msg.method == HTTrequestMSG::POST ? "POST" : 
                                  msg.method == HTTrequestMSG::DELETE ? "DELETE" : "UNKNOWN") << "\n";
    std::cout << "State: " << msg.state << "\n";
    std::cout << "Path: " << msg.path << "\n";
    std::cout << "Version: " << msg.version << "\n";
    std::cout << "Query: " << msg.query << "\n";
    std::cout << "Body: " << msg.body << "\n";
    std::cout << "Content Length: " << msg.content_length << "\n";
    std::cout << "Processed Bytes: " << msg.process_bytes << "\n";
    std::cout << "Error: " << msg.error << "\n";
    std::cout << "Boundary: " << msg.boundary << "\n";
    std::cout << "Temp File Path: " << msg.temp_file_path << "\n";
    std::cout << "Hostname: " << msg.hostname << "\n";
    std::cout << "\n";
}

// Função para verificar se um arquivo existe
bool fileExists2(const std::string& fileName) {
    struct stat buffer;
    return (stat(fileName.c_str(), &buffer) == 0);
}

void testErrors(ServerErrorHandler& errorHandler) {
    std::cout << GREEN << "\n\nTesta páginas de erro:\n" << RESET;
    std::vector<int> errorCodes;
    errorCodes.push_back(400);
    errorCodes.push_back(403);
    errorCodes.push_back(404);
    errorCodes.push_back(500);
    errorCodes.push_back(502);

    for (size_t i = 0; i < errorCodes.size(); ++i) {
        int code = errorCodes[i];
        std::string errorPage = errorHandler.generateErrorPage(code);
        std::cout << "Erro " << code << ":\n" << errorPage << "\n";
    }
}

void saveErrorPages(ServerErrorHandler& errorHandler) {
    std::vector<int> errorCodes;
    errorCodes.push_back(400);
    errorCodes.push_back(403);
    errorCodes.push_back(404);
    errorCodes.push_back(500);
    errorCodes.push_back(502);

    for (size_t i = 0; i < errorCodes.size(); ++i) {
        int code = errorCodes[i];
        std::string fileName = "error_" + utils::to_string(code) + ".html";

        if (!fileExists2(fileName)) {
            std::string errorPage = errorHandler.generateErrorPage(code);
            std::ofstream outFile(fileName.c_str());
            if (outFile.is_open()) {
                outFile << errorPage;
                outFile.close();
                std::cout << "Saved error page " << code << " to " << fileName << "\n";
            } else {
                std::cerr << "Failed to open file " << fileName << " for writing.\n";
            }
        } else {
            std::cout << "File " << fileName << " already exists. Skipping...\n";
        }
    }
}

int main() 
{
    HTTPParser parser;
    HTTrequestMSG msg;
    ServerErrorHandler errorHandler;

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


        std::string postRequestCGI =
        "POST /bin-cgi HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:125.0) Gecko/20100101 Firefox/125.0\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\n"
        "Accept-Language: en-US,en;q=0.5\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Content-Type: multipart/form-data; boundary=---------------------------269449056911015867562387764176\r\n"
        "Content-Length: 248\r\n"
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

    // Verificando o tamanho da requisição
    std::cout << "Tamanho da requisição postRequestCGI: " << postRequestCGI.size() << " bytes\n";


   // Requisições inválidas (com erro)
    std::string badRequestContentLength =
        "POST /submit-form HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:125.0) Gecko/20100101 Firefox/125.0\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\n"
        "Accept-Language: en-US,en;q=0.5\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Content-Type: multipart/form-data; boundary=---------------------------1234567890\r\n"
        "Content-Length: 1000\r\n" // Length muito maior que o conteúdo real
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

    std::string badRequestMissingHost =
    "POST /submit-form HTTP/1.1\r\n"
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
    "\r\n";

    std::string badRequestNoMethod =
        "/index.html HTTP/1.1\r\n" // Falta o método HTTP
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

    std::cout << GREEN << "\nTesta requisição GET:\n" << RESET;
    if (parser.parseRequest(getRequest, msg, 10000)) {
        printHttpRequest(msg);
    } else {
        std::cout << RED << "Erro no parser da requisição GET.\n" << RESET;
    }

    // Limpa msg para o próximo teste
    msg = HTTrequestMSG();

    std::cout << GREEN << "\nTesta requisição POST:\n" << RESET;
    if (parser.parseRequest(postRequest, msg, 10000)) {
        printHttpRequest(msg);
    } else {
        std::cout << RED << "Erro no parser da requisição POST.\n" << RESET;
    }

    // Limpa msg para o próximo teste
    msg = HTTrequestMSG();

    std::cout << GREEN << "\nTesta requisição DELETE:\n" << RESET;
    if (parser.parseRequest(deleteRequest, msg, 10000)) {
        printHttpRequest(msg);
    } else {
        std::cout << RED << "Erro no parser da requisição DELETE.\n" << RESET;
    }

    // Limpa msg para o próximo teste
    msg = HTTrequestMSG();

    std::cout << GREEN << "\nTesta requisição CGI GET:\n" << RESET;
    if (parser.parseRequest(getRequestCGI, msg, 10000)) {
        printHttpRequest(msg);
    } else {
        std::cout << RED << "Erro no parser da requisição CGI GET.\n" << RESET;
    }

    msg = HTTrequestMSG();

    std::cout << GREEN << "\nTesta requisição CGI POST:\n" << RESET;
    if (parser.parseRequest(postRequestCGI, msg, 10000)) {
        printHttpRequest(msg);
    } else {
        std::cout << RED << "Erro no parser da requisição CGI POST.\n" << RESET;
    }

    // Testa requisições inválidas (com erro)
    std::cout << GREEN << "\nTesta requisição com Content-Length incorreto:\n" << RESET;
    if (parser.parseRequest(badRequestContentLength, msg, 10000)) {
        printHttpRequest(msg);
    } else {
        std::cout << RED << "Erro no parser da requisição com Content-Length incorreto.\n" << msg.error << RESET;
    }

    msg = HTTrequestMSG();

    std::cout << GREEN << "\nTesta requisição malformado - missing host:\n" << RESET;
    if (parser.parseRequest(badRequestMissingHost, msg, 10000)) {
        printHttpRequest(msg);
    } else {
        std::cout << RED << "Erro no parser da requisição com header malformado.\n" << msg.error << RESET;
    }

    msg = HTTrequestMSG();

    std::cout << GREEN << "\nTesta requisição sem método HTTP:\n" << RESET;
    if (parser.parseRequest(badRequestNoMethod, msg, 10000)) {
        printHttpRequest(msg);
    } else {
        std::cout << RED << "Erro no parser da requisição sem método HTTP.\n" << msg.error << RESET;
    }

    // Testa a geração de páginas de erro
    testErrors(errorHandler);
    saveErrorPages(errorHandler);

    return 0;
}