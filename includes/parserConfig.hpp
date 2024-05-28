#ifndef PARSERCONFIG_HPP
#define PARSERCONFIG_HPP

#include "librarie.hpp"
#include "conf_info.hpp"

class ParserConfig {
public:
    ParserConfig(conf_File_Info* configData, const std::string& path_location = "");
    ParserConfig(const ParserConfig& src);
    ~ParserConfig();

    ParserConfig& operator=(const ParserConfig& src);

    int obtainPort() const;
    const std::string& retrieveServerName() const;
    const std::string& fetchIndex() const;
    const std::string acquireRoot() const;
    bool checkAutoIndex() const;
    bool validateErrorPage(int errorNumber) const;
    const std::string& fetchErrorPage(int errorNumber) const;
    bool confirmCGI() const;
    const std::string& accessCGIScript() const;
    bool verifyRedirection() const;
    const ForwardingURL& fetchRedirection() const;
    std::string matchPath(const std::string& searchPath) const;
    ParserConfig extractContext(const std::string& requestedPath) const;
    std::string determineLocation() const;
    int calculateClientBodySize() const;
    const std::string& obtainUploadDirectory() const;
    bool validateMethod(const std::string& httpMethod) const;

    // Add getter for Server_configurations
    const conf_File_Info* getServerConfigurations() const;

private:
    conf_File_Info* Server_configurations;
    std::string locationPath;
};

#endif