#ifndef PARSERCONFIG_HPP
#define PARSERCONFIG_HPP

#include "librarie.hpp"
#include "conf_info.hpp"

class ParserConfig
{
public:
    ParserConfig(conf_File_Info* configData, const std::string& path_location = "/");
    ParserConfig(const ParserConfig& src);
    ~ParserConfig();
    ParserConfig& operator=(const ParserConfig& src);

    //Retorna a porta especificada nas configurações
    int obtainPort() const;
    //Retorna o nome do servidor especificado nas configurações
    const std::string& retrieveServerName() const;
    //Retorna o arquivo padrão para servir quando uma solicitação de diretório é feita
    const std::string& fetchIndex() const;
    //Retorna o diretório raiz do servidor. Se não estiver especificado, retorna o diretório atual
    const std::string acquireRoot() const;
    //Verifica se a lista de diretórios está ok ou não
    bool checkAutoIndex() const;
    //Verifica se há uma página de erro especificada para um determinado código de erro HTTP
    bool validateErrorPage(int errorNumber) const;
    //Retorna a página de erro associada a um código de erro HTTP específico
    const std::string& fetchErrorPage(int errorNumber) const;
    //Verifica se há um script CGI especificado nas configurações
    bool confirmCGI() const;
    //Retorna o caminho para o script CGI especificado nas configurações
    const std::string& accessCGIScript() const;
    //Verifica se há uma configuração de redirecionamento HTTP
    bool verifyRedirection() const;
    //Retorna a configuração de redirecionamento
    const ForwardingURL& fetchRedirection() const;
    //Verifica se um determinado caminho corresponde a um local especificado nas configurações
    std::string matchPath(const std::string& searchPath) const;
    //Retorna uma instância do ParserConfig correspondente ao contexto especificado pelo caminho
    ParserConfig extractContext(const std::string& requestedPath) const;
    //Retorna a localização atual da configuração
    std::string determineLocation() const;
    //Retorna o tamanho máximo do corpo da solicitação do cliente
    int calculateClientBodySize() const;
    //Retorna o diretório de upload especificado nas configurações
    const std::string& obtainUploadDirectory() const;
    //Verifica se um método HTTP específico é permitido para este contexto de configuração
    bool validateMethod(const std::string& httpMethod) const;

    conf_File_Info* getServerConfigurations() const {
        return Server_configurations;
    }

private:
    //Armazena as configurações do servidor associadas ao ParserConfig, incluem informações como a porta, o nome, o índice padrão, o diretório raiz, etc.
    conf_File_Info* Server_configurations;
    //Representa o caminho da localização associada 
    std::string locationPath;
    // usada para retornar uma string vazia
    std::string emptyString;
};

#endif 