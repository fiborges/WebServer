#include "../includes/RequestParser.hpp"

const std::string HTTPParser::HTTP_LINE_BREAK = "\r\n";
const std::string HTTPParser::FINAL_CHUNK = "0\r\n\r\n";
const std::string HTTPParser::DELIMITER = HTTP_LINE_BREAK + HTTP_LINE_BREAK;


bool HTTPParser::parseRequest(std::string& raw, HTTrequestMSG& msg, size_t maxSize) {
    if (!parseHeader(raw, msg)) {
        msg.error = "Header parsing failed";
        return false;
    }
    setContentLength(msg);
    if (msg.path.find("cgi") != std::string::npos) {
        msg.is_cgi = true;
        std::string boundary = getBoundary(msg.headers["Content-Type"]);
        setupCGIEnvironment(msg);
    }
    if (msg.is_cgi && msg.headers["Content-Type"].find("multipart/form-data") != std::string::npos) {
        std::string boundary = getBoundary(msg.headers["Content-Type"]);
        if (boundary.empty()) {
            msg.error = "Bad Request: No boundary in multipart/form-data";
            return false;
        }
        return processMultipartData(raw, boundary, msg, maxSize);

    } else if (msg.headers["transfer-encoding"].find("chunked") != std::string::npos) {
        return processChunkedBody(raw, msg, maxSize);
    } else {
        if (msg.content_length > 0) {
            if (raw.length() < msg.content_length) {
                msg.error = "Incomplete Data";
                return false;
            }
            msg.body = raw.substr(0, msg.content_length);
            raw.erase(0, msg.content_length);
        }
    }
    if (!msg.body.empty()) {
        std::string tempFilePath = generateTempFileName();
        if (!saveRequestBodyToFile(msg.body, tempFilePath)) {
            msg.error = "Failed to save request body to file";
            return false;
        } else {
            std::cout << "Request body saved to " << tempFilePath << std::endl;
        }
        msg.temp_file_path = tempFilePath;
        printf("-----------------------Temp file path: %s\n", msg.temp_file_path.c_str());
    }
    return true;
}

std::string HTTPParser::getBoundary(const std::string& contentType) {
    size_t pos = contentType.find("boundary=");
    if (pos == std::string::npos) {
        return "";  // Não foi encontrado boundary.
    }
    pos += 9; // Tamanho de "boundary="
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
        // Definir variáveis de ambiente específicas para CGI
        if (msg.method == HTTrequestMSG::POST) {
            msg.cgi_env["CONTENT_TYPE"] = msg.headers["Content-Type"];
            msg.cgi_env["CONTENT_LENGTH"] = std::to_string(msg.content_length);
        }
        if (msg.method == HTTrequestMSG::GET && !msg.query.empty()) {
            msg.cgi_env["QUERY_STRING"] = msg.query;
        }
        // Verificar se a solicitação é multipart e incluir o boundary, se aplicável
        if (!msg.boundary.empty()) {
            msg.cgi_env["BOUNDARY"] = msg.boundary;
        }
    }
    // Configurar o número da porta do servidor
    size_t colonPos = msg.headers["Host"].find(":");
    std::string serverPort = (colonPos != std::string::npos) ? msg.headers["Host"].substr(colonPos + 1) : "80";
    msg.cgi_env["SERVER_PORT"] = serverPort;
}

bool HTTPParser::processChunkedBody(std::string& raw, HTTrequestMSG& msg, size_t maxSize) {
    size_t pos = 0;
    while (pos < raw.size()) {
        // Encontra o final da linha que contém o tamanho do chunk
        size_t chunkSizeEnd = raw.find(HTTP_LINE_BREAK, pos);
        if (chunkSizeEnd == std::string::npos) {
            return false; // Não foi possível encontrar o final da linha
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

void HTTPParser::setContentLength(HTTrequestMSG& msg){
    if (msg.headers.count("Content-Length") > 0) {
        std::string contentLengthStr = msg.headers["Content-Length"];
        if (!contentLengthStr.empty() && std::all_of(contentLengthStr.begin(), contentLengthStr.end(), ::isdigit)) {
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
    while (std::getline(headerStream, header)) {
        if (header.empty()) continue;
        size_t pos = header.find(": ");
        if (pos != std::string::npos) {
            std::string key = header.substr(0, pos);
            std::string value = header.substr(pos + 2);
            msg.headers[key] = value;  // Atualiza os cabeçalhos do HTTPrequestMSG
        }
    }

    // Anexa o conteúdo extraído ao corpo do HTTPrequestMSG
    msg.body += content;
}


std::string HTTPParser::generateTempFileName() {
    char buffer[] = "/tmp/tempfileXXXXXX";  // Path para o arquivo temporário com placeholders para mkstemp
    int fd = mkstemp(buffer);  // Cria um arquivo temporário seguro
    if (fd == -1) {
        std::cerr << "Failed to create a temporary file: " << std::strerror(errno) << std::endl;
        return "";  // Retorna string vazia em caso de erro
    }

    close(fd);  // Fecha o arquivo pois apenas queremos o nome
    return std::string(buffer);  // Retorna o caminho para o arquivo temporário
}

bool HTTPParser::saveRequestBodyToFile(const std::string& body, std::string& filePath) {
    std::ofstream outputFile(filePath.c_str(), std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);  // Abre o arquivo para escrita
    if (!outputFile) {
        std::cerr << "Failed to open the file for writing: " << filePath << std::endl;
        return false;  // Retorna falso se falhar ao abrir o arquivo
    }

    outputFile << body;  // Escreve o corpo no arquivo
    outputFile.close();  // Fecha o arquivo
    return true;  // Retorna verdadeiro se tudo ocorrer bem
}

bool HTTPParser::processMultipartData(const std::string& raw, const std::string& boundary, HTTrequestMSG& msg, size_t maxSize) {
    std::string delimiter = "--" + boundary + "\r\n";
    std::string endDelimiter = "--" + boundary + "--";
    size_t pos = 0;
    size_t endPos = raw.find(delimiter);

    while (endPos != std::string::npos) {
        size_t start = pos + delimiter.length();
        endPos = raw.find(delimiter, start);
        if (endPos == std::string::npos) endPos = raw.find(endDelimiter, start);
        if (endPos != std::string::npos) {
            parsePart(raw.substr(start, endPos - start), msg); // Processa cada parte
            pos = endPos + delimiter.length();
        }
    }

    // Após processar todas as partes, salva o corpo completo no arquivo temporário
    std::string tempFilePath = generateTempFileName();
    if (!saveRequestBodyToFile(msg.body, tempFilePath)) {
        msg.error = "Failed to save request body to file";
        return false;
    }
    std::cout << BLUE << "Request body saved to " << RESET << tempFilePath << std::endl;
    msg.temp_file_path = tempFilePath;

    return true;
}

