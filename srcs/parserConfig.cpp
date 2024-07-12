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

const std::string& ParserConfig::retrieveHost() const {
    return Server_configurations->host;
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

const std::string& ParserConfig::fetchCGIExtension() const 
{
    return Server_configurations->cgiExtension;
}

bool ParserConfig::verifyRedirection() const
{
    return Server_configurations->redirectURL.httpStatusCode && !Server_configurations->redirectURL.destinationURL.empty();
}

const ForwardingURL& ParserConfig::fetchRedirection() const
{
    return Server_configurations->redirectURL;
}

const std::string& ParserConfig::fetchUploadToDirectory() const
{
    return Server_configurations->uploadToDirectory;
}

std::string ParserConfig::matchPath(const std::string& searchPath) const {
    for (Locations::const_iterator it = Server_configurations->LocationsMap.begin(); it != Server_configurations->LocationsMap.end(); ++it) {
        if (searchPath == it->first) {
            return it->first;
        }
    }

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

    if (Server_configurations->LocationsMap.count(requestedPath)) {
        environmentInfo = &Server_configurations->LocationsMap.at(requestedPath);
    } else if (requestedPath == "/") {
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
    if (environmentInfo->redirectURL.httpStatusCode != 0) {
        Server_configurations->redirectURL = environmentInfo->redirectURL;
    }
    if (!environmentInfo->Path_CGI.empty()) {
        Server_configurations->Path_CGI = environmentInfo->Path_CGI;
    }

    return ParserConfig(environmentInfo, requestedPath);
}

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

conf_File_Info ParserConfig::getServerConfigurations() const {
    return *Server_configurations;
}

bool matchWildcard(const std::string& pattern, const std::string& str) {
    size_t p = 0, s = 0, star = std::string::npos, match = 0;

    while (s < str.size()) {
        if (p < pattern.size() && (pattern[p] == '?' || pattern[p] == str[s])) {
            ++p;
            ++s;
        } else if (p < pattern.size() && pattern[p] == '*') {
            star = p++;
            match = s;
        } else if (star != std::string::npos) {
            p = star + 1;
            s = ++match;
        } else {
            return false;
        }
    }

    while (p < pattern.size() && pattern[p] == '*') {
        ++p;
    }

    return p == pattern.size();
}
