#include "../includes/parserConfig.hpp"
#include "../includes/conf_info.hpp"
#include "../includes/parser_utils.hpp"

ParserConfig::ParserConfig(conf_File_Info* configData, const std::string& path_location)
    : Server_configurations(configData), locationPath(path_location)
{
    if (Server_configurations->defaultFile.empty()){
        Server_configurations->defaultFile = "index.html";
    }
}

ParserConfig::ParserConfig(const ParserConfig& src)
    : Server_configurations(src.Server_configurations), locationPath(src.locationPath)
{
}

ParserConfig::~ParserConfig()
{
}

ParserConfig& ParserConfig::operator=(const ParserConfig& src)
{
    if (this != &src)
    {
        Server_configurations = src.Server_configurations;
        locationPath = src.locationPath;
    }
    return *this;
}

int ParserConfig::obtainPort() const
{
    return Server_configurations->portListen;
}

const std::string& ParserConfig::retrieveServerName() const
{
    return Server_configurations->ServerName;
}

const std::string& ParserConfig::fetchIndex() const
{
    return Server_configurations->defaultFile;
}

const std::string ParserConfig::acquireRoot() const
{
    if (Server_configurations->RootDirectory.empty())
    {
        return "./";
    }
    return Server_configurations->RootDirectory + "/";
}

bool ParserConfig::checkAutoIndex() const
{
    return Server_configurations->directoryListingEnabled;
}

bool ParserConfig::validateErrorPage(int errorNumber) const
{
    return Server_configurations->errorMap.count(errorNumber);
}

const std::string& ParserConfig::fetchErrorPage(int errorNumber) const
{
    return Server_configurations->errorMap.at(errorNumber);
}

bool ParserConfig::confirmCGI() const
{
    return !Server_configurations->Path_CGI.empty();
}

const std::string& ParserConfig::accessCGIScript() const
{
    return Server_configurations->Path_CGI;
}

bool ParserConfig::verifyRedirection() const
{
    return Server_configurations->redirectURL.httpStatusCode && !Server_configurations->redirectURL.destinationURL.empty();
}

const ForwardingURL& ParserConfig::fetchRedirection() const
{
    return Server_configurations->redirectURL;
}

/*std::string ParserConfig::matchPath(const std::string& searchPath) const
{
    std::string matchedPath = "";
    size_t maxMatchLength = 0;
    
    Locations::const_iterator locationIterator = Server_configurations->LocationsMap.begin();
    for (; locationIterator != Server_configurations->LocationsMap.end(); ++locationIterator){
        if (searchPath.find(locationIterator->first) == 0 && locationIterator->first.length() > maxMatchLength){
            matchedPath = locationIterator->first;
            maxMatchLength = locationIterator->first.length();
        }
    }
    return matchedPath;
}*/

std::string ParserConfig::matchPath(const std::string& searchPath) const {
    // Verificar correspondência exata primeiro
    for (Locations::const_iterator it = Server_configurations->ExactLocationsMap.begin(); it != Server_configurations->ExactLocationsMap.end(); ++it) {
        if (searchPath == it->first) {
            return it->first;
        }
    }

    // Verificar correspondência de prefixo
    std::string matchedPath = "/";
    size_t maxLength = 0;

    for (Locations::const_iterator it = Server_configurations->LocationsMap.begin(); it != Server_configurations->LocationsMap.end(); ++it) {
        if (searchPath.find(it->first) == 0 && it->first.length() > maxLength) {
            maxLength = it->first.length();
            matchedPath = it->first;
        }
    }

    if (matchedPath == "/") {
        std::cout << YELLOW << "No specific path matched for " << searchPath << ". Returning root (/) as default." << RESET << std::endl;
    }

    return matchedPath;
}

ParserConfig ParserConfig::extractContext(const std::string& requestedPath) const {
    conf_File_Info* environmentInfo;

    if (Server_configurations->ExactLocationsMap.count(requestedPath)) {
        environmentInfo = &Server_configurations->ExactLocationsMap.at(requestedPath);
    } else if (Server_configurations->LocationsMap.count(requestedPath)) {
        environmentInfo = &Server_configurations->LocationsMap.at(requestedPath);
    } else if (requestedPath == "/") {
        // Criar uma configuração padrão para a raiz se não estiver explicitamente definida
        static conf_File_Info defaultRootConfig;
        defaultRootConfig.portListen = Server_configurations->portListen;
        defaultRootConfig.ServerName = Server_configurations->ServerName;
        defaultRootConfig.RootDirectory = Server_configurations->RootDirectory;
        defaultRootConfig.defaultFile = Server_configurations->defaultFile;
        environmentInfo = &defaultRootConfig;
    } else {
        throw std::runtime_error("Path not found in configuration: " + requestedPath);
    }

    environmentInfo->portListen = Server_configurations->portListen;
    environmentInfo->ServerName = Server_configurations->ServerName;
    
    if (environmentInfo->RootDirectory.empty()) {
        environmentInfo->RootDirectory = Server_configurations->RootDirectory;
    }
    if (environmentInfo->defaultFile.empty()) {
        environmentInfo->defaultFile = Server_configurations->defaultFile;
    }

    return ParserConfig(environmentInfo, requestedPath);
}



/*ParserConfig ParserConfig::extractContext(const std::string& requestedPath) const
{
    std::string matchedPath = matchPath(requestedPath);
    conf_File_Info* environmentInfo = &Server_configurations->LocationsMap.at(matchedPath);
    
    environmentInfo->portListen = Server_configurations->portListen;
    environmentInfo->ServerName = Server_configurations->ServerName;
    
    if (environmentInfo->RootDirectory.empty()){
        environmentInfo->RootDirectory = Server_configurations->RootDirectory;
    }
    if (environmentInfo->defaultFile.empty()){
        environmentInfo->defaultFile = Server_configurations->defaultFile;
    }
    return ParserConfig(environmentInfo, matchedPath);
}*/

std::string ParserConfig::determineLocation() const
{
    return locationPath;
}

int ParserConfig::calculateClientBodySize() const
{
    return Server_configurations->maxRequestSize;
}

const std::string& ParserConfig::obtainUploadDirectory() const
{
    return Server_configurations->fileUploadDirectory;
}

bool ParserConfig::validateMethod(const std::string& httpMethod) const
{
    return Server_configurations->allowedMethods.empty()
           || Server_configurations->allowedMethods.count(ParserUtils::toLower(httpMethod));
}

// Getter for Server_configurations
const conf_File_Info* ParserConfig::getServerConfigurations() const {
    return Server_configurations;
}