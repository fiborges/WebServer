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

void setupServers(const char* configFileName, std::vector<ServerInfo*>& servers, std::vector<const conf_File_Info*>* configs)
{
    ParserClass parser(configFileName);
    ConfiguredServers configuredServers = parser.fetchSpecifications();

    for (size_t i = 0; i < configuredServers.size(); ++i)
    {
        ParserConfig parserConfig = configuredServers[i];
        const conf_File_Info* config = parserConfig.getServerConfigurations();
        configs->push_back(config);
        ServerInfo* server = new ServerInfo();
		setupServer(*server, *config);
		servers.push_back(server);
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
	
	
		//remove_directory(global_path);

	}
	catch(const std::exception &e)
	{
		std::cerr << RED << "Error: " << e.what() << RESET << std::endl;
	}
	
	return 0;
}
