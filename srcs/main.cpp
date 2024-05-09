#include "../includes/parser.hpp"
#include "../includes/erros.hpp"

void ParserClass::debug() const {
    std::cout << "Debugging ConfigParser State:" << std::endl;
    std::cout << "File being parsed: " << configFilePath << std::endl;
    std::cout << "Total directives parsed: " << conf_info.size() << std::endl;

    int blockCount = 1;
    for (std::vector<conf_File_Info>::const_iterator dir_it = conf_info.begin(); dir_it != conf_info.end(); ++dir_it) {
        const conf_File_Info& directive = *dir_it;
        std::cout << "Server Block " << blockCount++ << ":" << std::endl;
        std::cout << "\tListen: " << directive.portListen << std::endl;
        std::cout << "\tServer Name: " << directive.ServerName << std::endl;
        std::cout << "\tRoot: " << directive.RootDirectory << std::endl;
        std::cout << "\tIndex File: " << directive.defaultFile << std::endl;
        std::cout << "\tAutoindex: " << (directive.directoryListingEnabled ? "Enabled" : "Disabled") << std::endl;
        std::cout << "\tClient Max Body Size: " << directive.maxRequestSize << " bytes" << std::endl;
        std::cout << "\tUpload Directory: " << directive.fileUploadDirectory << std::endl;
        
        if (directive.redirectURL.httpStatusCode) {
            std::cout << "\tRedirect Code: " << directive.redirectURL.httpStatusCode << " to " << directive.redirectURL.destinationURL << std::endl;
        }

        std::cout << "\tAllowed Methods: ";
        for (std::set<std::string>::const_iterator meth_it = directive.allowedMethods.begin(); meth_it != directive.allowedMethods.end(); ++meth_it) {
            std::cout << *meth_it << " ";
        }
        std::cout << std::endl;

        std::cout << "\tError Pages:" << std::endl;
        for (std::map<int, std::string>::const_iterator page_it = directive.errorMap.begin(); page_it != directive.errorMap.end(); ++page_it) {
            std::cout << "\t\tError Code: " << page_it->first << " - Page: " << page_it->second << std::endl;
        }

        std::cout << "\tLocations Defined: " << directive.LocationsMap.size() << std::endl;
        for (Locations::const_iterator loc_it = directive.LocationsMap.begin(); loc_it != directive.LocationsMap.end(); ++loc_it) {
            std::cout << "\t\tLocation Path: " << loc_it->first << " - Root: " << loc_it->second.RootDirectory << std::endl;
    // Imprimir informações de CGI, se disponíveis
            if (!loc_it->second.Path_CGI.empty()) {
                std::cout << "\t\tCGI Path: " << loc_it->second.Path_CGI << std::endl;
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

        ConfiguredServers configs = parser.fetchSpecifications();
        
        //ParserConfig parserConfig = configs.front(); // or configs.front(), config[0] depending on the container type
        //conf_File_Info* config = parserConfig.getServerConfigurations();
        
        std::vector<ServerInfo> servers;
        for (size_t i = 0; i < configs.size(); ++i) {
            ParserConfig parserConfig = configs[i];
            conf_File_Info* config = parserConfig.getServerConfigurations();
            ServerInfo server;
            setupServer(server, *config);
            servers.push_back(server);
        }



        runServer(servers);

        //std::vector<ServerInfo> servers;
        //setupServer(servers, config);

        // for (size_t i = 0; i < configs.size(); ++i) {
        //     ServerInfo server;
        //     setupServer(server, config[i]);
        //     servers.push_back(server);
        // }

        // for(std::vector<ServerInfo>::iterator it = servers.begin(); it != servers.end(); ++it) {
        //     runServer(*it);
        // }



        //ServerInfo server;
        //setupServer(server, *config);
        //runServer(server);


        std::cout << GREEN << "\nConfig file parsed successfully\n" << RESET;
    }
    catch(const std::exception &e)
    {
        std::cerr << RED << "Error: " << e.what() << RESET << std::endl;
    }
    
    return 0;
}


// int main(int argc, char **argv)
// {
//     ServerErrorHandler::initialize();
//    if (argc != 2)
// 	{
// 	    std::cout << RED << "Error: Incorrect number of parameters provided.\n" << RESET;
//         std::cout << GREEN << "Usage: Please run the program with the correct configuration file as follows:\n" << RESET;
//         std::cout << "./webserv <config_file>\n";
//         std::cout << YELLOW << "Example: ./webserv config.txt\n" << RESET;
// 		return (1);
// 	}
//     try
//     {
//         ParserClass parser(argv[1]);
//         parser.debug();
//         std::cout << GREEN << "\nConfig file parsed successfully\n" << RESET;
//     }
//     catch(const std::exception &e)
//     {
//         std::cerr << RED << "Error: " << e.what() << RESET << std::endl;
//     }
    
//     return 0;
// }