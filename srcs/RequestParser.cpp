#include "../includes/RequestParser.hpp"
#include "../includes/erros.hpp"
#include <sstream>

const std::string HTTPParser::HTTP_LINE_BREAK = "\r\n";
const std::string HTTPParser::FINAL_CHUNK = "0\r\n\r\n";
const std::string HTTPParser::DELIMITER = HTTP_LINE_BREAK + HTTP_LINE_BREAK;

bool HTTPParser::parseRequest(std::string& raw, HTTrequestMSG& msg, size_t maxSize) {
    ServerErrorHandler errorHandler;

    if (!parseHeader(raw, msg)) {
        msg.error = errorHandler.generateErrorPage(400);
        std::cout << "Failed to parse header" << std::endl;
        return false;
    }
    setContentLength(msg);

    if (static_cast<size_t>(msg.content_length) > raw.size()) {
        printf("msg content lenght: %d , raw size: %lu ", msg.content_length , raw.size());
        msg.error = errorHandler.generateErrorPage(400);
        std::cout << "Content-Length header is larger than actual data size" << std::endl;
        return false;
    }
    if (msg.path.find(".py") != std::string::npos) {
        msg.is_cgi = true;
        std::cout << "Detected CGI request" << std::endl;
        std::string boundary = getBoundary(msg.headers["Content-Type"]);
        setupCGIEnvironment(msg);
    }
    if (msg.is_cgi && msg.headers["Content-Type"].find("multipart/form-data") != std::string::npos) {
        std::string boundary = getBoundary(msg.headers["Content-Type"]);
        std::cout << "Boundary for multipart/form-data: " << boundary << std::endl;
        if (boundary.empty()) {
            msg.error = errorHandler.generateErrorPage(400);
            std::cout << "No boundary in multipart/form-data" << std::endl;
            return false;
        }
        return processMultipartData(raw, boundary, msg);

    } else if (msg.headers["transfer-encoding"].find("chunked") != std::string::npos) {
        return processChunkedBody(raw, msg, maxSize);
    } else {
        if (msg.content_length > 0) {
            size_t content_length = static_cast<size_t>(msg.content_length);
            if (raw.length() < content_length) {
                msg.error = errorHandler.generateErrorPage(400);
                return false;
            }
            size_t processedSize = 0;
            while (processedSize < content_length) {
                size_t chunkSize = std::min(content_length - processedSize, maxSize);
                msg.body.append(raw, processedSize, chunkSize);
                processedSize += chunkSize;
            }
            raw.erase(0, content_length);
        }
    }
    if (!msg.body.empty()) {
        std::string tempFilePath = generateTempFileName();
        if (!saveRequestBodyToFile(msg.body, tempFilePath)) {
            msg.error = errorHandler.generateErrorPage(500);
            return false;
        }
        msg.temp_file_path = tempFilePath;
    }
    return true;
}

std::string HTTPParser::getBoundary(const std::string& contentType) {
    size_t pos = contentType.find("boundary=");
    if (pos == std::string::npos) {
        return "";
    }
    pos += 9;
    size_t end = contentType.find(';', pos);
    if (end == std::string::npos) {
        end = contentType.length();
    }
    
    while (pos < end && std::isspace(contentType[pos])) {
        ++pos;
    }
    while (end > pos && std::isspace(contentType[end - 1])) {
        --end;
    }
    return contentType.substr(pos, end - pos);
}

void HTTPParser::setupCGIEnvironment(HTTrequestMSG& msg) {
    msg.cgi_env["REQUEST_METHOD"] = methodToString(msg.method);
    msg.cgi_env["REQUEST_URI"] = msg.path;
    msg.cgi_env["SERVER_PROTOCOL"] = msg.version;
    msg.cgi_env["GATEWAY_INTERFACE"] = "CGI/1.1";
    msg.cgi_env["SCRIPT_NAME"] = msg.path;
    msg.cgi_env["SERVER_NAME"] = msg.headers["Host"];

    if (msg.is_cgi) {
        if (msg.method == HTTrequestMSG::POST) {
            msg.cgi_env["CONTENT_TYPE"] = msg.headers["Content-Type"];
            std::stringstream ss;
            ss << msg.content_length;
            msg.cgi_env["CONTENT_LENGTH"] = ss.str();
        }
        if (msg.method == HTTrequestMSG::GET && !msg.query.empty()) {
            msg.cgi_env["QUERY_STRING"] = msg.query;
        }
        if (!msg.boundary.empty()) {
            msg.cgi_env["BOUNDARY"] = msg.boundary;
        }
    }
    size_t colonPos = msg.headers["Host"].find(":");
    std::string serverPort = (colonPos != std::string::npos) ? msg.headers["Host"].substr(colonPos + 1) : "80";
    msg.cgi_env["SERVER_PORT"] = serverPort;
}

bool HTTPParser::processChunkedBody(std::string& raw, HTTrequestMSG& msg, size_t maxSize) {
    ServerErrorHandler errorHandler;
    size_t pos = 0;
    while (pos < raw.size()) {
        size_t chunkSizeEnd = raw.find(HTTP_LINE_BREAK, pos);
        if (chunkSizeEnd == std::string::npos) {
            msg.error = errorHandler.generateErrorPage(400);
            return false;
        }
        std::string chunkSizeHex = raw.substr(pos, chunkSizeEnd - pos);
        int chunkSize = parseHex(chunkSizeHex);
        if (chunkSize == 0) {
            msg.state = HTTrequestMSG::FINISH;
            return true;
        }
        size_t chunkDataStart = chunkSizeEnd + HTTP_LINE_BREAK.length();
        size_t chunkDataEnd = chunkDataStart + chunkSize;

        if (chunkDataEnd + HTTP_LINE_BREAK.length() > raw.size()) {
            msg.error = errorHandler.generateErrorPage(400);
            return false;
        }

        msg.body.append(raw.substr(chunkDataStart, chunkSize));

        msg.process_bytes += chunkSize;
        if (static_cast<size_t>(msg.process_bytes) > maxSize) {
            msg.error = errorHandler.generateErrorPage(413);
            msg.state = HTTrequestMSG::FINISH;
            return false;
        }
        pos = chunkDataEnd + HTTP_LINE_BREAK.length();
    }
    raw.erase(0, pos);
    return true;
}

bool HTTPParser::parseHeader(std::string& raw, HTTrequestMSG& msg) {
    ServerErrorHandler errorHandler;
    size_t pos = raw.find("\r\n");
    if (pos == std::string::npos) {
        msg.error = errorHandler.generateErrorPage(400);
        return false;
    }
    std::string requestLine = raw.substr(0, pos);
    raw.erase(0, pos + 2);
    std::istringstream requestLineStream(requestLine);
    if (!readRequestLine2(requestLineStream, msg)) {
        msg.error = errorHandler.generateErrorPage(400);
        return false;
    }
    
    pos = raw.find("\r\n\r\n");
    if (pos == std::string::npos) {
        msg.error = errorHandler.generateErrorPage(400);
        return false;
    }
    std::string headers = raw.substr(0, pos);
    raw.erase(0, pos + 4);
    std::istringstream headersStream(headers);
    if (!readHeaders2(headersStream, msg)) {
        msg.error = errorHandler.generateErrorPage(400);
        return false;
    }
    std::string hostHeader = msg.headers["Host"];
    msg.hostname = hostHeader;
   
    return true;
}

bool HTTPParser::readRequestLine2(std::istringstream& stream, HTTrequestMSG& msg) {
    std::string method, path, version;
    stream >> method >> path >> version;
    if (stream.fail()) {
        return false;
    }
    setMethod(method, msg);
    msg.version = version;

    size_t delim = path.find("?");
    if (delim != std::string::npos) {
        msg.query = path.substr(delim + 1);
        path.erase(delim);
    }
    msg.path = path;
    return true;
}

bool HTTPParser::readHeaders2(std::istringstream& stream, HTTrequestMSG& msg) {
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
    if (stream.fail() && !stream.eof()) {
        return false;
    }
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

void HTTPParser::setContentLength(HTTrequestMSG& msg){
    if (msg.headers.count("Content-Length") > 0) {
        std::string contentLengthStr = msg.headers["Content-Length"];
        bool allDigits = true;
        for (std::string::const_iterator it = contentLengthStr.begin(); it != contentLengthStr.end(); ++it) {
            if (!::isdigit(*it)) {
                allDigits = false;
                break;
            }
        }
        if (!contentLengthStr.empty() && allDigits) {
            int contentLength = std::atoi(contentLengthStr.c_str());
            msg.content_length = contentLength;
        } else {
            msg.error = "Invalid Content-Length value";
        }
    } else {
        msg.content_length = 0;
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

void HTTPParser::parsePart(const std::string& part, HTTrequestMSG& msg) {
    std::istringstream stream(part);
    std::string line;
    std::string headers = "";
    std::string content;
    bool inHeader = true;

    while (std::getline(stream, line)) {
        if (line.empty() || line == "\r") {
            inHeader = false;
            continue;
        }
        if (inHeader) {
            headers += line + "\n";
        } else {
            content += line;
        }
    }

    std::istringstream headerStream(headers);
    std::string header;
    while (std::getline(headerStream, header)) {
        if (header.empty()) continue;
        size_t pos = header.find(": ");
        if (pos != std::string::npos) {
            std::string key = header.substr(0, pos);
            std::string value = header.substr(pos + 2);
            msg.headers[key] = value;
        }
    }

    msg.body += content;
}

std::string HTTPParser::generateTempFileName() {
    char buffer[] = "/tmp/tempfileXXXXXX";
    int fd = mkstemp(buffer);
    if (fd == -1) {
        std::cerr << "Failed to create a temporary file: " << std::strerror(errno) << std::endl;
        return "";
    }

    close(fd);
    return std::string(buffer);
}

bool HTTPParser::saveRequestBodyToFile(const std::string& body, std::string& filePath) {
    std::ofstream outputFile(filePath.c_str(), std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);
    if (!outputFile) {
        std::cerr << "Failed to open the file for writing: " << filePath << std::endl;
        return false;
    }

    outputFile << body;
    outputFile.close();
    return true;
}

bool HTTPParser::processMultipartData(const std::string& raw, const std::string& boundary, HTTrequestMSG& msg) {
    std::string delimiter = "--" + boundary + "\r\n";
    std::string endDelimiter = "--" + boundary + "--";
    size_t pos = 0;
    size_t endPos = raw.find(delimiter);

    while (endPos != std::string::npos) {
        size_t start = pos + delimiter.length();
        endPos = raw.find(delimiter, start);
        if (endPos == std::string::npos) endPos = raw.find(endDelimiter, start);
        if (endPos != std::string::npos) {
            parsePart(raw.substr(start, endPos - start), msg);
            pos = endPos + delimiter.length();
        }
    }

    std::string tempFilePath = generateTempFileName();
    if (!saveRequestBodyToFile(msg.body, tempFilePath)) {
        msg.error = "Failed to save request body to file";
        return false;
    }
    
    msg.temp_file_path = tempFilePath;

    return true;
}

size_t HTTPParser::getContentLength(const std::string& request)
{
    const std::string contentLengthHeader = "Content-Length: ";
    size_t start = request.find(contentLengthHeader);
    if (start == std::string::npos)
    {
        return 0;
    }

    start += contentLengthHeader.size();
    size_t end = request.find("\r\n", start);
    if (end == std::string::npos)
    {
        return 0;
    }

    std::string contentLengthStr = request.substr(start, end - start);
    std::istringstream iss(contentLengthStr);
    size_t contentLength;
    iss >> contentLength;
    return contentLength;
}