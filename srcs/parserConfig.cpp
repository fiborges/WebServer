#include "../includes/parserConfig.hpp"
#include "../includes/conf_info.hpp"
#include "../includes/parser_utils.hpp"

ParserConfig::ParserConfig(Directives* directives, const std::string& location)
    : _directives(directives)
    , _location(location)
{
    if (_directives->index.empty())
    {
        _directives->index = "index.html";
    }
}

ParserConfig::ParserConfig(const ParserConfig& src)
    : _directives(src._directives)
    , _location(src._location)
{
}

ParserConfig::~ParserConfig()
{
}

ParserConfig& ParserConfig::operator=(const ParserConfig& rhs)
{
    if (this != &rhs)
    {
        _directives = rhs._directives;
        _location = rhs._location;
    }
    return *this;
}

int ParserConfig::getPort() const
{
    return _directives->listen;
}

const std::string& ParserConfig::getServerName() const
{
    return _directives->server_name;
}

const std::string& ParserConfig::getIndex() const
{
    return _directives->index;
}

const std::string ParserConfig::getRoot() const
{
    if (_directives->root.empty())
    {
        return "./";
    }
    return _directives->root + "/";
}

bool ParserConfig::hasAutoIndex() const
{
    return _directives->autoindex;
}

bool ParserConfig::hasErrorPage(int error) const
{
    return _directives->error_page.count(error);
}

const std::string& ParserConfig::getErrorPage(int error) const
{
    return _directives->error_page.at(error);
}

/*bool ConfigSpec::hasCGI() const
{
    return !_directives->cgi.empty();
}*/

/*const std::string& ConfigSpec::getCGI() const
{
    printf("1-ConfigSpec::getCGI() called\n");
    printf("2-CGI dentro da funcao getCGI: %s\n", _directives->cgi.c_str());
    return _directives->cgi;
}*/

bool ParserConfig::hasRedirect() const
{
    return _directives->redirect.code && !_directives->redirect.url.empty();
}

const Redirect& ParserConfig::getRedirect() const
{
    return _directives->redirect;
}

std::string ParserConfig::match(const std::string& path) const
{
    Locations::iterator it = _directives->locations.begin();
    for (; it != _directives->locations.end(); ++it)
    {
        if (path.find(it->first) != std::string::npos)
        {
            return it->first;
        }
    }
    return _empty;
}

ParserConfig ParserConfig::getContext(const std::string& path) const
{
    Directives* ctx = &_directives->locations.at(path);
    ctx->listen = _directives->listen;
    ctx->server_name = _directives->server_name;
    if (ctx->root.empty())
    {
        ctx->root = _directives->root;
    }
    if (ctx->index.empty())
    {
        ctx->index = _directives->index;
    }
    return ParserConfig(ctx, path);
}

std::string ParserConfig::getLocation() const
{
    return _location;
}

int ParserConfig::getClientBodySize() const
{
    return _directives->client_max_body_size;
}

const std::string& ParserConfig::getUploadDir() const
{
    return _directives->upload_dir;
}

bool ParserConfig::isMethodAllowed(const std::string& method) const
{
    return _directives->limit_except.empty()
           || _directives->limit_except.count(ParserUtils::toLower(method));
}