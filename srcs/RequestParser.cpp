#include "../includes/RequestParser.hpp"

const std::string HTTPParser::HTTP_LINE_BREAK = "\r\n";
const std::string HTTPParser::FINAL_CHUNK = "0\r\n\r\n";
const std::string HTTPParser::DELIMITER = HTTP_LINE_BREAK + HTTP_LINE_BREAK;

bool HTTPParser::parseRequest(std::string& raw, HTTrequestMSG& msg, size_t maxSize) {
    // Parse headers first
    if (!parseHeader(raw, msg)) {
        return false;
    }

    // Check if it's multipart/form-data
    std::string contentType = msg.headers["Content-Type"];
    if (contentType.find("multipart/form-data") != std::string::npos) {
    std::string boundary = getBoundary(contentType);
    if (boundary.empty()) {
        msg.error = "Bad Request: No boundary in multipart/form-data";
        return false;
    }
    // Treat multipart/form-data requests as CGI if it meets additional criteria
    if (msg.path.find("cgi") != std::string::npos) {
        msg.is_cgi = true;
        printf("entrei aqui\n");
        printf("msg.is_cgi: %d\n", msg.is_cgi);
        setupCGIEnvironment(msg); // Configure CGI environment variables
    }
    // Process multipart data
    return processMultipartData(raw, boundary, msg, maxSize);
}
    printf("\n\nRequest Path: %s\n\n", msg.path.c_str());
    if (msg.path.find("cgi") != std::string::npos) {
        printf("entrei aqui");
        printf("msg.isggi: %d", msg.is_cgi);
        msg.is_cgi = true;
        setupCGIEnvironment(msg); // Configure CGI environment variables
    }

    // Continue with other processing if not multipart/form-data
    while (msg.state != HTTrequestMSG::FINISH) {
        switch (msg.state) {
        case HTTrequestMSG::TRANSFER_CONTROL:
            if (isChunkedTransferEncoding(msg)) {
                msg.state = HTTrequestMSG::CHUNKED;
            } else {
                setContentLength(msg);
                msg.state = msg.content_length > 0 ? HTTrequestMSG::CONTENT_LENGTH : HTTrequestMSG::FINISH;
            }
            break;
        case HTTrequestMSG::BODY:
        case HTTrequestMSG::CONTENT_LENGTH:
            if (msg.process_bytes + raw.length() > maxSize) {
                msg.error = "Request entity too large";
                msg.state = HTTrequestMSG::FINISH;
                return false;
            }
            msg.body.append(raw);
            raw.clear();
            msg.process_bytes = msg.body.length();
            if (msg.process_bytes >= msg.content_length) {
                msg.state = HTTrequestMSG::FINISH;
            }
            break;
        case HTTrequestMSG::CHUNKED:
            return processChunkedBody(raw, msg, maxSize);
        }
    }

    // Set CGI environment variables if it's a CGI request
    if (msg.is_cgi) {
        setupCGIEnvironment(msg);
    }

    return true;
}

void HTTPParser::setupCGIEnvironment(const HTTrequestMSG& msg) {
    // Cria uma cópia do mapa de cabeçalhos
    std::map<std::string, std::string> headers_copy = msg.headers;

    // Cria uma cópia do mapa de variáveis de ambiente
    std::map<std::string, std::string> cgi_env_copy = msg.cgi_env;

    // Define as variáveis de ambiente relevantes
    cgi_env_copy["REQUEST_METHOD"] = methodToString(msg.method);
    cgi_env_copy["REQUEST_URI"] = msg.path;
    cgi_env_copy["SERVER_PROTOCOL"] = msg.version;
    if (msg.method == HTTrequestMSG::POST) {
        cgi_env_copy["CONTENT_TYPE"] = headers_copy["Content-Type"];
        cgi_env_copy["CONTENT_LENGTH"] = utils::to_string(msg.content_length);
    }
    if (msg.method == HTTrequestMSG::GET && !msg.query.empty()) {
        cgi_env_copy["QUERY_STRING"] = msg.query;
    }
    if (!msg.boundary.empty()) {
        cgi_env_copy["BOUNDARY"] = msg.boundary;
    }
    cgi_env_copy["GATEWAY_INTERFACE"] = "CGI/1.1";
    cgi_env_copy["SCRIPT_NAME"] = msg.path;
    cgi_env_copy["SERVER_NAME"] = headers_copy["Host"];
    size_t colonPos = headers_copy["Host"].find(":");
    std::string serverPort = (colonPos != std::string::npos) ? headers_copy["Host"].substr(colonPos + 1) : "80";
    cgi_env_copy["SERVER_PORT"] = serverPort;

    // Print statements for debugging
    printf("CGI Environment Variables:\n");
    for (const auto& cgi_var : cgi_env_copy) {
        printf("  %s: %s\n", cgi_var.first.c_str(), cgi_var.second.c_str());
    }
}

bool HTTPParser::processChunkedBody(std::string& raw, HTTrequestMSG& msg, size_t maxSize) {
    size_t pos = 0;
    while (pos < raw.size()) {
        // Encontra o final da linha que contém o tamanho do chunk
        size_t chunkSizeEnd = raw.find(HTTP_LINE_BREAK, pos);
        if (chunkSizeEnd == std::string::npos) {
            return false; // Não foi possível encontrar o final da linha do tamanho do chunk
        }

        // Extrai o tamanho do chunk e converte de hex para decimal
        std::string chunkSizeHex = raw.substr(pos, chunkSizeEnd - pos);
        int chunkSize = parseHex(chunkSizeHex);
        if (chunkSize == 0) {
            // Chegamos ao último chunk, então podemos considerar a mensagem como concluída
            msg.state = HTTrequestMSG::FINISH;
            return true;
        }

        // Calcula a posição inicial dos dados do chunk
        size_t chunkDataStart = chunkSizeEnd + HTTP_LINE_BREAK.length();
        size_t chunkDataEnd = chunkDataStart + chunkSize;

        // Verifica se temos dados suficientes no buffer
        if (chunkDataEnd + HTTP_LINE_BREAK.length() > raw.size()) {
            return false; // Não temos dados suficientes, esperamos mais dados do cliente
        }

        // Adiciona o corpo do chunk ao corpo da mensagem
        msg.body.append(raw.substr(chunkDataStart, chunkSize));

        // Atualiza o processamento de bytes
        msg.process_bytes += chunkSize;
        if (msg.process_bytes > maxSize) {
            msg.error = "Request entity too large";
            msg.state = HTTrequestMSG::FINISH;
            return false;
        }

        // Atualiza a posição para o próximo chunk
        pos = chunkDataEnd + HTTP_LINE_BREAK.length();
    }

    // Prepara para receber mais dados se não terminou
    raw.erase(0, pos);
    return true;
}

bool HTTPParser::parseHeader(std::string& raw, HTTrequestMSG& msg) {
    size_t end = raw.find(DELIMITER);
    if (end == std::string::npos) {
        printf("Delimiter not found\n");
        return false;
    }

    std::istringstream stream(raw.substr(0, end));
    readRequestLine(stream, msg);
    readHeaders(stream, msg);

    // Extrair boundary se necessário
    if (msg.headers["Content-Type"].find("multipart/form-data") != std::string::npos) {
        msg.boundary = getBoundary(msg.headers["Content-Type"]);
    }

    raw.erase(0, end + DELIMITER.length());
    printf("Header parsing successful\n");
    // Atualizar o estado da mensagem
    msg.state = HTTrequestMSG::TRANSFER_CONTROL;
    return true;
}


void HTTPParser::readRequestLine(std::istringstream& stream, HTTrequestMSG& msg) {
    std::string method, path, version;
    stream >> method >> path >> version;
    setMethod(method, msg);
    msg.version = version;

    size_t delim = path.find("?");
    if (delim != std::string::npos) {
        msg.query = path.substr(delim + 1);
        path.erase(delim);
    }
    msg.path = path;
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
    if (!s.empty() && s[s.size() - 1] == '\r') {
        s.erase(s.size() - 1);
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
        msg.content_length = std::atoi(msg.headers["content-length"].c_str());
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

bool HTTPParser::processMultipartData(const std::string& raw, const std::string& boundary, HTTrequestMSG& msg, size_t maxSize) {
    std::string delimiter = "--" + boundary + "\r\n";
    std::string endDelimiter = "--" + boundary + "--";
    size_t pos = 0;
    size_t endPos = 0;

    while ((pos = raw.find(delimiter, pos)) != std::string::npos) {
        pos += delimiter.length();
        endPos = raw.find(delimiter, pos);
        if (endPos == std::string::npos) {
            endPos = raw.find(endDelimiter, pos);
            if (endPos != std::string::npos) {
                parsePart(raw.substr(pos, endPos - pos), msg);
                break;
            }
        }
        parsePart(raw.substr(pos, endPos - pos - 2), msg); // -2 to handle the trailing "\r\n"
        pos = endPos;
    }
    return true;  // Garanta que este método retorna um bool
}

void HTTPParser::parsePart(const std::string& part, HTTrequestMSG& msg) {
    std::istringstream stream(part);
    std::string line;
    std::string headers = "";
    std::string content;
    bool inHeader = true;

    while (std::getline(stream, line)) {
        if (line.empty() || line == "\r") {
            inHeader = false; // Headers ended, next is content
            continue;
        }
        if (inHeader) {
            headers += line + "\n";
        } else {
            content += line;
        }
    }
    // Process headers and content here
    std::istringstream headerStream(headers);
    std::string header;
    while (std::getline(headerStream, header, '\n')) {
        size_t pos = header.find(": ");
        if (pos != std::string::npos) {
            std::string key = header.substr(0, pos);
            std::string value = header.substr(pos + 2);
            msg.headers[key] = value;
        }
    }

    // Save content to msg.body or do additional processing if needed
    msg.body = content;
}


std::string HTTPParser::getBoundary(const std::string& contentType) {
    size_t pos = contentType.find("boundary=");
    if (pos == std::string::npos) {
        return "";  // Return an empty string if no boundary is found
    }
    std::string boundary = contentType.substr(pos + 9); // 9 is the length of "boundary="
    if (boundary.front() == '"') {
        boundary = boundary.substr(1, boundary.size() - 2); // Remove quotes if present
    }
    return boundary;
}




