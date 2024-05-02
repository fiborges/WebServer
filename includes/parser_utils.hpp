# ifndef PARSER_UTILS_HPP
#define PARSER_UTILS_HPP

#include "librarie.hpp"
#include <cctype>

class ParserUtils {
public:
    typedef std::vector<std::string> Strings;

    //Divide uma string em substrings com base em um delimitador e retorna um vetor de strings contendo as partes divididas
    static Strings split(const std::string& input, const std::string& delimiter);
    //(sobrecarga): Retorna uma substring específica de uma string dividida com base em um delimitador e um índice fornecido
    static std::string getSplitSubstring(const std::string& input, const std::string& delimiter, int idx);
    //Converte uma string para minúsculas
    static std::string toLower(const std::string& input);
    //Remove os espaços em branco à esquerda e à direita de uma string
    static std::string& trim(std::string& s);
    //Verifica se uma string começa com um determinado prefixo
    static bool startsWith(const std::string& input, const std::string& prefix);
    //Converte um número inteiro em uma string
    static std::string itoa(int n);
    //Remove os espaços em branco à esquerda e à direita de cada string em um vetor de strings
    static Strings strip(const Strings& content);
    // Substitui todas as ocorrências de uma substring por outra em uma string
    static void replace(std::string& text, const std::string& search, const std::string& replace);
    //Remove todas as instâncias de um caractere específico de uma string
    static std::string removeCharacter(const std::string& str, char c);
    
private:
    //Remove os espaços em branco à esquerda de uma string
    static std::string& trim_Left(std::string& s);
    //Remove os espaços em branco à direita de uma string
    static std::string& trim_Rigth(std::string& s);
};

#endif

