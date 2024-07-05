#include "../includes/parser.hpp"
//#include "../includes/erros.hpp"

volatile sig_atomic_t flag = 0;
std::vector<std::string> createdFiles;
std::map<int, std::map<std::string, ParserConfig> > serversByPortAndHost;


//Manipulador de sinal
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
	//std::set<int> addedPorts;
	std::map<int, std::vector<ParserConfig> > serversByPort;
	//HTTrequestMSG httpRequestMsg;
	//std::string host = httpRequestMsg.hostname;


	// Organize servers by port
	for (size_t i = 0; i < configuredServers.size(); ++i)
	{
		ParserConfig parserConfig = configuredServers[i];
		serversByPortAndHost[parserConfig.obtainPort()][parserConfig.retrieveHost()] = parserConfig;
		serversByPort[parserConfig.obtainPort()].push_back(parserConfig);
	}

	// std::cout << " Total ports in map: " << serversByPortAndHost.size() << std::endl;
    std::map<int, std::map<std::string, ParserConfig> >::iterator portEntry;
    // for (portEntry = serversByPortAndHost.begin(); portEntry != serversByPortAndHost.end(); ++portEntry) {
    //     std::cout << "   Port: " << portEntry->first << ", Hosts count: " << portEntry->second.size() << std::endl;
    //     std::map<std::string, ParserConfig >::iterator hostEntry;
    //     for (hostEntry = portEntry->second.begin(); hostEntry != portEntry->second.end(); ++hostEntry) {
    //         std::cout << "     Host: " << hostEntry->first << ", Configs count: " << hostEntry->first.size() << std::endl;
    //     }
    // }


	std::set<int> processedPorts;
	for (portEntry = serversByPortAndHost.begin(); portEntry != serversByPortAndHost.end(); ++portEntry)
	{
		int port = portEntry->first;

		// Se a porta já foi processada, pule para a próxima iteração do loop
		if (processedPorts.find(port) != processedPorts.end()) {
			continue;
		}

		// Marque a porta como processada
		processedPorts.insert(port);

		std::map<std::string, ParserConfig>::iterator hostEntry;
		hostEntry = portEntry->second.begin();
		if (hostEntry != portEntry->second.end())
		{
			ParserConfig& config = hostEntry->second;
			const conf_File_Info configInfo = config.getServerConfigurations();
			ServerInfo *server = new ServerInfo();

			//std::cout << "==> Port Number: " << configInfo.portListen << " | Server Name: " << configInfo.ServerName << " | Server Host: " << configInfo.host << std::endl;
			configs->push_back(&configInfo);
			setupServer(*server, configInfo);
			servers.push_back(server);

			std::string host = hostEntry->first;
			//std::cout << "Porta: " << port << ", Host: " << host << std::endl;
		}
	}
	return parser;
}





int main(int argc, char **argv)
{
	//GlobalFile globalFile(argv[1]);
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
		std::vector<ServerInfo*> servers;
		std::vector<const conf_File_Info*> configs;
		//setupServers(argv[1], servers, &configs);
		ParserClass* parser = setupServers(argv[1], servers, &configs);
		//for (size_t i = 0; i < configs.size(); ++i)
		//	runServer(servers);
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


// void setupServers(const char* configFileName, std::vector<ServerInfo*>& servers, std::vector<const conf_File_Info*>* configs)
// {
// 	ParserClass parser(configFileName);
// 	ConfiguredServers configuredServers = parser.fetchSpecifications();
// 	std::set<int> addedPorts;
// 	std::map<int, std::vector<ParserConfig> > serversByPort;
// 	HTTrequestMSG httpRequestMsg;
// 	std::string host = httpRequestMsg.hostname;
// 	//std::cout << "====> Hostname: " << httpRequestMsg.hostname << std::endl;

// 	// Organize servers by port
// 	for (size_t i = 0; i < configuredServers.size(); ++i)
// 	{
// 		ParserConfig parserConfig = configuredServers[i];
// 		serversByPort[parserConfig.obtainPort()].push_back(parserConfig);
// 	}

// 	// Process each port
// 	for (std::map<int, std::vector<ParserConfig> >::iterator it = serversByPort.begin(); it != serversByPort.end(); ++it)
// 	{
// 		bool nameFound = false;
// 		for (size_t i = 0; i < it->second.size(); ++i)
// 		{
// 			if (it->second[i].retrieveServerName() == host)
// 			{
// 				nameFound = true;
// 				break;
// 			}
// 		}

// 		for (size_t i = 0; i < it->second.size(); ++i)
// 		{
// 			if (nameFound && it->second[i].retrieveServerName() != host)
// 				continue;

// 			if (addedPorts.find(it->first) != addedPorts.end())
// 				continue;

// 			const conf_File_Info* config = it->second[i].getServerConfigurations();
// 			ServerInfo* server = new ServerInfo();
// 			std::cout << "==> Port Number: " << config->portListen << " | Server Name: " << config->ServerName << std::endl;
// 			configs->push_back(config);
// 			setupServer(*server, *config);
// 			servers.push_back(server);

// 			addedPorts.insert(it->first);
// 			if (!nameFound)
// 				break;
// 		}
// 	}
// }
