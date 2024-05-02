#include "../includes/librarie.hpp"
#include "../includes/conf_info.hpp"
#include "../includes/parser.hpp"
#include "../includes/parser_utils.hpp"

ParserClass::ParserClass(const std::string& file_path)
    : configFilePath(file_path), configurationFile(configFilePath.c_str()),
      currentState("Out"), numberOfModules(0), conFileInProgress(nullptr), lineTracker(0) {
    if (!configurationFile) {
        throw std::runtime_error(file_path + ": " + strerror(errno));
    }
    checkAndConfirmValidMap();
    readAndProcessConfig();
}

ParserClass::~ParserClass()
{
}

const ConfiguredServers& ParserClass::fetchSpecifications()
{
    if (serverConfigurations.empty())
    {
        for (size_t index = 0; index < conf_info.size(); ++index)
        {
            serverConfigurations.push_back(ParserConfig(&conf_info[index]));
        }
    }
    return serverConfigurations;
}

void ParserClass::readAndProcessConfig() {
    std::string line;
    while (std::getline(configurationFile, line)) {
        lineTracker++;
        ParserUtils::trim(line);
        if (lineIsIgnorable(line)) continue;

        ParserUtils::Strings pieces = ParserUtils::strip(ParserUtils::split(line, " "));
        if (currentState == "Out") {
            locateServerModule(pieces);
        } else if (currentState == "In") {
            handleServerModule(pieces);
        } else if (currentState == "In_Location") {
            parseLocationModule(pieces);
        }
    }
    ensureAllModulesClosed();
    validateRequiredParameters();
}

void ParserClass::validateRequiredParameters()
{
    for (std::vector<conf_File_Info>::iterator configIterator = conf_info.begin(); configIterator != conf_info.end(); ++configIterator)
    {
        if (configIterator->listen == 0)
        {
            throw ConfigError(RED "Error: Missing 'listen' directive in a server block. " 
                "Every server block must include a 'listen' directive to specify the port number." RESET);
        }
        if (configIterator->server_name.empty())
        {
            throw ConfigError(RED "Error: Missing 'server_name' directive in a server block. "
                "You must define 'server_name' to identify the server within the network." RESET);
        }
    }
}

inline bool ParserClass::lineIsIgnorable(const std::string& fileLine)
{
    return fileLine[0] == '#' || fileLine.empty();
}

inline void ParserClass::locateServerModule(ParserUtils::Strings& segments)
{
    if (segments[0] == "server")
    {
        checkServer(segments);
        startServerModule();
        currentState = "In";
        numberOfModules++;
        return;
    }
    throw ConfigError(createErrorMsg(
    RED "Invalid configuration syntax: expecting 'server' keyword to start a server block, "
    "but got '" + segments[0] + "'. Please check the syntax and ensure that each server "
    "block starts with the 'server' keyword." RESET));
}

inline void ParserClass::handleServerModule(const ParserUtils::Strings& pieces)
{
    if (validationMapKeys.count(pieces[0])){
        confFileHandler assign = validationMapKeys.at(pieces[0]);
        (this->*assign)(pieces, conFileInProgress);
        return;
    }
    if (pieces[0] == "}"){
        currentState = "Out";
        numberOfModules--;
        return;
    }
    if (pieces[0] == "location"){
        checkLocation(pieces);
        startLocationModule(pieces[1]);
        currentState = "In_Location";
        numberOfModules++;
        return;
    }
    throw ConfigError(createErrorMsg(
    RED "Error: Unknown directive '" + pieces[0] + "' encountered. This directive is either "
    "misspelled or not allowed in this context. Please check your configuration file for errors "
    "and consult the documentation for a list of valid directives." RESET));
}

inline void ParserClass::parseLocationModule(const ParserUtils::Strings& pieces)
{
    if (pieces[0] == "listen" || pieces[0] == "server_name"){
        throw ConfigError(createErrorMsg(RED "Error: The '" + pieces[0] + "' directive is not allowed within a location block. "
            "Please review your configuration to ensure directives are placed in the correct context." RESET));
    }
    if (validationMapKeys.count(pieces[0])){
        confFileHandler assign = validationMapKeys.at(pieces[0]);
        (this->*assign)(pieces, conFileInProgress);
        return;
    }
    if (pieces[0] == "}") {
        endCurrentModule();
        currentState = "In";
        numberOfModules--;
        return;
    }
    throw ConfigError(createErrorMsg(YELLOW "Warning: Unknown directive '" + pieces[0] + "' encountered. Please check for typos or "
        "consult the documentation for a list of valid directives within a location block." RESET));
}

inline void ParserClass::ensureAllModulesClosed()
{
    if (numberOfModules == 0){
        return;
    }
    throw ConfigError(createErrorMsg(RED "Configuration Error: Unexpected end of file. "
        "This usually indicates a missing '}' for a configuration block. "
        "Please check your configuration to ensure all blocks are properly closed with '}'."RESET));
}

void ParserClass::checkAndConfirmValidMap()
{
    validationMapKeys["listen"] = &ParserClass::confirmListenSettings;
    validationMapKeys["server_name"] = &ParserClass::confirmServerName;
    validationMapKeys["index"] = &ParserClass::checkIndex;
    validationMapKeys["root"] = &ParserClass::confirmRootPath;
    validationMapKeys["autoindex"] = &ParserClass::checkAutoindex;
    validationMapKeys["error_page"] = &ParserClass::verifyErrorPage;
    validationMapKeys["cgi_pass"] = &ParserClass::confirmCGISettings;
    validationMapKeys["redirect"] = &ParserClass::confirmRedirect;
    validationMapKeys["limit_except"] = &ParserClass::checkProcedures;
    validationMapKeys["client_body_size"] = &ParserClass::ensureClientBodyCapacity;
    validationMapKeys["upload_dir"] = &ParserClass::confirmUploadDir;
}

std::string ParserClass::createErrorMsg(const std::string& erro_msg)
{
    std::stringstream ss;
    ss << erro_msg << " in " << configFilePath << ":" << lineTracker;
    return ss.str();
}

inline void ParserClass::checkServer(const ParserUtils::Strings& pieces)
{
    if (pieces.size() != 2 || pieces[1] != "{"){
        throw ConfigError(createErrorMsg(RED "Configuration Syntax Error: The 'server' directive should be followed by an opening '{'. "
            "Please ensure each 'server' block starts with 'server {' to define the beginning of a server configuration block correctly." RESET));
    }
}

inline void ParserClass::checkLocation(const ParserUtils::Strings& pieces)
{
    ensureCorrectArgNumber(pieces, pieces[1] == "{");

    if (pieces.size() != 3)
    {
        throw ConfigError(createErrorMsg(RED "Configuration Syntax Error: Each 'location' directive must be followed by a path and then an opening '{'. "
            "Example: location /path/ { . Ensure your 'location' directive matches this format." RESET));
    }
}

void ParserClass::confirmListenSettings(const ParserUtils::Strings& parameters, conf_File_Info* Keyword)
{
    ensureCorrectArgNumber(parameters, parameters.size() != 2);

    int portNumber = std::atoi(parameters[1].c_str());
    if (portNumber < 3 || portNumber > 65535){
        throw ConfigError(createErrorMsg(RED "Configuration Error: The port number '" + parameters[1] + 
            "' is invalid. Port numbers must be between 3 and 65535. Please specify a valid port number." RESET));
    }
    Keyword->listen = portNumber;
}

void ParserClass::confirmServerName(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword)
{
    ensureCorrectArgNumber(commandParts, commandParts.size() != 2);
    Keyword->server_name = commandParts[1];
}

void ParserClass::checkIndex(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword)
{
    ensureCorrectArgNumber(commandParts, commandParts.size() != 2);
    Keyword->index = commandParts[1];
}

void ParserClass::confirmRootPath(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword)
{
    ensureCorrectArgNumber(commandParts, commandParts.size() != 2);
    Keyword->root = commandParts[1];
}

void ParserClass::checkAutoindex(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword)
{
    ensureCorrectArgNumber(commandParts, commandParts.size() != 2);
    if (commandParts[1] != "on" && commandParts[1] != "off"){
        throw ConfigError(createErrorMsg(RED "Configuration Error: The 'autoindex' value '" + commandParts[1] + 
            "' is invalid. Only 'on' or 'off' are accepted values. Please adjust your 'autoindex' setting to use one of these valid options." RESET));
    }
    Keyword->autoindex = (commandParts[1] == "on") ? true : false;
}

void ParserClass::verifyErrorPage(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword)
{
    ensureCorrectArgNumber(commandParts, commandParts.size() < 3);
    std::string errorPageUrl = commandParts[commandParts.size() - 1];
    for (size_t errorIndex = 1; errorIndex < commandParts.size() - 1; ++errorIndex){
        int errorCode = std::atoi(commandParts[errorIndex].c_str());
        if (errorCode < 300 || errorCode > 599){
            throw ConfigError(
                createErrorMsg(RED "Configuration Error: The specified HTTP status code '" + commandParts[errorIndex] + 
                "' is invalid. Valid error codes must be between 300 and 599." RESET));
        }
        Keyword->error_page[errorCode] = errorPageUrl;
    }
}

void ParserClass::confirmCGISettings(const ParserUtils::Strings& commandParts, conf_File_Info* keyword) {
    ensureCorrectArgNumber(commandParts, commandParts.size() != 2);
    keyword->cgi = commandParts[1];
    printf(GREEN "CGI Configuration Validated: " RESET "%s\n", keyword->cgi.c_str());
    printf(BLUE "CGI Script Path: " RESET "%s\n", commandParts[1].c_str());
}

void ParserClass::confirmRedirect(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword)
{
    ensureCorrectArgNumber(commandParts, commandParts.size() != 3);
    int redirectCode = std::atoi(commandParts[1].c_str());
    Keyword->redirect.code = redirectCode;
    Keyword->redirect.url = commandParts[2];
}

void ParserClass::checkProcedures(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword)
{
    ensureCorrectArgNumber(commandParts, commandParts.size() < 2);

    static std::set<std::string> permittedHTTPMethods;
    permittedHTTPMethods.insert("get");
    permittedHTTPMethods.insert("post");
    permittedHTTPMethods.insert("delete");

    for (size_t cmd_Index = 1; cmd_Index < commandParts.size(); ++cmd_Index){
        std::string currentMethod = ParserUtils::toLower(commandParts[cmd_Index]);
        if (!permittedHTTPMethods.count(currentMethod)){
            throw ConfigError(createErrorMsg(RED "Configuration Error: Invalid HTTP method '" + commandParts[cmd_Index] + "'. Valid methods are GET, POST, and DELETE." RESET));
        }
        Keyword->limit_except.insert(currentMethod);
    }
}

void ParserClass::ensureClientBodyCapacity(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword)
{
    ensureCorrectArgNumber(commandParts, commandParts.size() != 2);

    std::istringstream inputStream(commandParts[1]);
    long int bodySize;
    char extraCharacter;
    if (!(inputStream >> bodySize) || bodySize < 0 || inputStream.get(extraCharacter)){
        throw ConfigError(createErrorMsg(RED "Invalid value for 'client_body_size': " + commandParts[1] + ". Please provide a VALID VALUE." RESET));
    }
    Keyword->client_max_body_size = std::atoi(commandParts[1].c_str()) << 20;
}

void ParserClass::confirmUploadDir(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword)
{
    ensureCorrectArgNumber(commandParts, commandParts.size() != 2);
    Keyword->upload_dir = commandParts[1];
}

inline void ParserClass::startServerModule()
{
    conf_info.push_back(conf_File_Info());
    contextHistory.push(&conf_info.back());
    conFileInProgress = &conf_info.back();
}

inline void ParserClass::startLocationModule(const std::string& location)
{
    conFileInProgress->locations[location] = conf_File_Info();
    contextHistory.push(&conFileInProgress->locations[location]);
    conFileInProgress = contextHistory.top();
}

inline void ParserClass::endCurrentModule()
{
    contextHistory.pop();
    conFileInProgress = contextHistory.top();
}

void ParserClass::ensureCorrectArgNumber(const ParserUtils::Strings& tokens, bool badCondition)
{
    if (badCondition)
    {
        throw ConfigError(createErrorMsg(RED "Invalid number of arguments for directive '" + tokens[0] + "'. " +
        "Please ensure the correct number of arguments is provided." RESET));
    }
}

ParserClass::ConfigError::ConfigError(const std::string& err)
    : std::invalid_argument(err)
{
}
