#ifndef ERROS_HPP
#define ERROS_HPP

#include "librarie.hpp"
#include "RequestParser.hpp"
#include "parser.hpp"
#include "parserConfig.hpp"

class ServerErrorHandler {
public:
    ServerErrorHandler();
    std::string generateErrorPage(int errorCode);
private:
    std::map<int, std::string> errorMessages;
    std::string generateErrorPage(int errorCode, const std::string& errorMessage);
    void initializeErrorMessages();
};

#endif
