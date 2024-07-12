#include "../includes/parser_utils.hpp"
#include <cctype>

ParserUtils::Strings ParserUtils::split(const std::string& input, const std::string& delimiter) {
    Strings tokens;
    size_t start = 0;
    size_t end = input.find(delimiter);
    while (end != std::string::npos) {
        tokens.push_back(input.substr(start, end - start));
        start = end + delimiter.length();
        end = input.find(delimiter, start);
    }
    tokens.push_back(input.substr(start));
    return tokens;
}

std::string ParserUtils::getSplitSubstring(const std::string& input, const std::string& delimiter, int idx) {
    Strings tokens = split(input, delimiter);
    if (idx >= 0 && idx < static_cast<int>(tokens.size())) {
        return tokens[idx];
    } else 
    {
        return "";
    }
}

std::string ParserUtils::toLower(const std::string& input) {
    std::string s = input;
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

bool ParserUtils::startsWith(const std::string& input, const std::string& prefix) {
    return input.compare(0, prefix.length(), prefix) == 0;
}

std::string& ParserUtils::trim(std::string& s) {
    return trim_Left(trim_Rigth(s));
}

std::string ParserUtils::itoa(int n) {
    std::stringstream s;
    s << n;
    return s.str();
}

ParserUtils::Strings ParserUtils::strip(const Strings& content) {
    Strings result;
    for (Strings::const_iterator it = content.begin(); it != content.end(); ++it) {
        if (!it->empty()) {
            result.push_back(*it);
        }
    }
    return result;
}

void ParserUtils::replace(std::string& text, const std::string& search, const std::string& replace) {
    size_t position = 0;
    while ((position = text.find(search, position)) != std::string::npos) {
        text.erase(position, search.length());
        text.insert(position, replace);
        position += replace.length();
    }
}

std::string ParserUtils::removeCharacter(const std::string& str, char c) {
    std::string result;
    char lastChar = '\0';
    for (size_t idx = 0; idx < str.length(); ++idx) {
        char currentChar = str[idx];
        if (currentChar != c || lastChar != currentChar) {
            result += currentChar;
        }
        lastChar = currentChar;
    }
    return result;
}

std::string& ParserUtils::trim_Left(std::string& s) {
    std::string::iterator it;
    for (it = s.begin(); it != s.end(); ++it) {
        if (!std::isspace(*it)) break;
    }
    s.erase(s.begin(), it);
    return s;
}

std::string& ParserUtils::trim_Rigth(std::string& s) {
    std::string::reverse_iterator rit;
    for (rit = s.rbegin(); rit != s.rend(); ++rit) {
        if (!std::isspace(*rit)) break;
    }
    s.erase(rit.base(), s.end());
    return s;
}


std::string ParserUtils::normalizePath(const std::string& path) {
    std::string normalized = path;
     while (!normalized.empty() && normalized[normalized.size() - 1] == '/') {
        normalized.erase(normalized.size() - 1);
    }
    if (!normalized.empty() && normalized[0] != '/') {
        normalized = "/" + normalized;
    }
    return normalized;
    }