#ifndef PARSERCONFIG_HPP
#define PARSERCONFIG_HPP

#include "librarie.hpp"
#include "conf_info.hpp"

class ParserConfig
{
public:
    ParserConfig(Directives* directives, const std::string& location = "/");
    ParserConfig(const ParserConfig& src);
    ~ParserConfig();
    ParserConfig& operator=(const ParserConfig& rhs);

    int getPort() const;
    const std::string& getServerName() const;
    const std::string& getIndex() const;
    const std::string getRoot() const;
    bool hasAutoIndex() const;
    bool hasErrorPage(int error) const;
    const std::string& getErrorPage(int error) const;
    bool hasCGI() const;
    const std::string& getCGI() const;
    bool hasRedirect() const;
    const Redirect& getRedirect() const;
    std::string match(const std::string& path) const;
    ParserConfig getContext(const std::string& path) const;
    std::string getLocation() const;
    int getClientBodySize() const;
    const std::string& getUploadDir() const;
    bool isMethodAllowed(const std::string& method) const;

private:
    Directives* _directives;
    std::string _location;
    std::string _empty;
};

#endif 