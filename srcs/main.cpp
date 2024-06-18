#include "../includes/parser.hpp"
#include "../includes/erros.hpp"

// void ParserClass::debug() const {
// 	std::cout << "Debugging ConfigParser State:" << std::endl;
// 	std::cout << "File being parsed: " << configFilePath << std::endl;
// 	std::cout << "Total directives parsed: " << conf_info.size() << std::endl;

// 	int blockCount = 1;
// 	for (std::vector<conf_File_Info>::const_iterator dir_it = conf_info.begin(); dir_it != conf_info.end(); ++dir_it) {
// 		const conf_File_Info& directive = *dir_it;
// 		std::cout << "Server Block " << blockCount++ << ":" << std::endl;
// 		std::cout << "\tListen: " << directive.portListen << std::endl;
// 		std::cout << "\tServer Name: " << directive.ServerName << std::endl;
// 		std::cout << "\tRoot: " << directive.RootDirectory << std::endl;
// 		std::cout << "\tIndex File: " << directive.defaultFile << std::endl;
// 		std::cout << "\tAutoindex: " << (directive.directoryListingEnabled ? "Enabled" : "Disabled") << std::endl;
// 		std::cout << "\tClient Max Body Size: " << directive.maxRequestSize << " bytes" << std::endl;
// 		std::cout << "\tUpload Directory: " << directive.fileUploadDirectory << std::endl;
		
// 		if (directive.redirectURL.httpStatusCode) {
// 			std::cout << "\tRedirect Code: " << directive.redirectURL.httpStatusCode << " to " << directive.redirectURL.destinationURL << std::endl;
// 		}

// 		std::cout << "\tAllowed Methods: ";
// 		for (std::set<std::string>::const_iterator meth_it = directive.allowedMethods.begin(); meth_it != directive.allowedMethods.end(); ++meth_it) {
// 			std::cout << *meth_it << " ";
// 		}
// 		std::cout << std::endl;

// 		std::cout << "\tError Pages:" << std::endl;
// 		for (std::map<int, std::string>::const_iterator page_it = directive.errorMap.begin(); page_it != directive.errorMap.end(); ++page_it) {
// 			std::cout << "\t\tError Code: " << page_it->first << " - Page: " << page_it->second << std::endl;
// 		}

// 		std::cout << "\tLocations Defined: " << directive.LocationsMap.size() << std::endl;
// 		for (Locations::const_iterator loc_it = directive.LocationsMap.begin(); loc_it != directive.LocationsMap.end(); ++loc_it) {
// 			std::cout << "\t\tLocation Path: " << loc_it->first << " - Root: " << loc_it->second.RootDirectory << std::endl;
// 	// Imprimir informações de CGI, se disponíveis
// 			if (!loc_it->second.Path_CGI.empty()) {
// 				std::cout << "\t\tCGI Path: " << loc_it->second.Path_CGI << std::endl;
// 			} else {
// 				std::cout << "\t\tCGI Path: Not Defined" << std::endl;
// 			}
// 		}       
// 	}
// }





// void setupServers(const char* configFileName, std::vector<ServerInfo>& servers, const conf_File_Info** config)
// {
//     ParserClass parser(configFileName);
//     ConfiguredServers configs = parser.fetchSpecifications();

//     for (size_t i = 0; i < configs.size(); ++i) {
//         ParserConfig parserConfig = configs[i];
//         *config = parserConfig.getServerConfigurations();
//         ServerInfo server;
//         setupServer(server, **config);
//         servers.push_back(server);
//     }
// }


// int main(int argc, char **argv)
// {
// 	// Configurar o manipulador de sinal
// 		signal(SIGINT, handle_sigint);
// 		global_path = "resources/";

// 	if (argc != 2)
// 	{
// 		std::cout << RED << "Error: Incorrect number of parameters provided.\n" << RESET;
// 		std::cout << GREEN << "Usage: Please run the program with the correct configuration file as follows:\n" << RESET;
// 		std::cout << "./webserv <config_file>\n";
// 		std::cout << YELLOW << "Example: ./webserv config.txt\n" << RESET;
// 		return (1);
// 	}
// 	try
// 	{
//         std::vector<ServerInfo> servers;
//         const conf_File_Info* config;
//         setupServers(argv[1], servers, &config);
//         runServer(servers, *config);
// 		servers.clear();
// 		remove_directory(global_path);

// 		std::cout << GREEN << "\nConfig file parsed successfully\n" << RESET;
// 	}
// 	catch(const std::exception &e)
// 	{
// 		std::cerr << RED << "Error: " << e.what() << RESET << std::endl;
// 	}
	
// 	return 0;
// }


// Variável global para armazenar o caminho do diretório
//const char *global_path = NULL;
// std::vector<int> global_sockets;
// std::vector<ServerInfo> servers;
// std::vector<const conf_File_Info*> configs;

volatile sig_atomic_t flag = 0;

//Manipulador de sinal
//Manipulador de sinal
void handle_sigint(int sig)
{
    (void)sig;
	flag = 1; 
    // if(global_path != NULL)
    //     remove_directory(global_path);
    
    // Close the sockets
    // for(std::vector<int>::iterator it = global_sockets.begin(); it != global_sockets.end(); ++it)
    // {
    //     shutdown(*it, SHUT_RDWR);
    //     close(*it);
    // }
    // global_sockets.clear();	

    // // Free the memory for the ServerInfo and conf_File_Info objects
    // // for (std::vector<ServerInfo>::iterator it = servers.begin(); it != servers.end(); ++it)
    // // {
    // //     it->freeMemory(); // You'll need to implement this method in the ServerInfo class
    // // }
    // servers.clear();

    // // for (std::vector<const conf_File_Info*>::iterator it = configs.begin(); it != configs.end(); ++it)
    // // {
    // //     delete *it;
    // // }
    // configs.clear();
    
    // std::cout << std::endl;
    //exit(0);
}


void setupServers(const char* configFileName, std::vector<ServerInfo>& servers, std::vector<const conf_File_Info*>* configs)
{
	ParserClass parser(configFileName);
	ConfiguredServers configuredServers = parser.fetchSpecifications();

	for (size_t i = 0; i < configuredServers.size(); ++i)
	{
		ParserConfig parserConfig = configuredServers[i];
		const conf_File_Info* config = parserConfig.getServerConfigurations();
		configs->push_back(config);
		ServerInfo server;
		setupServer(server, *config);
		servers.push_back(server);
		//global_sockets.push_back(server.getSocketFD());
	}
}


int main(int argc, char **argv)
{
	// Configurar o manipulador de sinal
	signal(SIGINT, handle_sigint);
	//global_path = "resources/";

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
		std::vector<ServerInfo> servers;
		std::vector<const conf_File_Info*> configs;
		setupServers(argv[1], servers, &configs);
		//for (size_t i = 0; i < configs.size(); ++i)
		//	runServer(servers);
		fd_set read_fds, write_fds;
        int max_fd;
        setupRunServer(servers, read_fds, write_fds, max_fd);

        for (size_t i = 0; i < configs.size(); ++i)
            runServer(servers, read_fds, write_fds, max_fd);
	
		servers.clear();

		//remove_directory(global_path);

		std::cout << GREEN << "\nConfig file parsed successfully\n" << RESET;
	}
	catch(const std::exception &e)
	{
		std::cerr << RED << "Error: " << e.what() << RESET << std::endl;
	}
	
	return 0;
}
