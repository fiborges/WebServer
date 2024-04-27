# ifndef PARSER_HPP
#define PARSER_HPP

#include "librarie.hpp"

class parser
{
    public:
        parser(void);

        parser(parser const &src);

        //contrutor para iniciar o obj utilizar o caminho dado pela string
        explicit parser(std::string const &pathFile);

        ~parser(void);

        parser &operator=(parser const &src);

        void parseFile(char *pathFile);

        bool ensureValidSintaxConfFile(void);

        bool ensureMatchingBrace(void);

    private:
        std::string _pathToTheFile;
        std::string _entireConfig;

};

#endif