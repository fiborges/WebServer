#include "../includes/parser.hpp"

void ParserClass::debug() const {
    std::cout << "Debugging ConfigParser State:" << std::endl;
    std::cout << "File being parsed: " << _filename << std::endl;
    std::cout << "Total directives parsed: " << _directives.size() << std::endl;

    int blockCount = 1;
    for (std::vector<Directives>::const_iterator dir_it = _directives.begin(); dir_it != _directives.end(); ++dir_it) {
        const Directives& directive = *dir_it;
        std::cout << "Server Block " << blockCount++ << ":" << std::endl;
        std::cout << "\tListen: " << directive.listen << std::endl;
        std::cout << "\tServer Name: " << directive.server_name << std::endl;
        std::cout << "\tRoot: " << directive.root << std::endl;
        std::cout << "\tIndex File: " << directive.index << std::endl;
        std::cout << "\tAutoindex: " << (directive.autoindex ? "Enabled" : "Disabled") << std::endl;
        std::cout << "\tClient Max Body Size: " << directive.client_max_body_size << " bytes" << std::endl;
        std::cout << "\tUpload Directory: " << directive.upload_dir << std::endl;
        std::cout << "\tCGI Path: " << directive.cgi << std::endl;
        
        if (directive.redirect.code) {
            std::cout << "\tRedirect Code: " << directive.redirect.code << " to " << directive.redirect.url << std::endl;
        }

        std::cout << "\tAllowed Methods: ";
        for (std::set<std::string>::const_iterator meth_it = directive.limit_except.begin(); meth_it != directive.limit_except.end(); ++meth_it) {
            std::cout << *meth_it << " ";
        }
        std::cout << std::endl;

        std::cout << "\tError Pages:" << std::endl;
        for (std::map<int, std::string>::const_iterator page_it = directive.error_page.begin(); page_it != directive.error_page.end(); ++page_it) {
            std::cout << "\t\tError Code: " << page_it->first << " - Page: " << page_it->second << std::endl;
        }

        std::cout << "\tLocations Defined: " << directive.locations.size() << std::endl;
        for (Locations::const_iterator loc_it = directive.locations.begin(); loc_it != directive.locations.end(); ++loc_it) {
            std::cout << "\t\tLocation Path: " << loc_it->first << " - Root: " << loc_it->second.root << std::endl;
    // Imprimir informações de CGI, se disponíveis
            if (!loc_it->second.cgi.empty()) {
                std::cout << "\t\tCGI Path: " << loc_it->second.cgi << std::endl;
            } else {
                std::cout << "\t\tCGI Path: Not Defined" << std::endl;
            }
        }       
    }
}

int main(int argc, char **argv)
{
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
        ParserClass parser(argv[1]);
        parser.debug();
        std::cout << GREEN << "\nConfig file parsed successfully\n" << RESET;
    }
    catch(const std::exception &e)
    {
        std::cerr << RED << "Error: " << e.what() << RESET << std::endl;
    }
    
    return 0;
}