#ifndef SERVER_ERROR_HANDLER_HPP
#define SERVER_ERROR_HANDLER_HPP

#include "librarie.hpp"
#include "URIparser.hpp"
#include "parser.hpp"

class ServerErrorHandler {
private:
    static std::map<int, std::string> errorMessages;

public:
    static void initialize(); // Função para inicializar o mapa de erros
    static std::string getErrorMessage(int errorCode);
    //void sendErrorResponse(HttpResponse& response, int errorCode, const std::string& errorMsg);

};

#endif
