#include "../includes/parser.hpp"

parser::parser(void)
{
    return;
}

parser::parser(std::string const &pathFile)
{
    this->_pathToTheFile = pathFile;
}

parser::parser(parser const &src)
{
    *this = src;
}

parser::~parser(void)
{
    return;
}

parser &parser::operator=(const parser &src)
{
    if(this != &src)
        *this = src;
    return *this;
}

void parser::parseFile(char *pathFile)
{
    std::ifstream fileStream;
    std::stringstream data_content;

    // open file
    fileStream.open(pathFile);
    if (!fileStream.is_open())
        throw std::runtime_error("Oops! Couldn't swing open the file door: " + std::string(pathFile));

    std::cout << "Nice! The file open successfully!" << std::endl;

    // read the file data_content
    data_content << fileStream.rdbuf();
    this-> _entireConfig = data_content.str();
    fileStream.close();
    std::cout << "All content from the file has been successfully loaded into memory!" << std::endl;


    // check syntax
    if (!ensureMatchingBrace() || !ensureValidSintaxConfFile())
        throw std::runtime_error("Syntax error detected in the input file. Please check for unmatched matchingBrackets or incorrect formatting.\n");

    std::cout << "Syntax check complete: Everything looks OK!";

    //funcao para extrair blocos de informacao
    //funcao para fazer o prser desses blocos

    //DEBUG
    //funcao para fazer debug e imprimitr tudo o que estou a fazer
}

bool parser::ensureMatchingBrace()
{
    std::stack<char> matchingBrackets;
    std::string configSnapshot = this->_entireConfig;

    for (std::string::iterator charIterator = configSnapshot.begin(); charIterator != configSnapshot.end(); ++charIterator)
    {
        char charRead = *charIterator;
        if (charRead == '{')
            matchingBrackets.push(charRead);
        else if (charRead == '}')
        {
            if (matchingBrackets.empty())
                throw std::runtime_error("Unexpected '}' found. This may indicate that there is an extra closing brace in the configuration file without a matching opening brace. Please check the configuration file for syntax errors related to brace pairing.\n");
            matchingBrackets.pop();
        }
    }
    if (!matchingBrackets.empty())
        throw std::runtime_error("Unclosed '{' found in the configuration file. This indicates that a brace opened but was not closed properly. Please review the configuration file to ensure that all braces are correctly paired and closed.\n");
    return true;
}

bool parser::ensureValidSintaxConfFile()
{
    std::string entireConfiguration = this->_entireConfig;

    // Find the start of the first word
    std::size_t firstNonWhitespacePos = entireConfiguration.find_first_not_of(" \f\n\r\t\v");
    if (firstNonWhitespacePos == std::string::npos) 
    {
        std::cerr << "Error: Config file is empty or contains only whitespace." << std::endl;
        return false;
    }

    // Debug: Print the first non-space character found
    std::cout << "First non-space character index: " << firstNonWhitespacePos << std::endl;

    // Look for the end of the first word
    std::size_t firstWordBoundary = entireConfiguration.find_first_of(" \f\n\r\t\v{", firstNonWhitespacePos);
    if (firstWordBoundary == std::string::npos) 
    {
        std::cerr << "Error: Unable to find the end of the first word." << std::endl;
        return false;
    }

    // Debug: Print the first word found
    std::string firstWord = entireConfiguration.substr(firstNonWhitespacePos, firstWordBoundary - firstNonWhitespacePos);
    std::cout << "First word: " << firstWord << std::endl;

    // Transform and check if the first word is "server"
    std::transform(firstWord.begin(), firstWord.end(), firstWord.begin(), ::tolower);
    if (firstWord != "server:") 
    {
        std::cerr << "Error: First word is not 'server:'." << std::endl;
        return false;
    }

    // Find the opening brace after the "server" keyword
    std::size_t openingBrace = entireConfiguration.find_first_not_of(" \f\n\r\t\v", firstWordBoundary);
    if (openingBrace == std::string::npos || entireConfiguration[openingBrace] != '{') 
    {
        std::cerr << "Error: Opening brace not found or misplaced after 'server:'." << std::endl;
        return false;
    }

    return true;
}
