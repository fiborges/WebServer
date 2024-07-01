#include "../includes/parser.hpp"
#include "../includes/erros.hpp"


volatile sig_atomic_t flag = 0;
std::vector<std::string> createdFiles;

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

// void setupServers(const char* configFileName, std::vector<ServerInfo*>& servers, std::vector<const conf_File_Info*>* configs)
// {
// 	ParserClass parser(configFileName);
// 	ConfiguredServers configuredServers = parser.fetchSpecifications();

// 	for (size_t i = 0; i < configuredServers.size(); ++i)
// 	{
// 		std::string host = "fred";
// 		ParserConfig parserConfig = configuredServers[i];
// 		std::cout << GREEN << configuredServers[i].obtainPort() << RESET << std::endl;
// 		std::cout << GREEN << configuredServers[i].retrieveServerName() << RESET << std::endl;
// 		const conf_File_Info* config = parserConfig.getServerConfigurations();
// 		HTTrequestMSG httpRequestMsg;
// 		ServerInfo* server = new ServerInfo();
// 		checkDoors(*server, *config, httpRequestMsg);
// 		configs->push_back(config);
// 		setupServer(*server, *config);
// 		servers.push_back(server);
// 	}
// }

// void setupServers(const char* configFileName, std::vector<ServerInfo*>& servers, std::vector<const conf_File_Info*>* configs)
// {
//     ParserClass parser(configFileName);
//     ConfiguredServers configuredServers = parser.fetchSpecifications();
//     bool anyServerAddedBasedOnCriteria = false; // Track if any server was added based on criteria

//     for (size_t i = 0; i < configuredServers.size(); ++i)
//     {
//         bool shouldAddServer = true;
//         std::string host = "fred"; // Intended server name to check against, but it does not exist
//         ParserConfig parserConfig = configuredServers[i];

//         // Compare the current server with all subsequent servers
//         for (size_t j = i + 1; j < configuredServers.size(); ++j)
//         {
//             if (configuredServers[i].obtainPort() == configuredServers[j].obtainPort())
//             {
//                 if (configuredServers[i].retrieveServerName() != host && configuredServers[j].retrieveServerName() != host)
//                 {
//                     shouldAddServer = false;
//                     break; // No need to check further if we already decided not to add the server
//                 }
//             }
//         }

//         // Add the current server if the flag is active
//         if (shouldAddServer)
//         {
//             const conf_File_Info* config = parserConfig.getServerConfigurations();
//             HTTrequestMSG httpRequestMsg;
//             ServerInfo* server = new ServerInfo();
//             checkDoors(*server, *config, httpRequestMsg);
//             configs->push_back(config);
//             setupServer(*server, *config);
//             servers.push_back(server);
//             anyServerAddedBasedOnCriteria = true; // Mark that a server was added based on criteria
//         }
//     }

//     // If no server was added based on the criteria, add the first server configuration by default
//     if (!anyServerAddedBasedOnCriteria && !configuredServers.empty())
//     {
//         const conf_File_Info* config = configuredServers[0].getServerConfigurations(); // Assuming getServerConfigurations() is the correct method to obtain the config
//         HTTrequestMSG httpRequestMsg;
//         ServerInfo* server = new ServerInfo();
//         checkDoors(*server, *config, httpRequestMsg);
//         configs->push_back(config);
//         setupServer(*server, *config);
//         servers.push_back(server);
//     }
// }


#include <set> // Include set header

// void setupServers(const char* configFileName , std::vector<ServerInfo*>& servers, std::vector<const conf_File_Info*>* configs) {
//     ParserClass parser(configFileName); // Assuming ParserClass is defined elsewhere
//     ConfiguredServers configuredServers = parser.fetchSpecifications(); // Stubbed for example
//     std::set<int> addedPorts; // To track ports of added servers
//     std::map<int, std::vector<ParserConfig> > serversByPort;
// 	std::string host = "maria";

//     // Organize servers by port
//     for (size_t i = 0; i < configuredServers.size(); ++i) {
//         ParserConfig parserConfig = configuredServers[i];
//         serversByPort[parserConfig.obtainPort()].push_back(parserConfig);
//     }

//     // Process each port
//     for (std::map<int, std::vector<ParserConfig> >::iterator it = serversByPort.begin(); it != serversByPort.end(); ++it) {
//         bool fredFound = false;
//         for (size_t i = 0; i < it->second.size(); ++i) {
//             if (it->second[i].retrieveServerName() == host) {
//                 fredFound = true;
//                 break;
//             }
//         }

//         // If fred is found, add only fred, otherwise add all servers for this port
//         for (size_t i = 0; i < it->second.size(); ++i) {
//             if (fredFound && it->second[i].retrieveServerName() != host) {
//                 continue; // Skip non-fred servers if fred is found
//             }

//             const conf_File_Info* config = it->second[i].getServerConfigurations();
//             HTTrequestMSG httpRequestMsg; // Ensure this is of type HTTrequestMSG
//             ServerInfo* server = new ServerInfo();
//             checkDoors(*server, *config, httpRequestMsg);
//             configs->push_back(config);
//             setupServer(*server, *config);
//             servers.push_back(server);
//         }
//     }
// }


void setupServers(const char* configFileName, std::vector<ServerInfo*>& servers, std::vector<const conf_File_Info*>* configs)
{
	ParserClass parser(configFileName);
	ConfiguredServers configuredServers = parser.fetchSpecifications();
	std::set<int> addedPorts;
	std::map<int, std::vector<ParserConfig> > serversByPort;
	//HTTrequestMSG httpRequestMsg;
	std::string host = "rodrigo1";

	// Organize servers by port
	for (size_t i = 0; i < configuredServers.size(); ++i)
	{
		ParserConfig parserConfig = configuredServers[i];
		serversByPort[parserConfig.obtainPort()].push_back(parserConfig);
	}

	// Process each port
	for (std::map<int, std::vector<ParserConfig> >::iterator it = serversByPort.begin(); it != serversByPort.end(); ++it)
	{
		bool nameFound = false;
		for (size_t i = 0; i < it->second.size(); ++i)
		{
			if (it->second[i].retrieveServerName() == host)
			{
				nameFound = true;
				break;
			}
		}

		for (size_t i = 0; i < it->second.size(); ++i)
		{
			if (nameFound && it->second[i].retrieveServerName() != host)
			{
				continue;
			}

			if (addedPorts.find(it->first) != addedPorts.end())
			{
				continue;
			}

			const conf_File_Info* config = it->second[i].getServerConfigurations();
			ServerInfo* server = new ServerInfo();
			std::cout << "[B] Port Number: " << config->portListen << std::endl;
			std::cout << "[B] Server Name: " << config->ServerName << std::endl;
			configs->push_back(config);
			setupServer(*server, *config);
			servers.push_back(server);

			addedPorts.insert(it->first);
			if (!nameFound)
				break;
		}
	}
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
		setupServers(argv[1], servers, &configs);
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
				runServer(servers, read_fds, write_fds, max_fd);
			std::cout << GREEN << SBLINK << "\n ==> WebServer exit successfully!\n\n" << RESET;
		}

		for(size_t i = 0; i < servers.size(); ++i)
		{
			close(servers[i]->getSocketFD());
			delete servers[i];
		}
		servers.clear();

	}
	catch(const std::exception &e)
	{
		std::cerr << RED << "Error: " << e.what() << RESET << std::endl;
	}
	
	return 0;
}
