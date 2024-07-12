# ifndef PARSER_UTILS_HPP
#define PARSER_UTILS_HPP

#include "librarie.hpp"

class ParserUtils {
public:
    typedef std::vector<std::string> Strings;
    static Strings split(const std::string& input, const std::string& delimiter);
    static std::string getSplitSubstring(const std::string& input, const std::string& delimiter, int idx);
    static std::string toLower(const std::string& input);
    static std::string& trim(std::string& s);
    static bool startsWith(const std::string& input, const std::string& prefix);
    static std::string itoa(int n);
    static Strings strip(const Strings& content);
    static void replace(std::string& text, const std::string& search, const std::string& replace);
    static std::string removeCharacter(const std::string& str, char c);
    static std::string normalizePath(const std::string& path);
    
private:
    static std::string& trim_Left(std::string& s);
    static std::string& trim_Rigth(std::string& s);
};

#endif

