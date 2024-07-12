# ifndef PARSER_HPP
#define PARSER_HPP

#include "librarie.hpp"
#include "parser_utils.hpp"
#include "parserConfig.hpp"
#include "conf_info.hpp"
#include "get.hpp"
#include "CGI.hpp"
#include "erros.hpp"

class ParserClass;
typedef std::vector<ParserConfig> ConfiguredServers;
typedef void (ParserClass::*confFileHandler)(const ParserUtils::Strings&, conf_File_Info*);

class ParserClass {
public:
    ParserClass(const std::string& file_path);
    ~ParserClass();

    const ConfiguredServers& fetchSpecifications();

    class ConfigError : public std::runtime_error {
    public:
        explicit ConfigError(const std::string& err);
    };

private:
    std::string configFilePath;
    std::ifstream configurationFile;
    std::vector<conf_File_Info> conf_info;
    ConfiguredServers serverConfigurations;
    std::map<std::string, confFileHandler> validationMapKeys;
    std::stack<conf_File_Info*> contextHistory;
    conf_File_Info* conFileInProgress;
    std::string currentState;
    int numberOfModules;
    int lineTracker;
    std::string locationPrefix;

    void readAndProcessConfig();
    void validateRequiredParameters();
    bool lineIsIgnorable(const std::string& line) const;
    void locateServerModule(const ParserUtils::Strings& pieces);
    void handleServerModule(const ParserUtils::Strings& pieces);
    void parseLocationModule(const ParserUtils::Strings& pieces);
    void ensureAllModulesClosed();
    void checkAndConfirmValidMap();
    void handleHost(const ParserUtils::Strings& tokens, conf_File_Info* config);
    std::string createErrorMsg(const std::string& erro_msg);
    void ensureCorrectArgNumber(const ParserUtils::Strings& tokens, bool badCondition);
    void confirmTryFile(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword);
    void checkServer(const ParserUtils::Strings& pieces);
    void checkLocation(const ParserUtils::Strings& pieces);
    void confirmListenSettings(const ParserUtils::Strings& parameters, conf_File_Info* Keyword);
    void confirmServerName(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword);
    void checkIndex(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword);
    void confirmRootPath(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword);
    void checkAutoindex(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword);
    void verifyErrorPage(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword);
    void confirmCGISettings(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword);
    void confirmCGIExtension(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword);
    void confirmUploadDirectory(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword);
    void confirmUploadToDirectory(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword);
    void confirmRedirect(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword);
    void checkProcedures(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword);
    void ensureClientBodyCapacity(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword);
    void startServerModule();
    void startLocationModule(const std::string& location);
    void endCurrentModule();
};

#endif