#include "../includes/erros.hpp"

ServerErrorHandler::ServerErrorHandler() {
    initializeErrorMessages();
}

// Inicializa o mapa de mensagens de erro
void ServerErrorHandler::initializeErrorMessages() {
    errorMessages[400] = "Bad Request";
    errorMessages[403] = "Forbidden";
    errorMessages[404] = "Not Found";
    errorMessages[500] = "Internal Server Error";
    errorMessages[502] = "Bad Gateway";
    // Adicionar mais erros conforme vamos vendo que aparecem
}

std::string ServerErrorHandler::generateErrorPage(int errorCode, const std::string& errorMessage) {
    std::ostringstream html;
    html << "<!DOCTYPE html>"
         << "<html lang=\"en\">"
         << "<head>"
         << "<meta charset=\"UTF-8\">"
         << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
         << "<style>"
         << "body { font-family: Arial, sans-serif; text-align: center; padding: 50px; }"
         << "h1 { font-size: 50px; }"
         << "p { font-size: 20px; color: #333; }"
         << ".error-container { max-width: 600px; margin: auto; }"
         << "</style>"
         << "<title>Error " << errorCode << "</title>"
         << "</head>"
         << "<body>"
         << "<div class=\"error-container\">"
         << "<h1>Error " << errorCode << "</h1>"
         << "<p>" << errorMessage << "</p>"
         << "</div>"
         << "</body>"
         << "</html>";
    return html.str();
}

// Gera uma página de erro HTML baseada no código de erro
std::string ServerErrorHandler::generateErrorPage(int errorCode) {
    std::map<int, std::string>::iterator it = errorMessages.find(errorCode);
    if (it != errorMessages.end()) {
        return generateErrorPage(errorCode, it->second);
    } else {
        return generateErrorPage(500, "Internal Server Error"); // Erro padrão
    }
}