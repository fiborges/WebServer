#include "../includes/parser.hpp"

int main(int argc, char **argv)
{
    parser parser;
   if (argc != 2)
	{
	    std::cout << RED << "Error: Incorrect number of parameters provided.\n" << RESET;
        std::cout << GREEN << "Usage: Please run the program with the correct configuration file as follows:\n" << RESET;
        std::cout << "./webserv <config_file>\n";
        std::cout << YELLOW << "Example: ./webserv config.txt\n" << RESET;
		return (1);
	}
    try
    {
        parser.parseFile(argv[1]);
        std::cout << GREEN << "\nConfig file parsed successfully\n" << RESET;
    }
    catch(const std::exception &e)
    {
        std::cerr << RED << "Error: " << e.what() << RESET << std::endl;
    }
    
    return 0;
}