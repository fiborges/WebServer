#ifndef URIPARSER_HPP
#define URIPARSER_HPP

#include "librarie.hpp"

class HTTrequestMSG {
public:
    //GET é representado pelo valor 0. POST é representado pelo valor 1. DELETE é representado pelo valor 2.
    enum Method { GET, POST, DELETE, UNKNOWN };

    // representa o estado da mensagem durante o processamento
    /*
    HEADERS: indica que a mensagem está a ser processada no header.
    BODY: Indica que a mensagem está no corpo da requisição.
    TRANSFER_CONTROL: Este estado é utilizado para controlar a transferência de dados, por exemplo, quando a codificação de transferência é utilizada, como "chunked" ou quando o tamanho do conteúdo é especificado no cabeçalho Content-Length.
    CONTENT_LENGTH: Este estado é ativado quando o tamanho do conteúdo da mensagem é conhecido, e o corpo da mensagem está a ser processado até que a quantidade de bytes indicada seja alcançada.
    CHUNKED: Este estado é utilizado quando a mensagem é enviada com codificação de transferência "chunked". Isso significa que o corpo da mensagem é dividido em pedaços (chunks) e cada chunk é precedido por seu próprio tamanho em hexadecimal.
    FINISH: Indica que o processamento da mensagem foi concluído com sucesso.
    */
    enum State { HEADERS, BODY, TRANSFER_CONTROL, CONTENT_LENGTH, CHUNKED, FINISH };

    Method method;                          // Método da requisição
    State state;                            // Estado atual da mensagem durante o processamento
    std::string path;                       // Caminho do recurso solicitado
    std::string version;                    // Versão do protocolo HTTP
    std::string query;                      // Parâmetros de consulta da URL
    std::map<std::string, std::string> headers; // Cabeçalhos HTTP
    std::string body;                       // Corpo da requisição
    int content_lenght;                            // Comprimento do conteúdo da requisição (Content-Length)
    int process_bytes;                            // Quantidade de bytes já processados
    std::string error;                      // Mensagem de erro, se houver
    bool is_cgi;                             //flag para cgi
    std::map<std::string, std::string> cgi_env;  // // Variáveis de ambiente para o CGI


    // Construtor padrão inicializa alguns atributos
    HTTrequestMSG() : method(UNKNOWN), state(HEADERS), content_lenght(0), process_bytes(0), is_cgi(false) {}
};

class HTTPParser 
{
public:
    // Constantes para delimitar e marcar o que já foi usado durante o processamento da mensagem
    static const std::string FINAL_CHUNK;   // indica o final de um chunk
    static const std::string HTTP_LINE_BREAK;          // Caracteres de retorno e nova linha
    static const std::string DELIMITER;     // Delimitador para separar cabeçalhos e corpo da mensagem

    // funcao main de analise de uma requisição HTTP
    static bool parseRequest(std::string& raw, HTTrequestMSG& msg, size_t maxSize);

    // Método para converter HTTP numa string so para teste basicamente
    static std::string methodToString(HTTrequestMSG::Method method);

private:

    static void readRequestLine(std::istringstream& stream, HTTrequestMSG& msg);  // Lê a linha de requisição
    static void readHeaders(std::istringstream& stream, HTTrequestMSG& msg);      // Lê os cabeçalhos da requisição
    static bool parseHeader(std::string& raw, HTTrequestMSG& msg);                // Analisa um cabeçalho
    static void setMethod(const std::string& method, HTTrequestMSG& msg);         // Define o método da requisição
    static void removeCarriageReturn(std::string& s);                                   // Remove o último caractere Cariage Return de uma string
    static void setContentLength(HTTrequestMSG& msg);                             // Define o comprimento do conteúdo da requisição
    static bool isChunkedTransferEncoding(const HTTrequestMSG& msg);               // Verifica se a transferência é feita em blocos
    static int parseHex(const std::string& strNum);                         // Converte uma string hexadecimal num número inteiro
};


#endif
