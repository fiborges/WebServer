#include "../includes/librarie.hpp"
#include "../includes/conf_info.hpp"
#include "../includes/parser.hpp"
#include "../includes/parser_utils.hpp"
#include "../includes/erros.hpp"
#include "../includes/parserConfig.hpp"

ParserClass::ParserClass(const std::string& file_path)
    : configFilePath(file_path), configurationFile(configFilePath.c_str()),
      conf_info(), serverConfigurations(), validationMapKeys(), contextHistory(),
      conFileInProgress(NULL), currentState("Out"), numberOfModules(0), lineTracker(0) {
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
        if (pieces[0] == "autoindex") {
            conFileInProgress->autoindexPresent = true;
        }
    }
    ensureAllModulesClosed();
    validateRequiredParameters();
}

void ParserClass::validateRequiredParameters() {
    for (std::vector<conf_File_Info>::iterator configIterator = conf_info.begin(); configIterator != conf_info.end(); ++configIterator) {
        if (configIterator->portListen == 0) {
            throw ConfigError("Error: Missing 'listen' directive in a server block. Every server block must include a 'listen' directive to specify the port number.");
        }
        if (configIterator->ServerName.empty()) {
            throw ConfigError("Error: Missing 'server_name' directive in a server block. You must define 'server_name' to identify the server within the network.");
        }
        if (configIterator->RootDirectory.empty()) {
            throw ConfigError("Error: Missing 'root' directive in a server block. Every server block must include a 'root' directive to specify the root directory.");
        }
        for (std::map<std::string, conf_File_Info>::iterator locIter = configIterator->LocationsMap.begin(); locIter != configIterator->LocationsMap.end(); ++locIter) {
            if (locIter->second.redirectURL.httpStatusCode != 0) {
                configIterator->redirectURL = locIter->second.redirectURL;
            }
            if (!locIter->second.Path_CGI.empty()) {
                configIterator->Path_CGI = locIter->second.Path_CGI;
            }
        }
    }
}

bool ParserClass::lineIsIgnorable(const std::string& line) const
{
    return line.empty() || line[0] == '#' || (line.size() >= 2 && line[0] == '/' && line[1] == '/');
}

void ParserClass::locateServerModule(const ParserUtils::Strings& pieces)
{
    if (pieces[0] == "server") {
        checkServer(pieces);
        conf_info.push_back(conf_File_Info());
        conFileInProgress = &conf_info.back();
        currentState = "In";
        numberOfModules++;
    } else {
        throw ConfigError(createErrorMsg("Error: Each configuration file must begin with a 'server' block. Please check your configuration file for errors."));
    }
}

void ParserClass::handleServerModule(const ParserUtils::Strings& pieces)
{
    if (pieces[0] == "location") {
        checkLocation(pieces);
        currentState = "In_Location";
        locationPrefix = ParserUtils::normalizePath(pieces[1]);
        conFileInProgress->LocationsMap[locationPrefix] = conf_File_Info();
    } else if (validationMapKeys.count(pieces[0])) {
        confFileHandler handler = validationMapKeys.at(pieces[0]);
        (this->*handler)(pieces, conFileInProgress);
    } else if (pieces[0] == "}") {
        currentState = "Out";
        numberOfModules--;
    } else {
        throw ConfigError(createErrorMsg("Error: Unknown directive '" + pieces[0] + "' encountered within a server block. This directive is either misspelled or not allowed in this context. Please check your configuration file for errors."));
    }
}

void ParserClass::checkLocation(const ParserUtils::Strings& pieces) {
    if (pieces.size() < 2 || pieces[pieces.size() - 1] != "{") {
        throw ConfigError(createErrorMsg("Configuration Syntax Error: Each 'location' directive must be followed by a path and then an opening '{'. "
            "Please check your configuration file and ensure proper syntax."));
    }

    std::string location = pieces[1];

    locationPrefix = ParserUtils::normalizePath(location);

    if (locationPrefix.empty()) {
        locationPrefix = "/";
    }

    if (conFileInProgress->LocationsMap.find(locationPrefix) != conFileInProgress->LocationsMap.end()) {
        throw ConfigError(createErrorMsg("Duplicate location detected: " + locationPrefix));
    } else {
        conFileInProgress->LocationsMap[locationPrefix] = conf_File_Info();
    }
}

void ParserClass::parseLocationModule(const ParserUtils::Strings& pieces) {
    if (pieces[0] == "}") {
        currentState = "In";
    } else if (pieces[0] == "listen" || pieces[0] == "server_name") {
        throw ConfigError(createErrorMsg("Error: Invalid directive '" + pieces[0] + "' within a location block. 'listen' and 'server_name' directives should be placed at the server block level, not within location blocks."));
    } else if (validationMapKeys.count(pieces[0])) {
        std::string locationPath = locationPrefix;
        if (locationPath.empty()) {
            locationPath = "/";
        }
        confFileHandler handler = validationMapKeys.at(pieces[0]);

        if (locationPath.find('*') != std::string::npos) {
            for (std::map<std::string, conf_File_Info>::iterator it = conFileInProgress->LocationsMap.begin();
                 it != conFileInProgress->LocationsMap.end(); ++it) {
                if (matchWildcard(locationPath, it->first)) {
                    std::cout << "Wildcard handler match for: " << it->first << std::endl;
                    (this->*handler)(pieces, &it->second);
                }
            }
        } else {
            (this->*handler)(pieces, &conFileInProgress->LocationsMap[locationPath]);
        }
    } else {
        throw ConfigError(createErrorMsg("Error: Unknown directive '" + pieces[0] + "' encountered within a location block. This directive is either misspelled or not allowed in this context. Please check your configuration file for errors and consult the documentation for a list of valid directives within location blocks."));
    }
}

inline void ParserClass::startLocationModule(const std::string& location) {
    std::string locationPath = ParserUtils::normalizePath(location);
    if (locationPath.empty()) {
        locationPath = "/";
    }

    if (conFileInProgress->LocationsMap.find(locationPath) != conFileInProgress->LocationsMap.end()) {
        throw ConfigError(createErrorMsg("Duplicate location detected: " + locationPath));
    } else {
        conFileInProgress->LocationsMap[locationPath] = conf_File_Info();
    }

    contextHistory.push(&conFileInProgress->LocationsMap[locationPath]);
    conFileInProgress = contextHistory.top();
}



void ParserClass::ensureAllModulesClosed()
{
    if (numberOfModules == 0) return;
    throw ConfigError(createErrorMsg("Configuration Error: Unexpected end of file. Make sure all blocks are properly closed with a closing curly brace '}' and there are no unmatched opening braces '{'."));
}

void ParserClass::confirmTryFile(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword) {
    ensureCorrectArgNumber(commandParts, commandParts.size() != 2);
    Keyword->tryFile = commandParts[1];
}

void ParserClass::checkAndConfirmValidMap() {
    validationMapKeys["listen"] = &ParserClass::confirmListenSettings;
    validationMapKeys["server_name"] = &ParserClass::confirmServerName;
    validationMapKeys["index"] = &ParserClass::checkIndex;
    validationMapKeys["root"] = &ParserClass::confirmRootPath;
    validationMapKeys["autoindex"] = &ParserClass::checkAutoindex;
    validationMapKeys["error_page"] = &ParserClass::verifyErrorPage;
    validationMapKeys["redirect"] = &ParserClass::confirmRedirect;
    validationMapKeys["limit_except"] = &ParserClass::checkProcedures;
    validationMapKeys["client_body_size"] = &ParserClass::ensureClientBodyCapacity;
    validationMapKeys["cgi_pass"] = &ParserClass::confirmCGISettings;
    validationMapKeys["cgi_ext"] = &ParserClass::confirmCGIExtension;
    validationMapKeys["upload_dir"] = &ParserClass::confirmUploadDirectory;
    validationMapKeys["upload_to"] = &ParserClass::confirmUploadToDirectory;
    validationMapKeys["host"] = &ParserClass::handleHost;
    validationMapKeys["try_file"] = &ParserClass::confirmTryFile;
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
        throw ConfigError(createErrorMsg("Error: The 'server' directive should be followed by an opening '{'. Please check your configuration file and ensure proper syntax."));
    }
}

void ParserClass::confirmListenSettings(const ParserUtils::Strings& parameters, conf_File_Info* Keyword)
{
    ensureCorrectArgNumber(parameters, parameters.size() != 2);
    int portNumber = std::atoi(parameters[1].c_str());
    if (portNumber < 1025 || portNumber > 9999 || portNumber == 3306){
        throw ConfigError(createErrorMsg("Error: Invalid port number in 'listen' directive. Port numbers must be between 1025 and 9999, excluding 3306."));
    }
    Keyword->portListen = portNumber;
}

void ParserClass::confirmServerName(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword)
{
    ensureCorrectArgNumber(commandParts, commandParts.size() != 2);
    Keyword->ServerName = commandParts[1];
}

void ParserClass::checkIndex(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword)
{
    ensureCorrectArgNumber(commandParts, commandParts.size() != 2);
    Keyword->defaultFile = commandParts[1];
}

void ParserClass::confirmRootPath(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword)
{
    ensureCorrectArgNumber(commandParts, commandParts.size() != 2);
    Keyword->RootDirectory = commandParts[1];
}

void ParserClass::checkAutoindex(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword)
{
    ensureCorrectArgNumber(commandParts, commandParts.size() != 2);
    if (commandParts[1] != "on" && commandParts[1] != "off"){
        throw ConfigError(createErrorMsg("Configuration Error: The 'autoindex' value '" + commandParts[1] + "' is invalid. Only 'on' or 'off' are accepted values. Please adjust your 'autoindex' setting to use one of these valid options."));
    }
    Keyword->directoryListingEnabled = (commandParts[1] == "on") ? true : false;
    Keyword->autoindexPresent = true;
}

void ParserClass::verifyErrorPage(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword)
{
    ensureCorrectArgNumber(commandParts, commandParts.size() < 3);
    std::string errorPageUrl = commandParts[commandParts.size() - 1];
    for (size_t errorIndex = 1; errorIndex < commandParts.size() - 1; ++errorIndex){
        int errorCode = std::atoi(commandParts[errorIndex].c_str());
        if (errorCode < 300 || errorCode > 599){
            throw ConfigError(createErrorMsg("Configuration Error: The specified HTTP status code '" + commandParts[errorIndex] + "' is invalid. Valid error codes must be between 300 and 599."));
        }
        Keyword->errorMap[errorCode] = errorPageUrl;
    }
}

void ParserClass::confirmCGISettings(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword) {
    ensureCorrectArgNumber(commandParts, commandParts.size() != 2);
    Keyword->Path_CGI = commandParts[1];
}

void ParserClass::confirmCGIExtension(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword) {
    ensureCorrectArgNumber(commandParts, commandParts.size() != 2);
    Keyword->cgiExtension = commandParts[1];
}

void ParserClass::confirmRedirect(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword)
{
    ensureCorrectArgNumber(commandParts, commandParts.size() != 3);
    int redirectCode = std::atoi(commandParts[1].c_str());
    if (redirectCode < 300 || redirectCode > 399) {
        throw ConfigError(createErrorMsg("Error: Invalid redirection status code. Must be between 300 and 399."));
    }
    Keyword->redirectURL.httpStatusCode = redirectCode;
    Keyword->redirectURL.destinationURL = commandParts[2];
}

void ParserClass::checkProcedures(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword) {
    ensureCorrectArgNumber(commandParts, commandParts.size() < 2);

    static std::set<std::string> permittedHTTPMethods;
    permittedHTTPMethods.insert("GET");
    permittedHTTPMethods.insert("POST");
    permittedHTTPMethods.insert("DELETE");

    Keyword->allowedMethods.clear();
    for (size_t cmd_Index = 1; cmd_Index < commandParts.size(); ++cmd_Index) {
        std::string currentMethod = commandParts[cmd_Index];
        std::transform(currentMethod.begin(), currentMethod.end(), currentMethod.begin(), ::toupper);
        if (!permittedHTTPMethods.count(currentMethod)) {
            throw ConfigError(createErrorMsg("Configuration Error: Invalid HTTP method '" + commandParts[cmd_Index] + "'. Valid methods are GET, POST, and DELETE."));
        }
        Keyword->allowedMethods.insert(currentMethod);
    }
}

void ParserClass::ensureClientBodyCapacity(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword) {
    if (commandParts.size() != 2) {
        throw ConfigError(createErrorMsg(RED "Configuration Error: 'client_body_size' requires exactly one argument." RESET));
    }

    std::string sizeStr = commandParts[1];
    long int bodySize;
    char unit = sizeStr[sizeStr.size() - 1];
    
    if (std::isdigit(unit)) {
        try {
            bodySize = atol(sizeStr.c_str());
        } catch (const std::exception&) {
            throw ConfigError(createErrorMsg(RED "Invalid value for 'client_body_size': " + commandParts[1] + ". Please provide a valid value." RESET));
        }
    } else {
        sizeStr.erase(sizeStr.size() - 1);
        try {
            bodySize = atol(sizeStr.c_str());
        } catch (const std::exception&) {
            throw ConfigError(createErrorMsg(RED "Invalid value for 'client_body_size': " + commandParts[1] + ". Please provide a valid value." RESET));
        }

        switch (unit) {
            case 'k':
            case 'K':
                bodySize *= 1024;
                break;
            case 'm':
            case 'M':
                bodySize *= 1024 * 1024;
                break;
            case 'g':
            case 'G':
                bodySize *= 1024 * 1024 * 1024;
                break;
            default:
                throw ConfigError(createErrorMsg(RED "Invalid unit for 'client_body_size': " + std::string(1, unit) + ". Use 'k', 'm', or 'g'." RESET));
        }
    }
    
    if (bodySize < 0) {
        throw ConfigError(createErrorMsg(RED "Invalid value for 'client_body_size': " + commandParts[1] + ". Size must be positive." RESET));
    }

    Keyword->maxRequestSize = bodySize;
}

void ParserClass::confirmUploadDirectory(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword) {
    ensureCorrectArgNumber(commandParts, commandParts.size() != 2);
    Keyword->fileUploadDirectory = commandParts[1];
}

void ParserClass::confirmUploadToDirectory(const ParserUtils::Strings& commandParts, conf_File_Info* Keyword) {
    ensureCorrectArgNumber(commandParts, commandParts.size() != 2);
    Keyword->uploadToDirectory = commandParts[1];
}

inline void ParserClass::ensureCorrectArgNumber(const ParserUtils::Strings& tokens, bool badCondition)
{
    if (badCondition)
    {
        throw ConfigError(createErrorMsg("Invalid number of arguments for directive '" + tokens[0] + "'. Please ensure the correct number of arguments is provided."));
    }
}

void ParserClass::startServerModule()
{
    conf_info.push_back(conf_File_Info());
    contextHistory.push(&conf_info.back());
    conFileInProgress = &conf_info.back();
}

void ParserClass::endCurrentModule()
{
    contextHistory.pop();
    conFileInProgress = contextHistory.top();
}

void ParserClass::handleHost(const ParserUtils::Strings& tokens, conf_File_Info* config) {
    if (tokens.size() != 2) {
        throw std::runtime_error("Invalid host configuration");
    }
    config->host = tokens[1];
}

ParserClass::ConfigError::ConfigError(const std::string& err)
    : std::runtime_error(err)
{
}