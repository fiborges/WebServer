#ifndef REQUESTPARSER_HPP
#define REQUESTPARSER_HPP

#include "librarie.hpp"

namespace utils {
    template <typename T>
    std::string to_string(T value) {
        std::ostringstream os;
        os << value;
        return os.str();
    }
}

class HTTrequestMSG {
public:
    enum Method { GET, POST, DELETE, UNKNOWN };
    enum State { HEADERS, BODY, TRANSFER_CONTROL, CONTENT_LENGTH, CHUNKED, FINISH };

    Method method;
    State state;
    std::string path;
    std::string version;
    std::string query;
    std::map<std::string, std::string> headers;
    std::string body;
    int content_length;
    int process_bytes;
    std::string error;
    std::string boundary;
    bool is_cgi;
    std::map<std::string, std::string> cgi_env;
    std::string temp_file_path;
    std::string hostname;

    HTTrequestMSG() : method(UNKNOWN), state(HEADERS), content_length(0), process_bytes(0), is_cgi(false) {}
};

class HTTPParser {
public:
    static const std::string HTTP_LINE_BREAK;
    static const std::string FINAL_CHUNK;
    static const std::string DELIMITER;

    static bool parseRequest(std::string& raw, HTTrequestMSG& msg, size_t maxSize);
    static std::string methodToString(HTTrequestMSG::Method method);
    static bool processChunkedBody(std::string& raw, HTTrequestMSG& msg, size_t maxSize);
    static bool processMultipartData(const std::string& raw, const std::string& boundary, HTTrequestMSG& msg);
    static void parsePart(const std::string& part, HTTrequestMSG& msg);
    static std::string getBoundary(const std::string& contentType);
    static void setupCGIEnvironment(HTTrequestMSG& msg);
    static std::string generateTempFileName();
    static bool saveRequestBodyToFile(const std::string& body, std::string& filePath);

    size_t getContentLength(const std::string& request);

private:
    static void readRequestLine(std::istringstream& stream, HTTrequestMSG& msg);
    static void readHeaders(std::istringstream& stream, HTTrequestMSG& msg);
    static bool readRequestLine2(std::istringstream& stream, HTTrequestMSG& msg);
    static bool readHeaders2(std::istringstream& stream, HTTrequestMSG& msg);
    static bool parseHeader(std::string& raw, HTTrequestMSG& msg);
    static void setMethod(const std::string& method, HTTrequestMSG& msg);
    static void removeCarriageReturn(std::string& s);
    static void setContentLength(HTTrequestMSG& msg);
    static bool isChunkedTransferEncoding(const HTTrequestMSG& msg);
    static int parseHex(const std::string& strNum);
};

#endif

