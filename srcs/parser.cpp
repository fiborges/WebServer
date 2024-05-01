#include "../includes/librarie.hpp"
#include "../includes/conf_info.hpp"
#include "../includes/parser.hpp"
#include "../includes/parser_utils.hpp"

ParserClass::ParserClass(const std::string& filename)
    : _filename(filename)
    , _config(_filename.c_str())
    , _state(OutsideServerBlock)
    , _openBlocks(0)
    , _currentDirectives(NULL)
    , _lineNumber(0)
{
    if (!_config)
    {
        throw std::runtime_error(filename + ": " + strerror(errno));
    }

    initializeValidationMap();
    this->parse();
}

ParserClass::~ParserClass()
{
}

const ServerConfig& ParserClass::fetchSpecifications()
{
    if (_specs.empty())
    {
        for (size_t idx = 0; idx < _directives.size(); ++idx)
        {
            _specs.push_back(ParserConfig(&_directives[idx]));
        }
    }
    return _specs;
}

void ParserClass::parse()
{
    std::string line;
    while (std::getline(_config, line))
    {
        _lineNumber++;
        ParserUtils::trim(line);
        if (isCommentOrEmpty(line))
        {
            continue;
        }

        ParserUtils::Strings tokens = ParserUtils::strip(ParserUtils::split(line, " "));
        switch (_state)
        {
        case OutsideServerBlock:
        {
            findNewServerBlock(tokens);
            break;
        }
        case InsideServerBlock:
        {
            parseServerBlock(tokens);
            break;
        }
        case InsideLocationBlock:
        {
            parseLocationBlock(tokens);
            break;
        }
        }
    }
    checkOpenBlocks();
    ensureEssentialDirectivesExist();
}

void ParserClass::ensureEssentialDirectivesExist()
{
    for (std::vector<Directives>::iterator it = _directives.begin(); it != _directives.end(); ++it)
    {
        if (it->listen == 0)
        {
            throw ParseException("Missing 'listen' directive in a server block.");
        }
        if (it->server_name.empty())
        {
            throw ParseException("Missing 'server_name' directive in a server block.");
        }
    }
}

inline bool ParserClass::isCommentOrEmpty(const std::string& line)
{
    return line[0] == '#' || line.empty();
}

inline void ParserClass::findNewServerBlock(ParserUtils::Strings& tokens)
{
    if (tokens[0] == "server")
    {
        validateServer(tokens);
        enterServerContext();
        _state = InsideServerBlock;
        _openBlocks++;
        return;
    }
    throw ParseException(fmtError("expecting 'server', got '" + tokens[0] + "'"));
}

inline void ParserClass::parseServerBlock(const ParserUtils::Strings& tokens)
{
    if (_keywords.count(tokens[0]))
    {
        ValidationMethod assign = _keywords.at(tokens[0]);
        (this->*assign)(tokens, _currentDirectives);
        return;
    }
    if (tokens[0] == "}")
    {
        _state = OutsideServerBlock;
        _openBlocks--;
        return;
    }
    if (tokens[0] == "location")
    {
        validateLocation(tokens);
        enterLocationContext(tokens[1]);
        _state = InsideLocationBlock;
        _openBlocks++;
        return;
    }
    throw ParseException(fmtError("unknown directive '" + tokens[0] + "'"));
}

inline void ParserClass::parseLocationBlock(const ParserUtils::Strings& tokens)
{
    if (tokens[0] == "listen" || tokens[0] == "server_name")
    {
        throw ParseException(fmtError("'" + tokens[0] + "' directive is not allowed here"));
    }
    if (_keywords.count(tokens[0]))
    {
        ValidationMethod assign = _keywords.at(tokens[0]);
        (this->*assign)(tokens, _currentDirectives);
        return;
    }
    if (tokens[0] == "}")
    {
        exitContext();
        _state = InsideServerBlock;
        _openBlocks--;
        return;
    }
    throw ParseException(fmtError("unknown directive '" + tokens[0] + "'"));
}

inline void ParserClass::checkOpenBlocks()
{
    if (_openBlocks == 0)
    {
        return;
    }
    throw ParseException(fmtError("unexpected end of file, expecting '}'"));
}

void ParserClass::initializeValidationMap()
{
    _keywords["listen"] = &ParserClass::validateListen;
    _keywords["server_name"] = &ParserClass::validateServerName;
    _keywords["index"] = &ParserClass::validateIndex;
    _keywords["root"] = &ParserClass::validateRoot;
    _keywords["autoindex"] = &ParserClass::validateAutoindex;
    _keywords["error_page"] = &ParserClass::validateErrorPage;
    _keywords["cgi_pass"] = &ParserClass::validateCGI;
    _keywords["redirect"] = &ParserClass::validateRedirect;
    _keywords["limit_except"] = &ParserClass::validateMethods;
    _keywords["client_body_size"] = &ParserClass::validateClientBodySize;
    _keywords["upload_dir"] = &ParserClass::validateUploadDir;
}

std::string ParserClass::fmtError(const std::string& message)
{
    std::stringstream ss;
    ss << message << " in " << _filename << ":" << _lineNumber;
    return ss.str();
}

inline void ParserClass::validateServer(const ParserUtils::Strings& tokens)
{
    if (tokens.size() != 2 || tokens[1] != "{")
    {
        throw ParseException(fmtError("directive 'server' has no opening '{'"));
    }
}

inline void ParserClass::validateLocation(const ParserUtils::Strings& tokens)
{
    checkArgCount(tokens, tokens[1] == "{");

    if (tokens.size() != 3)
    {
        throw ParseException(fmtError("directive 'location' has no opening '{'"));
    }
}

void ParserClass::validateListen(const ParserUtils::Strings& tokens, Directives* directive)
{
    checkArgCount(tokens, tokens.size() != 2);

    int port = std::atoi(tokens[1].c_str());
    if (port < 3 || port > 65535)
    {
        throw ParseException(fmtError("invalid port number '" + tokens[1] + "'"));
    }
    directive->listen = port;
}

void ParserClass::validateServerName(const ParserUtils::Strings& tokens, Directives* directive)
{
    checkArgCount(tokens, tokens.size() != 2);
    directive->server_name = tokens[1];
}

void ParserClass::validateIndex(const ParserUtils::Strings& tokens, Directives* directive)
{
    checkArgCount(tokens, tokens.size() != 2);
    directive->index = tokens[1];
}

void ParserClass::validateRoot(const ParserUtils::Strings& tokens, Directives* directive)
{
    checkArgCount(tokens, tokens.size() != 2);
    directive->root = tokens[1];
}

void ParserClass::validateAutoindex(const ParserUtils::Strings& tokens, Directives* directive)
{
    checkArgCount(tokens, tokens.size() != 2);
    if (tokens[1] != "on" && tokens[1] != "off")
    {
        throw ParseException(fmtError(
            "invalid value '" + tokens[1]
            + "' in 'autoindex' directive, it must be 'on' or 'off'"));
    }
    directive->autoindex = (tokens[1] == "on") ? true : false;
}

void ParserClass::validateErrorPage(const ParserUtils::Strings& tokens, Directives* directive)
{
    checkArgCount(tokens, tokens.size() < 3);
    std::string page = tokens[tokens.size() - 1];
    for (size_t idx = 1; idx < tokens.size() - 1; ++idx)
    {
        int error = std::atoi(tokens[idx].c_str());
        if (error < 300 || error > 599)
        {
            throw ParseException(
                fmtError("value '" + tokens[idx] + "' must be between 300 and 599"));
        }
        directive->error_page[error] = page;
    }
}

void ParserClass::validateCGI(const ParserUtils::Strings& tokens, Directives* directive)
{
    checkArgCount(tokens, tokens.size() != 2);
    directive->cgi = tokens[1];
    printf("CGI dentro da funcao validate: %s\n", directive->cgi.c_str());
    printf("CGI Tokens: %s\n", tokens[1].c_str());
}

void ParserClass::validateRedirect(const ParserUtils::Strings& tokens, Directives* directive)
{
    checkArgCount(tokens, tokens.size() != 3);
    int statusCode = std::atoi(tokens[1].c_str());
    directive->redirect.code = statusCode;
    directive->redirect.url = tokens[2];
}

void ParserClass::validateMethods(const ParserUtils::Strings& tokens, Directives* directive)
{
    checkArgCount(tokens, tokens.size() < 2);

    static std::set<std::string> methods;
    methods.insert("get");
    methods.insert("post");
    methods.insert("delete");

    for (size_t idx = 1; idx < tokens.size(); ++idx)
    {
        std::string method = ParserUtils::toLower(tokens[idx]);
        if (!methods.count(method))
        {
            throw ParseException(fmtError("invalid method '" + tokens[idx] + "'"));
        }
        directive->limit_except.insert(method);
    }
}

void ParserClass::validateClientBodySize(const ParserUtils::Strings& tokens, Directives* directive)
{
    checkArgCount(tokens, tokens.size() != 2);

    std::istringstream iss(tokens[1]);
    long int value;
    char extraChar;

    if (!(iss >> value) || value < 0 || iss.get(extraChar))
    {
        throw ParseException(fmtError("invalid value for 'client_body_size': " + tokens[1]));
    }
    directive->client_max_body_size = std::atoi(tokens[1].c_str()) << 20;
}

void ParserClass::validateUploadDir(const ParserUtils::Strings& tokens, Directives* directive)
{
    checkArgCount(tokens, tokens.size() != 2);
    directive->upload_dir = tokens[1];
}

inline void ParserClass::enterServerContext()
{
    _directives.push_back(Directives());
    _contextStack.push(&_directives.back());
    _currentDirectives = &_directives.back();
}

inline void ParserClass::enterLocationContext(const std::string& location)
{
    _currentDirectives->locations[location] = Directives();
    _contextStack.push(&_currentDirectives->locations[location]);
    _currentDirectives = _contextStack.top();
}

inline void ParserClass::exitContext()
{
    _contextStack.pop();
    _currentDirectives = _contextStack.top();
}

void ParserClass::checkArgCount(const ParserUtils::Strings& tokens, bool badCondition)
{
    if (badCondition)
    {
        std::string message = "invalid number of arguments for directive '";
        throw ParseException(fmtError(message + tokens[0] + "'"));
    }
}

ParserClass::ParseException::ParseException(const std::string& err)
    : std::invalid_argument(err)
{
}
