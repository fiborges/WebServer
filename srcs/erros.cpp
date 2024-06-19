#include "../includes/erros.hpp"

ServerErrorHandler::ServerErrorHandler() {
    initializeErrorMessages();
}

// Inicializa o mapa de mensagens de erro
void ServerErrorHandler::initializeErrorMessages() {
    errorMessages[400] = "Bad Request";
    errorMessages[401] = "Unauthorized";
    errorMessages[403] = "Forbidden";
    errorMessages[404] = "Not Found";
    errorMessages[405] = "Method Not Allowed";
    errorMessages[406] = "Not Acceptable";
    errorMessages[407] = "Proxy Authentication Required";
    errorMessages[408] = "Request Timeout";
    errorMessages[409] = "Conflict";
    errorMessages[410] = "Gone";
    errorMessages[411] = "Length Required";
    errorMessages[412] = "Precondition Failed";
    errorMessages[413] = "Payload Too Large";
    errorMessages[414] = "URI Too Long";
    errorMessages[415] = "Unsupported Media Type";
    errorMessages[416] = "Range Not Satisfiable";
    errorMessages[417] = "Expectation Failed";
    errorMessages[418] = "I'm a teapot";
    errorMessages[421] = "Misdirected Request";
    errorMessages[422] = "Unprocessable Entity";
    errorMessages[423] = "Locked";
    errorMessages[424] = "Failed Dependency";
    errorMessages[426] = "Upgrade Required";
    errorMessages[428] = "Precondition Required";
    errorMessages[429] = "Too Many Requests";
    errorMessages[431] = "Request Header Fields Too Large";
    errorMessages[451] = "Unavailable For Legal Reasons";
    errorMessages[500] = "Internal Server Error";
    errorMessages[501] = "Not Implemented";
    errorMessages[502] = "Bad Gateway";
    errorMessages[503] = "Service Unavailable";
    errorMessages[504] = "Gateway Timeout";
    errorMessages[505] = "HTTP Version Not Supported";
    errorMessages[506] = "Variant Also Negotiates";
    errorMessages[507] = "Insufficient Storage";
    errorMessages[508] = "Loop Detected";
    errorMessages[510] = "Not Extended";
    errorMessages[511] = "Network Authentication Required";
    // Adicionar mais erros conforme vamos vendo que aparecem
}

std::string ServerErrorHandler::generateErrorPage(int errorCode, const std::string& errorMessage)
{
    std::ostringstream html;
    html << "<!DOCTYPE html>"
         << "<html lang=\"en\">"
         << "<head>"
         << "<meta charset=\"UTF-8\">"
         << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
         << "<style>"
         << "body { font-family: Arial, sans-serif; text-align: center; padding: 50px; background-color: #ADD8E6; }"
         << "h1 { font-size: 50px; }"
         << ".errorCode { color: red; }"
         << "p { font-size: 20px; color: #333; }"
         << ".error-container { max-width: 600px; margin: auto; }"
         << "</style>"
         << "<title>Error " << errorCode << "</title>"
         << "</head>"
         << "<body>"
         << "<div class=\"error-container\">"
         << "<h1 class=\"errorCode\">Error " << errorCode << "</h1>"
         << "<p>" << errorMessage << "</p>"
         << "<br>"
         << "<button onclick=\"location.href='/'\">Home</button>"
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

std::string ServerErrorHandler::getErrorMessage(int errorCode)
{
    std::map<int, std::string>::iterator it = errorMessages.find(errorCode);
    if (it != errorMessages.end())
    {
        return it->second;
    }
    else
    {
        return "Unknown Error"; // Default error message
    }
}