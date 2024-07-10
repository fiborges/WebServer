#include "../includes/parser.hpp"

volatile sig_atomic_t flag = 0;
std::vector<std::string> createdFiles;
std::map<int, std::map<std::string, ParserConfig> > serversByPortAndHost;

void handle_sigint(int sig)
{
	(void)sig;
	for (std::vector<std::string>::iterator it = createdFiles.begin(); it != createdFiles.end(); ++it)
	{
		std::remove(it->c_str());
	}
	std::cout << "\n\n" << RED << " ==> WebServer shutting down gracefully..." << RESET << std::endl;
	flag = 1;
}


ParserClass* setupServers(const char* configFileName, std::vector<ServerInfo*>& servers, std::vector<const conf_File_Info*>* configs)
{
	ParserClass *parser = new ParserClass(configFileName);
	ConfiguredServers configuredServers = parser->fetchSpecifications();
	std::map<int, std::vector<ParserConfig> > serversByPort;

	for (size_t i = 0; i < configuredServers.size(); ++i)
	{
		ParserConfig parserConfig = configuredServers[i];
		serversByPortAndHost[parserConfig.obtainPort()][parserConfig.retrieveHost()] = parserConfig;
		serversByPort[parserConfig.obtainPort()].push_back(parserConfig);
	}

    std::map<int, std::map<std::string, ParserConfig> >::iterator portEntry;
    // for (portEntry = serversByPortAndHost.begin(); portEntry != serversByPortAndHost.end(); ++portEntry)
	// {
    //     std::cout << "   Port: " << portEntry->first << std::endl;
    //     std::map<std::string, ParserConfig >::iterator hostEntry;
    //     for (hostEntry = portEntry->second.begin(); hostEntry != portEntry->second.end(); ++hostEntry)
    //         std::cout << "     Host: " << hostEntry->first << std::endl;
    // }

	std::set<int> processedPorts;
	for (portEntry = serversByPortAndHost.begin(); portEntry != serversByPortAndHost.end(); ++portEntry)
	{
		int port = portEntry->first;

		if (processedPorts.find(port) != processedPorts.end())
			continue;

		processedPorts.insert(port);

		std::map<std::string, ParserConfig>::iterator hostEntry;
		hostEntry = portEntry->second.begin();
		if (hostEntry != portEntry->second.end())
		{
			ParserConfig& config = hostEntry->second;
			const conf_File_Info configInfo = config.getServerConfigurations();
			ServerInfo *server = new ServerInfo();
			//std::cout << "CONFIG ORIGINAL: "  << configInfo.ServerName << " | " << configInfo.host << std::endl;
			configs->push_back(&configInfo);
			//std::cout << "CONFIG ADICIOANDO: "  << configInfo.ServerName << " | " << configInfo.host << std::endl;
			setupServer(*server, configInfo);
			servers.push_back(server);
			std::string host = hostEntry->first;
		}
	}
	return parser;
}

int main(int argc, char **argv)
{
	signal(SIGINT, handle_sigint);

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
		std::vector<ServerInfo*> servers;
		std::vector<const conf_File_Info*> configs;
		ParserClass* parser = setupServers(argv[1], servers, &configs);
		fd_set read_fds, write_fds;
		int max_fd;

		int sair2 = 0;
		for(size_t i = 0; i < servers.size(); ++i)
		{
			if (servers[i]->sair == 1)
			{
				sair2 = 1;
				break;
			}
		}

		if (sair2 == 0)
		{
			setupRunServer(servers, read_fds, write_fds, max_fd);

			for (size_t i = 0; i < configs.size(); ++i)
			{
				runServer(servers, read_fds, write_fds, max_fd);
			}
			std::cout << GREEN << SBLINK << "\n ==> WebServer exit successfully!\n\n" << RESET;
		}

		for(size_t i = 0; i < servers.size(); ++i)
		{
			close(servers[i]->getSocketFD());
			delete servers[i];
		}
		servers.clear();
		delete parser;
	}
	catch(const std::exception &e)
	{
		std::cerr << RED << "Error: " << e.what() << RESET << std::endl;
	}
	
	return 0;
}





// ParserClass* setupServers(const char* configFileName, std::vector<ServerInfo*>& servers, std::vector<const conf_File_Info*>* configs) {
//     ParserClass *parser = new ParserClass(configFileName);
//     ConfiguredServers configuredServers = parser->fetchSpecifications();
//     std::map<int, ParserConfig> firstConfigByPort;

//     for (size_t i = 0; i < configuredServers.size(); ++i) {
//         ParserConfig parserConfig = configuredServers[i];
//         int port = parserConfig.obtainPort();
//         // Store only the first configuration for each port
//         if (firstConfigByPort.find(port) == firstConfigByPort.end()) {
//             firstConfigByPort.insert(std::make_pair(port, parserConfig));
//         }
//     }

// 	//std::map<int, std::map<std::string, ParserConfig> >::iterator portEntry;
//     //for (portEntry = firstConfigByPort.begin(); portEntry != firstConfigByPort.end(); ++portEntry) {
// 	for (std::map<int, ParserConfig>::iterator it = firstConfigByPort.begin(); it != firstConfigByPort.end(); ++it) {
// 		std::cout << "Port: " << it->first << std::endl;
// 		// Directly use the returned string
// 		std::string hostName = it->second.retrieveHost();
// 		std::cout << "   Host: " << hostName << std::endl;
// 	}

//     std::set<int> processedPorts;
//     for (std::map<int, ParserConfig>::iterator it = firstConfigByPort.begin(); it != firstConfigByPort.end(); ++it) {
//         int port = it->first;
//         const ParserConfig& config = it->second;

//         // Check if the port has already been processed
//         if (processedPorts.find(port) != processedPorts.end())
//             continue; // Skip if already processed

//         processedPorts.insert(port); // Mark as processed

//         // Assuming setupServer and other logic remains the same
//         const conf_File_Info configInfo = config.getServerConfigurations();
//         ServerInfo *server = new ServerInfo();
//         configs->push_back(&configInfo);
// 		std::cout << "CONFIG ADICIOANDO: "  << configInfo.ServerName << " | " << configInfo.host << std::endl;
//         setupServer(*server, configInfo);
//         servers.push_back(server);
//     }

//     return parser;
// }