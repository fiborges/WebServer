#include "../includes/parserConfig.hpp"
#include "../includes/conf_info.hpp"
#include "../includes/parser_utils.hpp"

ParserConfig::ParserConfig(conf_File_Info* configData, const std::string& path_location)
    : Server_configurations(configData)
    , locationPath(path_location)
{
    if (Server_configurations->defaultFile.empty()){
        Server_configurations->defaultFile = "index.html";
    }
}

ParserConfig::ParserConfig(const ParserConfig& src)
    : Server_configurations(src.Server_configurations)
    , locationPath(src.locationPath)
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
    //debug msg
    printf("1-ConfigSpec::accessCGIScript() called\n");
    printf("2-CGI dentro da funcao accessCGIScript: %s\n", Server_configurations->Path_CGI.c_str());
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

std::string ParserConfig::matchPath(const std::string& searchPath) const
{
    Locations::iterator locationIterator = Server_configurations->LocationsMap.begin();
    for (; locationIterator != Server_configurations->LocationsMap.end(); ++locationIterator){
        if (searchPath.find(locationIterator->first) != std::string::npos){
            return locationIterator->first;
        }
    }
    return emptyString;
}

ParserConfig ParserConfig::extractContext(const std::string& requestedPath) const
{
    conf_File_Info* environmentInfo = &Server_configurations->LocationsMap.at(requestedPath);
    environmentInfo->portListen = Server_configurations->portListen;
    environmentInfo->ServerName = Server_configurations->ServerName;
    
    if (environmentInfo->RootDirectory.empty()){
        environmentInfo->RootDirectory = Server_configurations->RootDirectory;
    }
    if (environmentInfo->defaultFile.empty()){
        environmentInfo->defaultFile = Server_configurations->defaultFile;
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