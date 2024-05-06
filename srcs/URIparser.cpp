#include "../includes/URIparser.hpp"
#include "../includes/erros.hpp"

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
                //sendErrorResponse(response, 501, msg.error);
                return false;
            }
            msg.state = (msg.method == HTTrequestMSG::GET || msg.method == HTTrequestMSG::DELETE || msg.method == HTTrequestMSG::UNKNOWN) ? HTTrequestMSG::FINISH : HTTrequestMSG::TRANSFER_CONTROL;
            break;
        case HTTrequestMSG::BODY:
            if (msg.process_bytes + raw.length() > maxSize) {
                msg.error = ServerErrorHandler::getErrorMessage(413);
                //sendErrorResponse(response, 413, msg.error);
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
                msg.error = ServerErrorHandler::getErrorMessage(413);
                //sendErrorResponse(response, 413, msg.error);
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
            // Processa cada chunk(MODULO) até encontrar o chunk final "0"
            while (raw.find(FINAL_CHUNK) == std::string::npos) {
                size_t chunk_size_end = raw.find(HTTP_LINE_BREAK);
                if (chunk_size_end == std::string::npos) {
                    msg.error = ServerErrorHandler::getErrorMessage(400); // "Bad Request"
                    //sendErrorResponse(response, 400, msg.error);
                    return false;
                }
                int chunk_size = parseHex(raw.substr(0, chunk_size_end));
                if (chunk_size == 0) {
                    msg.state = HTTrequestMSG::FINISH;
                    return true;
                }
                if (static_cast<size_t>(msg.process_bytes) + chunk_size > maxSize) {
                    msg.error = ServerErrorHandler::getErrorMessage(413);
                    //sendErrorResponse(response, 413, msg.error); // "Request Entity Too Large"
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
        msg.error = ServerErrorHandler::getErrorMessage(400);
        //sendErrorResponse(response, 400, msg.error);
        return false;
    }

    std::istringstream stream(raw.substr(0, end));
    readRequestLine(stream, msg);
    readHeaders(stream, msg);

    raw.erase(0, end + DELIMITER.length());
    return true;
}

// detecta se a requisição é CGI
void HTTPParser::readRequestLine(std::istringstream& stream, HTTrequestMSG& msg) {
    std::string method, path, version;
    stream >> method >> path >> version;
    setMethod(method, msg);
    msg.version = version;

    if (path.find("/cgi-bin/") != std::string::npos ||
        (path.size() >= 4 && path.substr(path.size() - 4) == ".cgi")) {
        msg.is_cgi = true;
    }

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

            // Configurar variáveis de ambiente cgi
            if (msg.is_cgi) {
                if (key == "Content-Length") {
                    msg.cgi_env["CONTENT_LENGTH"] = value;
                } else if (key == "Content-Type") {
                    msg.cgi_env["CONTENT_TYPE"] = value;
                }
            }
        }
    }

    if (msg.is_cgi) {
        msg.cgi_env["REQUEST_METHOD"] = methodToString(msg.method);
        msg.cgi_env["SCRIPT_NAME"] = msg.path;
        msg.cgi_env["QUERY_STRING"] = msg.query;
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
        msg.error = ServerErrorHandler::getErrorMessage(501);
        // "Not Implemented"
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
