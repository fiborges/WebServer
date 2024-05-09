# ifndef PARSER_HPP
#define PARSER_HPP

#include "librarie.hpp"
#include "parser_utils.hpp"
#include "parserConfig.hpp"
#include "conf_info.hpp"
#include "get.hpp"

class ParserClass;
//Ponteiro para mapear o conf_File_Info que valida e aplica diretivas.
typedef void (ParserClass::*confFileHandler)(const ParserUtils::Strings&, conf_File_Info*);
//Mapa que associa nomes de diretivas a métodos de validação em ParserClass
typedef std::map<std::string, confFileHandler> MapHandler;
//Vetor de ParserConfig, armazena configurações de servidor após parsing
typedef std::vector<ParserConfig> ConfiguredServers;

class ParserClass
{
public:
    ParserClass(const std::string& file_path);
    ~ParserClass();
    //funcao de debug tirar no fim! main
    void debug() const;

    //obter configurações do servidor depois d oparsing
    const ConfiguredServers& fetchSpecifications();

private:
    //Caminho para o arquivo de configuração
    std::string configFilePath;
    // Stream para leitura do arquivo de configuração
    std::ifstream configurationFile;
    //using string identifiers for states
    std::string currentState;
    // Mapa que associa strings a métodos de validação específicos
    MapHandler validationMapKeys;
    // Vetor de conf_File_Info, armazena diretivas de configuração
    std::vector<conf_File_Info> conf_info;
    // Número de módulos carregados
    int numberOfModules;
    // Ponteiro para o arquivo de configuração em andamento
    conf_File_Info* conFileInProgress;
    // Rastreador de linha
    int lineTracker;
    //Pilha usada para rastrear e gerenciar diferentes níveis de configuração durante o parsing
    std::stack<conf_File_Info*> contextHistory;
    // Vetor de ParserConfig, armazena configurações de servidor após parsing
    ConfiguredServers serverConfigurations;

    //Inicia o processo de leitura e interpretação do arquivo de configuração.
    void readAndProcessConfig();
    //Verifica a presença de todos os parâmetros obrigatórios no arquivo de configuração
    void validateRequiredParameters();
    //Retorna true se a linha deve ser ignorada (comentários ou linhas vazias).
    bool lineIsIgnorable(const std::string& fileLine);
    // Identifica o início de um novo bloco de configuração do servidor.
    void locateServerModule(ParserUtils::Strings& segments);
    //Processa os parametros encontrados dentro de um bloco do servidor.
    void handleServerModule(const ParserUtils::Strings& pieces);
    //Analisa e processa um bloco de localização dentro de uma configuração do servidor.
    void parseLocationModule(const ParserUtils::Strings& pieces);
    //Confirma que todos os blocos abertos no arquivo foram corretamente fechados.
    void ensureAllModulesClosed();
    //Verifica e confirma a integridade do mapa de validação de parametros
    void checkAndConfirmValidMap();
    //Formata e retorna uma mensagem de erro com base no texto fornecido.
    std::string createErrorMsg(const std::string& erro_msg);
    // Verifica a validade das configurações de um bloco de servidor com base nos tokens fornecidos.
    void checkServer(const ParserUtils::Strings& pieces);
    // Verifica a validade das configurações de um bloco de localização com base nos tokens fornecidos.
    void checkLocation(const ParserUtils::Strings& pieces);
    // Confirma se as configurações de porta especificadas nos tokens estão corretas e as aplica.
    void confirmListenSettings(const ParserUtils::Strings& parameters, conf_File_Info* Keyword);
    // Confirma se o nome do servidor especificado nos tokens é válido e o aplica ao contexto atual.
    void confirmServerName(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword);
    // Verifica a validade do índice especificado nos tokens para garantir que aponta para um recurso existente.
    void checkIndex(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword);
    // Confirma se o caminho raiz especificado nos tokens é válido e existente no sistema de arquivos.
    void confirmRootPath(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword);
    // Verifica se a configuração de autoindexação especificada nos tokens está correta (habilitada ou desabilitada).
    void checkAutoindex(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword);
    // Verifica a validade das páginas de erro especificadas nos tokens e as associa a códigos de erro apropriados.
    void verifyErrorPage(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword);
    // Confirma se as configurações de CGI especificadas nos tokens são válidas e aplicáveis.
    void confirmCGISettings(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword);
    // Confirma se as regras de redirecionamento especificadas nos tokens são válidas e as implementa.
    void confirmRedirect(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword);
    // Verifica a validade dos métodos HTTP especificados nos tokens para garantir que são permitidos.
    void checkProcedures(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword);
    // Garante que o tamanho do corpo do cliente especificado nos tokens não exceda o limite máximo permitido.
    void ensureClientBodyCapacity(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword);
    // Confirma se o diretório de upload especificado nos tokens é válido e acessível no sistema de arquivos.
    void confirmUploadDir(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword);
    // Inicia o processamento de um novo bloco de configuração do servidor.
    void startServerModule();
    // Inicia o processamento de um novo bloco de configuração de localização dentro de um bloco de servidor.
    void startLocationModule(const std::string& location);
    // Encerra o processamento do bloco de configuração atual e retorna ao contexto anterior.
    void endCurrentModule();
    // Verifica se a quantidade de argumentos fornecida é apropriada para a diretiva atual, lançando erro se não for.
    void ensureCorrectArgNumber(const ParserUtils::Strings& tokens, bool badCondition);
    class ConfigError : public std::invalid_argument
    {
    public:
        ConfigError(const std::string& err);
    };
};

#endif