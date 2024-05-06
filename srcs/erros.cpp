#include "../includes/erros.hpp"

std::map<int, std::string> ServerErrorHandler::errorMessages;

void ServerErrorHandler::initialize() {
    errorMessages[400] = "Bad Request";
    errorMessages[404] = "Not Found";
    errorMessages[500] = "Internal Server Error";
    errorMessages[501] = "Not Implemented";
    errorMessages[503] = "Service Unavailable";
}

std::string ServerErrorHandler::getErrorMessage(int errorCode) {
    std::map<int, std::string>::iterator it = errorMessages.find(errorCode);
    if (it != errorMessages.end()) {
        return it->second;
    }
    return "Unknown Error";
}

//função para depois responder ao cliente quando tratarmos dessa parte
/*
void sendErrorResponse(HttpResponse& response, int errorCode, const std::string& errorMsg) {
    response.setStatus(errorCode);
    response.setBody("<html><body><h1>Error " + std::to_string(errorCode) + "</h1><p>" + errorMsg + "</p></body></html>");
    response.send();
}
*/


/*
como usar:

int main() {
    ServerErrorHandler::initialize(); // Inicializa as mensagens de erro

    // Aqui vai o restante do código do servidor
    return 0;
}

void handleHttpRequest(const HttpRequest& request, HttpResponse& response) {
    try {
        // Código para processar a requisição
    } catch (const std::exception& e) {
        int errorCode = 500; // Suponha que algo identifique o código de erro como 500
        response.setStatus(errorCode);
        response.setBody("<html><body><h1>Error</h1><p>" + ServerErrorHandler::getErrorMessage(errorCode) + "</p></body></html>");
    }
}

*/
