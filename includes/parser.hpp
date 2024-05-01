# ifndef PARSER_HPP
#define PARSER_HPP

#include "librarie.hpp"
#include "parser_utils.hpp"
#include "parserConfig.hpp"
#include "conf_info.hpp"

class ParserClass;
typedef void (ParserClass::*ValidationMethod)(const ParserUtils::Strings&, Directives*);
typedef std::map<std::string, ValidationMethod> ValidationMap;
typedef std::vector<ParserConfig> ServerConfig;

class ParserClass
{
public:
    ParserClass(const std::string& filename);
    ~ParserClass();
    void debug() const;

    const ServerConfig& fetchSpecifications();

private:
    enum State
    {
        OutsideServerBlock,
        InsideServerBlock,
        InsideLocationBlock
    };

    std::string _filename;
    std::ifstream _config;
    ValidationMap _keywords;
    std::vector<Directives> _directives;
    State _state;
    int _openBlocks;
    Directives* _currentDirectives;
    int _lineNumber;
    std::stack<Directives*> _contextStack;
    ServerConfig _specs;

    void parse();
    void ensureEssentialDirectivesExist();
    bool isCommentOrEmpty(const std::string& line);
    void findNewServerBlock(ParserUtils::Strings& tokens);
    void parseServerBlock(const ParserUtils::Strings& tokens);
    void parseLocationBlock(const ParserUtils::Strings& tokens);
    void checkOpenBlocks();
    void initializeValidationMap();
    std::string fmtError(const std::string& message);

    void validateServer(const ParserUtils::Strings& tokens);
    void validateLocation(const ParserUtils::Strings& tokens);
    void validateListen(const ParserUtils::Strings& tokens, Directives* directive);
    void validateServerName(const ParserUtils::Strings& tokens, Directives* directive);
    void validateIndex(const ParserUtils::Strings& tokens, Directives* directive);
    void validateRoot(const ParserUtils::Strings& tokens, Directives* directive);
    void validateAutoindex(const ParserUtils::Strings& tokens, Directives* directive);
    void validateErrorPage(const ParserUtils::Strings& tokens, Directives* directive);
    void validateCGI(const ParserUtils::Strings& tokens, Directives* directive);
    void validateRedirect(const ParserUtils::Strings& tokens, Directives* directive);
    void validateMethods(const ParserUtils::Strings& tokens, Directives* directive);
    void validateClientBodySize(const ParserUtils::Strings& tokens, Directives* directive);
    void validateUploadDir(const ParserUtils::Strings& tokens, Directives* directive);

    void enterServerContext();
    void enterLocationContext(const std::string& location);
    void exitContext();
    void checkArgCount(const ParserUtils::Strings& tokens, bool badCondition);

    class ParseException : public std::invalid_argument
    {
    public:
        ParseException(const std::string& err);
    };
};

#endif