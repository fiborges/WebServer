# ifndef PARSER_UTILS_HPP
#define PARSER_UTILS_HPP

#include "librarie.hpp"
#include <cctype>

class ParserUtils {
public:
    typedef std::vector<std::string> Strings;

    static Strings split(const std::string& input, const std::string& delimiter);
    static std::string split(const std::string& input, const std::string& delimiter, int idx);
    static std::string toLower(const std::string& input);
    static std::string& trim(std::string& s);
    static bool startsWith(const std::string& input, const std::string& prefix);
    static std::string itoa(int n);
    static Strings strip(const Strings& content);
    static void replace(std::string& text, const std::string& search, const std::string& replace);
    static std::string strClean(const std::string& str, char c);
    
private:
    static std::string& ltrim(std::string& s);
    static std::string& rtrim(std::string& s);
};

#endif

