#include "../includes/URIparser.hpp"

const std::string HTTPParser::HTTP_LINE_BREAK = "\r\n";
const std::string HTTPParser::FINAL_CHUNK = "0\r\n\r\n";
const std::string HTTPParser::DELIMITER = HTTP_LINE_BREAK + HTTP_LINE_BREAK;

//Vamos receber os dados brutos (bytes) da conexão de socket. 
//É necessário primeiro converter esses bytes em uma string antes de processá-los

bool HTTPParser::parseRequest(std::string& raw, HTTrequestMSG& msg, size_t maxSize) {
    while (msg.state != HTTrequestMSG::FINISH) {
        switch (msg.state) {
        case HTTrequestMSG::HEADERS:
            if (!parseHeader(raw, msg)) {
                return false;
            }
            msg.state = (msg.method == HTTrequestMSG::GET || msg.method == HTTrequestMSG::DELETE || msg.method == HTTrequestMSG::UNKNOWN) ? HTTrequestMSG::FINISH : HTTrequestMSG::TRANSFER_CONTROL;
            break;
        case HTTrequestMSG::BODY:
            if (msg.process_bytes + raw.length() > maxSize) {
                msg.error = "Request entity too large";
                msg.state = HTTrequestMSG::FINISH;
                return false;
            }
            msg.body.append(raw);
            raw.clear();
            msg.process_bytes = msg.body.length();
            if (msg.process_bytes >= msg.content_lenght) {
                msg.state = HTTrequestMSG::FINISH;
            }
            break;
        case HTTrequestMSG::TRANSFER_CONTROL:
            if (isChunkedTransferEncoding(msg)) {
                msg.state = HTTrequestMSG::CHUNKED;
            } else {
                setContentLength(msg);
                msg.state = msg.content_lenght > 0 ? HTTrequestMSG::CONTENT_LENGTH : HTTrequestMSG::FINISH;
            }
            break;
        case HTTrequestMSG::CONTENT_LENGTH:
            if (msg.process_bytes + raw.length() > maxSize) {
                msg.error = "Request entity too large";
                msg.state = HTTrequestMSG::FINISH;
                return false;
            }
            msg.body.append(raw);
            raw.clear();
            msg.process_bytes = msg.body.length();
            if (msg.process_bytes >= msg.content_lenght) {
                msg.state = HTTrequestMSG::FINISH;
            }
            break;
        case HTTrequestMSG::CHUNKED:
            // Processa cada chunk até encontrar o chunk final "0"
            while (raw.find(FINAL_CHUNK) == std::string::npos) {
                size_t chunk_size_end = raw.find(HTTP_LINE_BREAK);
                if (chunk_size_end == std::string::npos) {
                    return false;
                }
                int chunk_size = parseHex(raw.substr(0, chunk_size_end));
                if (chunk_size == 0) {
                    msg.state = HTTrequestMSG::FINISH;
                    return true;
                }
                if (static_cast<size_t>(msg.process_bytes) + chunk_size > maxSize) {
                    msg.error = "Request entity too large";
                    msg.state = HTTrequestMSG::FINISH;
                    return false;
                }
                size_t chunk_content_start = chunk_size_end + HTTP_LINE_BREAK.length();
                if (raw.length() < chunk_content_start + chunk_size + HTTP_LINE_BREAK.length()) {
                    return false;
                }
                msg.body.append(raw.substr(chunk_content_start, chunk_size));
                raw.erase(0, chunk_content_start + chunk_size + HTTP_LINE_BREAK.length());
                msg.process_bytes = msg.body.length();
            }
            break;
        case HTTrequestMSG::FINISH:
            return true;
        }
    }
    return true;
}

bool HTTPParser::parseHeader(std::string& raw, HTTrequestMSG& msg) {
    size_t end = raw.find(DELIMITER);
    if (end == std::string::npos) {
        return false;
    }

    std::istringstream stream(raw.substr(0, end));
    readRequestLine(stream, msg);
    readHeaders(stream, msg);

    raw.erase(0, end + DELIMITER.length());
    return true;
}


void HTTPParser::readRequestLine(std::istringstream& stream, HTTrequestMSG& msg) {
    std::string method;
    stream >> method >> msg.path >> msg.version;
    setMethod(method, msg);

    size_t delim = msg.path.find("?");
    if (delim != std::string::npos) {
        msg.query = msg.path.substr(delim + 1);
        msg.path.resize(delim);
    }
}

void HTTPParser::readHeaders(std::istringstream& stream, HTTrequestMSG& msg) {
    std::string line;
    while (std::getline(stream, line) && !line.empty()) {
        removeCarriageReturn(line);
        size_t colonPos = line.find(": ");
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 2);
            msg.headers[key] = value;
        }
    }
}

void HTTPParser::setMethod(const std::string& method, HTTrequestMSG& msg) {
    if (method == "GET") {
        msg.method = HTTrequestMSG::GET;
    } else if (method == "POST") {
        msg.method = HTTrequestMSG::POST;
    } else if (method == "DELETE") {
        msg.method = HTTrequestMSG::DELETE;
    } else {
        msg.method = HTTrequestMSG::UNKNOWN;
    }
}

void HTTPParser::removeCarriageReturn(std::string& s) {
    if (!s.empty() && s.back() == '\r') {
        s.pop_back();
    }
}

int HTTPParser::parseHex(const std::string& strNum) {
    int result;
    std::stringstream ss;
    ss << std::hex << strNum;
    ss >> result;
    return result;
}

bool HTTPParser::isChunkedTransferEncoding(const HTTrequestMSG& msg) {
    std::map<std::string, std::string>::const_iterator it = msg.headers.find("transfer-encoding");
    if (it != msg.headers.end()) {
        return it->second.find("chunked") != std::string::npos;
    }
    return false;
}

void HTTPParser::setContentLength(HTTrequestMSG& msg) {
    if (msg.headers.count("content-length") > 0) {
        msg.content_lenght = std::atoi(msg.headers["content-length"].c_str());
    }
}

std::string HTTPParser::methodToString(HTTrequestMSG::Method method) {
    switch (method) {
        case HTTrequestMSG::GET: return "GET";
        case HTTrequestMSG::POST: return "POST";
        case HTTrequestMSG::DELETE: return "DELETE";
        default: return "UNKNOWN";
    }
}
/*
#include <iostream>
#include <string>
#include "URIparser.hpp" // Supondo que o arquivo de cabeçalho contém a definição de Message e a implementação de HTTPParser

int main() {
    std::string request = "GET /index.html HTTP/1.1\r\n"
                          "Host: www.example.com\r\n"
                          "User-Agent: Mozilla/5.0\r\n"
                          "Content-Length: 0\r\n"
                          "\r\n";

    HTTPParser parser;
    Message msg;

    size_t maxSize = 1024;

    // Processamento da requisição
    bool result = parser.parseRequest(request, msg, maxSize);

    if (result) {
        std::cout << "Request successfully processed!" << std::endl;
        std::cout << "Method: " << msg.method << std::endl;
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


std::string receiveDataFromSocket() {
    // Suponho que temos uma função para receber dados da conexão de socket
    // vou simular que recebo uma requisição GET básica
    std::string requestData = "GET /index.html HTTP/1.1\r\n"
                              "Host: example.com\r\n"
                              "User-Agent: Mozilla/5.0\r\n"
                              "Accept: text/html\r\n"
                              "\r\n";
    return requestData;
}


// Estrutura para armazenar a mensagem da requisição HTTP
struct HttpRequest {
    std::string method;
    std::string path;
    std::map<std::string, std::string> headers;
};

// Função para processar a string da requisição e extrair informações relevantes

HttpRequest processHttpRequest(const std::string& requestData) 
{
    HttpRequest request;

    // Uso stringstream para analisar a string da requisição
    std::istringstream iss(requestData);

    // Leio a linha de requisição para obter o método e o caminho
    iss >> request.method >> request.path;

    // Processo os cabeçalhos HTTP
    std::string line;
    while (std::getline(iss, line) && !line.empty()) 
    {
        // Separo o cabeçalho em chave e valor
        std::size_t pos = line.find(": ");
        if (pos != std::string::npos) 
        {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 2);
            request.headers[key] = value;
        }
    }

    return request;
}

int main() {
    // Recebo os dados da conexão de socket
    std::string requestData = "GET /index.html HTTP/1.1\r\n"
                              "Host: example.com\r\n"
                              "User-Agent: Mozilla/5.0\r\n"
                              "Accept: text/html\r\n"
                              "\r\n";

    // Processo a requisição HTTP
    HttpRequest request = processHttpRequest(requestData);

    // Exibo as informações extraídas
    std::cout << "Método: " << request.method << std::endl;
    std::cout << "Caminho do recurso: " << request.path << std::endl;
    std::cout << "Cabeçalhos HTTP:" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = request.headers.begin(); it != request.headers.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }

    return 0;
}

*/
