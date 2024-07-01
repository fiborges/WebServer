/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fde-carv <fde-carv@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/26 15:10:07 by fde-carv          #+#    #+#             */
/*   Updated: 2024/07/01 19:27:10 by fde-carv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../includes/get.hpp"

//std::vector<std::string> createdFiles;
//volatile sig_atomic_t flag = 0;

ServerInfo::ServerInfo()
{
	this->sockfd = -1;
	memset(&serv_addr, 0, sizeof(serv_addr));
	this->clientSocket = -1;
	//this->rootUrl = "resources";
	this->response = "";
	this->clientSockets.clear();
	this->portListen.clear();
	this->cli_addrs.clear();
	for (std::vector<sockaddr_in>::iterator it = this->cli_addrs.begin(); it != this->cli_addrs.end(); ++it)
		memset(&(*it), 0, sizeof(*it));
	this->contentLength = 0;
	this->rootOriginalDirectory = "";
	this->configs = std::map<int, conf_File_Info>();
	this->sair = 0;
}

ServerInfo::~ServerInfo()
{
	// Close all open sockets
	for (std::vector<int>::iterator it = clientSockets.begin(); it != clientSockets.end(); ++it)
	{
		if (*it != -1)
			close(*it);
	}
	clientSockets.clear();

	// Close the server socket
	if (sockfd >= 0)
	{
		close(sockfd);
		sockfd = -1;
	}

	portListen.clear();

	// Clean up any other allocated resources here
	cli_addrs.clear();
	response.clear();
	rootUrl.clear();
	rootOriginalDirectory.clear();
	configs.clear();
	complete_path.clear();

	// Clear remaining variables
	contentLength = 0;
	memset(&serv_addr, 0, sizeof(serv_addr));
	//std::cout << "== Destructor chamado ==" << std::endl;
}

void	ServerInfo::setSocketFD(int socket)
{
	sockfd = socket;
}

int	ServerInfo::getSocketFD() const
{
	return (sockfd);
}

void ServerInfo::setAddress(const sockaddr_in& address)
{
	serv_addr = address;
}

void	ServerInfo::setResponse(const std::string& response)
{
	this->response = response;
}


std::string	ServerInfo::getResponse() const
{
	return (response);
}

std::string ServerInfo::getRootUrl() const
{
	return rootUrl;
}

void	ServerInfo::addSocketToList(int sockfd)
{
	clientSockets.push_back(sockfd);
}

void	ServerInfo::addPortToList(int port)
{
	portListen.push_back(port);
}

std::vector<int> ServerInfo::getPortList() const
{
	return portListen;
}

std::vector<int>& ServerInfo::getSockets()
{
	return clientSockets;
};

void ServerInfo::setContentLength(size_t length)
{
	contentLength = length;
}

size_t ServerInfo::getContentLength() const
{
	return contentLength;
}

void ServerInfo::addConfig(int port, const conf_File_Info& config)
{
	configs[port] = config;
}

conf_File_Info& ServerInfo::getConfig(int port)
{
	return configs[port];
}

void ServerInfo::cleanup1()
{
	// Fecha todos os sockets de clientes
	for (std::vector<int>::iterator it = clientSockets.begin(); it != clientSockets.end(); ++it)
	{
		if (*it != -1)
			close(*it);
	}
	clientSockets.clear();

	// Fecha o socket principal
	if (sockfd != -1)
	{
		close(sockfd);
		sockfd = -1; // Evita uso futuro do socket fechado
	}

	// Limpa outras estruturas
	portListen.clear();
	cli_addrs.clear();
	configs.clear();
}

void ServerInfo::cleanup2()
{
	// Close all open sockets
	for (std::vector<int>::iterator it = clientSockets.begin(); it != clientSockets.end(); ++it)
	{
		close(*it);
	}
	clientSockets.clear();

	// Close the server socket
	if (sockfd != -1)
	{
		close(sockfd);
		sockfd = -1;
	}

	// Clean up any other allocated resources here
	cli_addrs.clear();
	response.clear();
	rootUrl.clear();
	portListen.clear();
	rootOriginalDirectory.clear();
	configs.clear();
	complete_path.clear();
}

void ServerInfo::setCompletePath(const std::string& path)
{
	std::string newPath = path;
	std::size_t lastDotPos = newPath.find_last_of('.');
	std::size_t lastSlashPos = newPath.find_last_of('/');

	if (lastDotPos != std::string::npos && lastDotPos > lastSlashPos)
	{
		if (!newPath.empty() && newPath[newPath.size() - 1] == '/')
			newPath.erase(newPath.size() - 1);
	}
	complete_path = newPath;
}

std::string ServerInfo::getCompletePath2() const
{
	return complete_path;
}

std::string ServerInfo::getRootOriginalDirectory() const
{
	return rootOriginalDirectory;
}

void ServerInfo::setRootOriginalDirectory(const std::string& dir)
{
	rootOriginalDirectory = dir;
}

// ================================================================================================= //
// ======================================= HELPER FUNCTIONS ======================================== //
// ================================================================================================= //

// Convert the method (int) to a string
std::string methodToString(HTTrequestMSG::Method method)
{
	switch (method) {
		case HTTrequestMSG::GET:
			return "GET";
		case HTTrequestMSG::POST:
			return "POST";
		case HTTrequestMSG::DELETE:
			return "DELETE";
		default:
			return "UNKNOWN";
	}
}

// Print the log of the request
void printLog(const std::string& method, const std::string& path, const std::string& version, const std::string& httpResponse, ServerInfo& server)
{
	static int requestCount = -1;
	requestCount++;

	time_t now = time(NULL);
	char timestamp[100];
	strftime(timestamp, sizeof(timestamp), "[%d/%b/%Y %T]", localtime(&now));

	std::string methodColor = (method == "GET") ? YELLOW : (method == "POST") ? CYAN : MAGENTA;

	std::string statusCodeStr;
	size_t statusCodePos = httpResponse.find("HTTP/1.1") + 9; // Position after "HTTP/1.1"
	if (statusCodePos != std::string::npos && httpResponse.length() >= statusCodePos + 3)
		statusCodeStr = httpResponse.substr(statusCodePos, 3);
	int statusCode = (statusCodeStr.empty()) ? 0 : atoi(statusCodeStr.c_str());

	std::string statusColor = (statusCode == 200) ? GREEN : RED;
	std::vector<int> ports = server.getPortList();
	if (!ports.empty())
	{
		int currentPort = ports.front();
		std::cout << BG_CYAN_BLACK << timestamp << RESET << GREEN << " [" << RESET << requestCount << GREEN << "] " << BLUE << \
		"Connected with client at 127.0.0.1:" << CYAN << currentPort << RESET << std::endl;
	}
	std::cout << BG_CYAN_BLACK << timestamp << RESET << BLUE << " [" << RESET << requestCount << BLUE << "]" \
		<< RESET << methodColor << " \"" << method << " " << path << " ";
	if (!version.empty())
		std::cout << version;
	std::cout << "\" " << RESET << statusColor << statusCode << RESET << " ";

	if (method == "GET")
		std::cout << server.getResponse().length();
	else if(method == "POST")
		std::cout << server.getContentLength();

	std::cout << "\n" << BG_CYAN_BLACK << timestamp << RESET << RED << " [" << RESET << requestCount << RED << "] " \
	<< BLUE << "Connection ended successfully" << RESET << std::endl;
}


// Function to handle errors without exiting the program
void ServerInfo::handleError(const std::string& errorMessage) //, int errorCode)
{
	std::cerr << RED1 << "\n" << errorMessage << RESET << std::endl;
	cleanup2();
	//exit(-1);
}

// Checks if the path is a directory
bool is_directory(const std::string &path)
{
	std::string modifiedPath = path;
	size_t lastDot = modifiedPath.find_last_of(".");
	size_t lastSlash = modifiedPath.find_last_of("/");

	if (lastDot != std::string::npos && lastDot > lastSlash)
	{
		if (modifiedPath[modifiedPath.length() - 1] == '/')
		{
			modifiedPath.erase(modifiedPath.length() - 1);
		}
	}

	//std::cout << " @@@@@@ Full path: " << modifiedPath << std::endl;
	DIR *dir = opendir(modifiedPath.c_str());
	if (dir)
	{
		//std::cout << " @@@@@@ Directory " << modifiedPath << " exists.\n";
		closedir(dir);
		return true;
	}
	else
	{
		//std::cout << " @@@@@@ Directory " << modifiedPath << " does not exist.\n";
		return false;
	}
}

// Setup the server
void setupServer(ServerInfo& server, const conf_File_Info& config)
{
	int sockfd = -1;
	try {
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
		{
			throw std::runtime_error("Error on socket creation");
		}
		// Add the socket to the global_sockets vector
		//global_sockets.push_back(sockfd);
		server.addSocketToList(sockfd);
		server.setSocketFD(sockfd); // Set the socket descriptor here
		sockaddr_in serv_addr;
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(config.portListen);
		server.setAddress(serv_addr);
		// setupDirectory(server, config);
		// std::string serverRoot = config.RootDirectory;
		// std::string fileUploadDirectory = config.fileUploadDirectory;
		// setupUploadDirectory(serverRoot, fileUploadDirectory);

		int opt = 1;
		if (setsockopt(server.getSocketFD(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		{
			throw std::runtime_error("Error on setsockopt");
		}
		// server.setSocketFD(sockfd); // This line is not needed
		if (bind(server.getSocketFD(), (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		{
			//throw std::runtime_error("Error on binding");
			std::cout << RED << "Error on binding" << RESET << std::endl;
			server.sair = 1;
			return ;
		}
		if (listen(server.getSocketFD(), 128) < 0)
		{
			throw std::runtime_error("Error on listen");
		}

		server.addPortToList(config.portListen);
		server.addConfig(config.portListen, config);

		std::string actualRoot = config.RootDirectory;
		conf_File_Info configForFirstPort = server.getConfig(server.getPortList()[0]);
		std::string serverRoot = configForFirstPort.RootDirectory;

		//std::cout << "Actual Root: " << actualRoot << std::endl;
		//std::cout << "Server Root: " << serverRoot << std::endl;

		if (serverRoot != actualRoot)
		{
			throw std::runtime_error("404 Not Found: The requested server root does not match the actual server root.");
		}

		if(serverRoot[serverRoot.size() - 1] != '/')
			serverRoot += "/";
		//std::cout << " +++++ Server Root: " << serverRoot << std::endl;
	}
	catch (const std::runtime_error& e)
	{
		if (sockfd != -1)
		{
			close(sockfd);
		}
		std::cerr << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}

}


//Read the request from the client and return it as a string
std::string readRequest(int sockfd, ServerInfo& server)
{
	char buffer[4096];
	std::string request;

	// Read the header
	while (1)
	{
		memset(buffer, 0, 4096);
		ssize_t bytesRead = recv(sockfd, buffer, 4095, 0);
		if (bytesRead < 0)
		{
			perror("recv");
			server.handleError("Error reading from socket.");
			server.sair = 1;
			return request;
			close(sockfd);//acrescimo evolution sheet
			//exit(-1);
		}
		else if (bytesRead == 0)
			break;
		else
			buffer[bytesRead] = '\0';

		request.append(buffer, bytesRead);
		if (request.find("\r\n\r\n") != std::string::npos)
			break;
	}

	// Read the Body
	HTTPParser parser;
	size_t contentLength = parser.getContentLength(request);
	// int raw = parser.get
	server.setContentLength(contentLength);
	//std::cout << "  Content-Length1: " << contentLength << std::endl;

	size_t actualDataSize = request.size();
	size_t headerSize = request.find("\r\n\r\n") + 4;
	//std::cout << "headerSize: " << headerSize << std::endl;
	//std::cout << "  DataSize: " << actualDataSize << std::endl;
	//std::cout << "  Raw: " <<  << std::endl;

	//server.setContentLength(actualDataSize - headerSize);
	//std::cout << "Content-Length2: " << contentLength << std::endl; // Print the content length

	if (contentLength > actualDataSize - headerSize)
	{
		size_t bytesReadTotal = actualDataSize - headerSize;
		while (bytesReadTotal < contentLength)
		{
			memset(buffer, 0, 4096);
			ssize_t bytesRead = recv(sockfd, buffer, std::min(static_cast<size_t>(4095), contentLength - bytesReadTotal), 0); // EWOULDBLOCK (esta flag bloqueia fich grandes)			//std::cout << "Bytes read: " << bytesRead << std::endl; // Print the number of bytes read

			if (bytesRead < 0)
			{
				server.handleError("Error reading from socket.");
				break ;
				//exit(-1);
			}
			else if (bytesRead == 0)
			{
				server.handleError("Socket has been closed by the other end.");
				// std::cerr << "Socket has been closed by the other end." << std::endl;
				break ;
			}
			else
			{
				buffer[bytesRead] = '\0';
			}

			if (bytesReadTotal > request.size() - headerSize)
			{
				std::cerr << "Read beyond the end of available data." << std::endl;
				break;
			}
			request.append(buffer, bytesRead);
			bytesReadTotal += bytesRead;
			//std::cout << "Bytes read total: " << bytesReadTotal << std::endl; // Print the number of bytes read
		}
	}
	//std::cout << "Request received: \n" << request << std::endl; // Print the request

	return request;
}

// To check tokens size for determinate root directory
std::vector<std::string> tokenize(const std::string& str, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(str);

	while (std::getline(tokenStream, token, delimiter))
	{
		if (!token.empty())
		{
			tokens.push_back(token);
			//std::cout << "TOKEN: " << token << std::endl;
		}
	}

	//std::cout << std::endl;
	return tokens;
}

// Make a new relative path from the root and the path of locations
std::string getNewPath(const std::string& root, const std::string& path)
{

	std::vector<std::string> rootTokens = tokenize(root, '/');
	std::vector<std::string> pathTokens = tokenize(path, '/');

	// for (size_t i = 0; i < rootTokens.size(); ++i) {
	//     std::cout << "==> Root tokens[" << i << "]: " << rootTokens[i] << std::endl;
	// }

	// for (size_t i = 0; i < pathTokens.size(); ++i) {
	//     std::cout << "==> Path tokens[" << i << "]: " << pathTokens[i] << std::endl;
	// }

	std::string newPath;
	std::string initialPath;
	if(pathTokens.empty())
		return root;
	else
	{
		size_t i = 0, j = 0;
		bool hasMatched = false;

		// Find common part
		while (i < rootTokens.size() && j < pathTokens.size()) {
			std::cout << "==> Comparing rootTokens[" << i << "]: " << rootTokens[i] << " with pathTokens[" << j << "]: " << pathTokens[j] << std::endl;
			if (rootTokens[i] == pathTokens[j]) {
				hasMatched = true;
				newPath += "/" + rootTokens[i];
				++j;
			} else if (hasMatched) {
				break;
			} else {
				initialPath += "/" + rootTokens[i];
			}
			++i;
		}
		// std::cout << "==> Initial path: " << initialPath << std::endl;
		// std::cout << "==> Final path: " << newPath << std::endl;
	}

	return initialPath + newPath;
}

// Take de first directory from a path
std::string getDirectoryPath(const std::string& fullPath)
{
	if (fullPath.empty() || fullPath[0] != '/')
		return "";
	size_t start = 1;
	size_t end = fullPath.find("/", start);
	if (end == std::string::npos)
		return fullPath.substr(0, fullPath.length());
	return fullPath.substr(0, end);
}

// Checks if the method is allowed
bool isMethodAllowed(const std::set<std::string>& allowedMethods, const std::string& requestMethod)
{

	if (allowedMethods.empty())
		return true; // Todos os métodos são permitidos se o conjunto de métodos permitidos estiver vazio

	std::string upperRequestMethod = requestMethod;

	for (std::set<std::string>::const_iterator it_meth = allowedMethods.begin(); it_meth != allowedMethods.end(); ++it_meth)
	{
		std::string allowedMethods = *it_meth;
		// std::cout << MAGENTA << "Current allowedMethods string: " << RESET << *it_meth << std::endl;
		// std::cout << MAGENTA << "Checking method: " << RESET << allowedMethods << std::endl;
		// std::cout << MAGENTA << "Request method: " << RESET << requestMethod << std::endl;
		std::string allowedMethod = *it_meth;
		std::transform(allowedMethod.begin(), allowedMethod.end(), allowedMethod.begin(), ::toupper);
		if (allowedMethod == upperRequestMethod)
			return true;
	}
	return false;
}

// Takes a relative path ou transformes in absolute path
std::string ServerInfo::getCompletePath(const std::string& path)
{
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	std::string full_path2 = std::string(cwd);

	if (!path.empty() && path[0] != '/')
		full_path2 += '/';

	full_path2 += path;
	//std::cout << "[getCompletePath] @@ Full path: " << full_path2 << std::endl;

	return full_path2;
}

// If last token is a file, remove it
std::string ifFileRmoveFile(std::string path)
{
	if (!path.empty() && path[path.size() - 1] == '/')
		path.erase(path.size() - 1);

	std::string::size_type lastSlashPos = path.find_last_of('/');
	std::string lastPart = path.substr(lastSlashPos + 1);

	if (lastPart.find('.') != std::string::npos)
		path = path.substr(0, lastSlashPos);

	//std::cout << "[isFileAndRemoveTrailingSlash] path: " << path << std::endl;
	return path;
}

std::string removeLastSlash(const std::string& fullPath)
{
	if (fullPath == "/") {
		return fullPath;
	}
	if (!fullPath.empty() && fullPath[fullPath.size() - 1] == '/') {
		return fullPath.substr(0, fullPath.size() - 1);
	}
	return fullPath;
}

// Checks if AUTOINDEX is enabled and handles the directory listing
bool handleDirectoryListing(conf_File_Info& serverConfig, HTTrequestMSG& requestMsg, ServerInfo& server)
{
	(void)requestMsg;
	if(serverConfig.directoryListingEnabled)
	{
		std::string rootDirectory = serverConfig.RootDirectory;
		if (!rootDirectory.empty() && rootDirectory[0] == '/')
			rootDirectory = rootDirectory.substr(1);

		std::string fullPath = server.getCompletePath2();
		//std::cout << "[AUTOINDEX] rootDirectory PATH: " << rootDirectory << std::endl;
		//std::cout << "[AUTOINDEX] requestMSG PATH original: " << requestMsg.path << std::endl;
		//std::cout << "[AUTOINDEX] Full path: " << fullPath << std::endl;

		if (fullPath.at(0) != '/')
		{
			fullPath = "/" + fullPath;
		}

		std::string full_path1 = fullPath;;
		std::string full_path2;
		if (full_path1.substr(0, 5) == "/home")
		{
			full_path2 = full_path1;
		}
		else
		{
			char cwd[1024];
			getcwd(cwd, sizeof(cwd));
			full_path2 = std::string(cwd) + full_path1;
		}

		struct stat path_stat;
		if(stat(full_path2.c_str(), &path_stat) != 0)
		{
			//std::cerr << "Error accessing " << full_path2 << ": " << strerror(errno) << std::endl;
			//server.setResponse("HTTP/1.1 500 Internal Server Error\r\n\r\n");
			handleError2(500, server, serverConfig, requestMsg);
			return false;
		}
		bool is_directory = S_ISDIR(path_stat.st_mode);

		std::string response;
		if (is_directory)
		{
			DIR *dir;
			struct dirent *ent;
			std::vector<int> portList = server.getPortList();
			int port = portList[0];
			std::string serverAddress = "127.0.0.1"; // replace this with actual function to get server address
			std::stringstream ss;
			ss << port;
			std::string portStr = ss.str();
			if ((dir = opendir (full_path2.c_str())) != NULL)
			{
				response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
				response += "<html><head><style>body { background: #ADD8E6; }</style></head><body>";

				std::vector<std::string> entries;
				while ((ent = readdir (dir)) != NULL)
				{
					entries.push_back(ent->d_name);
				}
				std::sort(entries.begin(), entries.end()); // Sort the entries

				for (size_t i = 0; i < entries.size(); i++)
				{
					response += entries[i];
					response += "<br>";
				}

				response += "<br>";
				response += "<button onclick=\"location.href='http://" + serverAddress + ":" + portStr + "'\" type=\"button\">HOME</button>";
				response += "</body></html>\n";
				closedir (dir);
			}
			else
			{
				//perror ("");
				//response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
				handleError2(500, server, serverConfig, requestMsg);
			}
			server.setResponse(response);
		}
		else
		{
			std::ifstream file(full_path2.c_str());
			if (file.is_open())
			{
				response = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
				server.setResponse(response);
			}
			else
			{
				handleError2(404, server, serverConfig, requestMsg);
				//server.setResponse("HTTP/1.1 404 Not Found AUTOINDEX\r\n\r\n");
			}
		}
	}
	return true;
}

void processErrorPage(std::string second, int errorCode, const std::string& rootDirectory)
{
	// Remover '/' se existir
	size_t pos = second.find('/');
	if (pos != std::string::npos)
	{
		second.erase(pos, 1);
	}

	//std::cout << "second: " << second << std::endl;
	// Procurar por três dígitos e comparar com errorCode
	if (second.size() >= 3)
	{
		std::string threeDigits = second.substr(0, 3);
		//std::cout << "threeDigits: " << threeDigits << std::endl;

		// Convert errorCode to string and get the first two digits
		std::stringstream ss;
		ss << errorCode;
		std::string errorCodeStr = ss.str();
		std::string firstTwoDigits = errorCodeStr.substr(0, 2);

		// If the first digit is the same, replace the second and third digits
		if (threeDigits[0] == errorCodeStr[0])
		{
			threeDigits[1] = errorCodeStr[1];
			threeDigits[2] = errorCodeStr[2];
		}
		//std::cout << "threeDigits2: " << threeDigits << std::endl;

		if (std::atoi(threeDigits.c_str()) == errorCode)
		{

			std::string path = rootDirectory + "/" + threeDigits + ".html";
			std::ofstream file(path.c_str());
			if (file)
			{

				// Create an instance of ServerErrorHandler
				ServerErrorHandler errorHandler;
				// Generate the error page content
				std::string errorPageContent = errorHandler.generateErrorPage(errorCode);
				// Write the error page content to the file
				file << errorPageContent;
				file.close();
				//std::cout << "  " << errorCode << ".html file created at " << path << std::endl;
				createdFiles.push_back(path);

				// file << "<html>\n"
				//      << "<head><title>" << errorCode << " Not Found</title></head>\n"
				//      << "<body>\n"
				//      << "<h1>" << errorCode << " Not Found</h1>\n"
				//      << "<p>The requested URL was not found on this server.</p>\n"
				//      << "</body>\n"
				//      << "</html>\n";
				// file.close();
				// std::cout << "  " << errorCode << ".html file created at " << path << std::endl;
			}
			// else
			// {
			// 	std::cerr << "  Error: Could not create " << errorCode << ".html file at " << path << std::endl;
			// }
		}
	}
}

void createHtmlFiles(const std::string& rootDirectory)
{
	ServerErrorHandler handler;
	std::string path = rootDirectory + "/" + "delete.html";
	std::ofstream file(path.c_str());
	if (file)
	{
		std::string htmlContent = handler.generateDelete();
		file << htmlContent;
		file.close();
		createdFiles.push_back(path);
	}
	path = rootDirectory + "/" + "get.html";
	std::ofstream file2(path.c_str());
	if (file2)
	{
		std::string htmlContent = handler.generateGet();
		file2 << htmlContent;
		file2.close();
		createdFiles.push_back(path);
	}
	path = rootDirectory + "/" + "post.html";
	std::ofstream file3(path.c_str());
	if (file3)
	{
		std::string htmlContent = handler.generatePost();
		file3 << htmlContent;
		file3.close();
		createdFiles.push_back(path);
	}
	path = rootDirectory + "/" + "upload.html";
	std::ofstream file4(path.c_str());
	if (file4)
	{
		std::string htmlContent = handler.generateUpload();
		file4 << htmlContent;
		file4.close();
		createdFiles.push_back(path);
	}
}

void createIndexFile(conf_File_Info &serverConfig, const std::string& rootDirectory)
{
	std::string name;
	if (serverConfig.defaultFile.empty())
		name = "index.html";
	else
		name = serverConfig.defaultFile;
	//std::cout << "name: " << name << std::endl;
	//std::cout << "RootDirectory: " << serverConfig.RootDirectory << std::endl;

	std::string path = rootDirectory + "/" + name;
	//std::cout << "HTML path: " << path << std::endl;
	std::ofstream file(path.c_str());
	if (file)
	{
		// Create an instance of ServerErrorHandler
		ServerErrorHandler handler;
		// Generate the error page content
		std::string base = handler.generateIndex(name);
		// Write the error page content to the file
		file << base;
		file.close();
		createdFiles.push_back(path);
		//std::cout << "  " << errorCode << ".html file created at " << path << std::endl;
		// createdFiles.push_back(path);
	}
	// else
	// {
	// 	std::cerr << "  Error: Could not create "  << std::endl;
	// }

}


// Principal Function to deal with rules from .conf file
bool processRulesRequest(HTTrequestMSG& requestMsg, ServerInfo& server)
{

	std::vector<int> ports = server.getPortList();
	int listeningPort = ports[0];
	conf_File_Info &serverConfig = server.getConfig(listeningPort);

	server.setRootOriginalDirectory(serverConfig.RootDirectory);

	//std::map<std::string, conf_File_Info> locationConfigs = serverConfig.LocationsMap;

	std::string browserRelativePath = removeLastSlash(requestMsg.path);
	//std::cout << "$$ >>>>> requestMsg.path: " << browserRelativePath << std::endl;
	//std::cout << "$$ >>>>> requestMsg.method: " << methodToString(requestMsg.method) << std::endl;

	std::map<int, std::string> errorMap;
	for (std::map<int, std::string>::const_iterator it = serverConfig.errorMap.begin(); it != serverConfig.errorMap.end(); ++it)
	{
		if (it != serverConfig.errorMap.end())
			errorMap[it->first] = it->second;
		//std::cout << "  " << it->first << " ---> " << it->second << std::endl;

		// Se a diretiva error_page estiver presente e o código de erro for 404, crie o arquivo 404.html
		//std:: cout << "    it->first: " << it->first << std::endl;
		//std:: cout << "    it->second: " << it->second << std::endl;

		processErrorPage(it->second, it->first, serverConfig.RootDirectory);
	}

	createIndexFile(serverConfig, serverConfig.RootDirectory);
	createHtmlFiles(serverConfig.RootDirectory);

	if (serverConfig.LocationsMap.size() > 0)
	{
		for (Locations::const_iterator it = serverConfig.LocationsMap.begin(); it != serverConfig.LocationsMap.end(); ++it)
		{
			//std::cout << "$ >>>>> it->first: " << it->first << std::endl;
			if (it->first == browserRelativePath)
			{
				if(it->first == "/")
				{
					//std::cout << "ENTREI no /"	<< std::endl;
					//std::cout << "[processRules]: "<< requestMsg.path << std::endl;
					std::string newMixedPath = getNewPath(serverConfig.RootDirectory, it->second.RootDirectory);
					std::string completeNewMixedPath = server.getCompletePath(newMixedPath);
					// if(requestMsg.path == "/")
					// 	requestMsg.path = "";
					std::string completeFullPath = completeNewMixedPath + requestMsg.path;
					//std::cout << YELLOW << completeFullPath << RESET << std::endl;

					//std::cout << " ## Root directory changed to: " << serverConfig.RootDirectory << std::endl;
					server.setCompletePath(completeFullPath);
					//std::cout << " ## CompletePath: " << server.getCompletePath2() << std::endl;
					//std::cout << " ## is_directory?: " << !is_directory(serverConfig.RootDirectory) << std::endl;

					std::string requestMethod = methodToString(requestMsg.method);
					bool methodAllowed = isMethodAllowed(it->second.allowedMethods, requestMethod);
					if (!methodAllowed)
					{
						// std::cerr << "Error: Forbidden method." << std::endl;
						// server.setResponse("HTTP/1.1 403 Forbidden\r\nContent-Type: text/plain\r\n\r\nMethod is Forbidden\nERROR 403\n");
						// requestMsg.path = "Forbidden";
						// requestMsg.version = "";
						// printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
						handleError2(403, server, serverConfig, requestMsg);
						return false;
					}
					return true;
				}
				//std::cout << "it antes redirect: " << it->first << std::endl;
				if(it->first == "/redirect")
				{
					//std::cout << "ENTREI NO REDIRECT" << std::endl;
					std::string newUrl = serverConfig.redirectURL.destinationURL;
					std::stringstream ss;
					ss << serverConfig.redirectURL.httpStatusCode;
					std::string response = requestMsg.version + " " +
										ss.str() + " " +
										"Moved Permanently\r\n"
										"Location: " + newUrl + "\r\n"
										"\r\n";
					std::cout << "RESPONSE: " << response << std::endl;
					server.setResponse(response);
					//std::cout << "CONTINUANDO NO REDIRECT" << std::endl;
					printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
					//std::cout << "FIM DO REDIRECT" << std::endl;
					return false;
				}

				std::string requestMethod = methodToString(requestMsg.method);
				bool methodAllowed = isMethodAllowed(it->second.allowedMethods, requestMethod);
				if (!methodAllowed)
				{
					//std::cerr << "Error: Forbidden method." << std::endl;
					//server.setResponse("HTTP/1.1 403 Forbidden\r\nContent-Type: text/plain\r\n\r\nMethod is Forbidden\nERROR 403\n");
					//requestMsg.path = "Forbidden";
					//requestMsg.version = "";
					//printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
					handleError2(405, server, serverConfig, requestMsg);
					return false;
				}

				std::string newMixedPath = getNewPath(serverConfig.RootDirectory, it->second.RootDirectory);
				std::string completeNewMixedPath = server.getCompletePath(newMixedPath);
				// if(requestMsg.path == "/")
				// 	requestMsg.path = "";
				std::string completeFullPath = completeNewMixedPath + requestMsg.path;
				//std::cout << "MMM: "<<MAGENTA << completeFullPath << RESET << std::endl;
				serverConfig.RootDirectory = newMixedPath;
				server.setCompletePath(completeFullPath);
				//std::cout << "OLA\n";
				if (is_directory(completeFullPath))
				{
					//std::cout << "ENTREI Na directoria (IF)" << std::endl;
					//std::cout << "checkar se existe index.html: "  << fileExistsInDirectory(completeFullPath, serverConfig.defaultFile) << std::endl;
					//std::cout << "AutoindexPresent : " << serverConfig.autoindexPresent << std::endl;

					if ((it->second.autoindexPresent == true))
					{
						if ((it->second.directoryListingEnabled == true) && (!fileExistsInDirectory(completeFullPath, serverConfig.defaultFile)))
						{
							//std::cout << "MMM 1: " <<it->second.directoryListingEnabled << std::endl;
							if (handleDirectoryListing(serverConfig, requestMsg, server))
								return true;
						}
					}
					else
					{
						if ((serverConfig.autoindexPresent == true) && (!fileExistsInDirectory(completeFullPath, serverConfig.defaultFile)))
						{
							if (serverConfig.directoryListingEnabled == true)
							{
								//std::cout << "MMM 2: " <<it->second.directoryListingEnabled << std::endl;
								if (handleDirectoryListing(serverConfig, requestMsg, server))
									return true;
							}
						}
					}

				}
			}
			else if (it->first == getDirectoryPath(requestMsg.path))
			{
				//std::cout << " \n+++++++++++++++++ ENTREI NO ELSE IF +++++++++++++++" << std::endl;
				// std::string fred = getNewPath(serverConfig.RootDirectory, it->second.RootDirectory);
				// std::string aaa = server.getCompletePath(fred);
				// aaa = aaa + requestMsg.path;
				// std::cout << GREEN <<  aaa << RESET << std::endl;
				//std::cout << "getDirectoryPath(requestMsg.path): " << getDirectoryPath(requestMsg.path) << std::endl;
				//std::cout << "serverConfig.RootDirectory: " << serverConfig.RootDirectory << std::endl;
				//std::cout << "it->second.RootDirectory: " << it->second.RootDirectory << std::endl;
				std::string fred = getNewPath(serverConfig.RootDirectory, it->second.RootDirectory);
				//std::cout << "fred: " << fred << std::endl;
				std::string bbb = server.getCompletePath(fred);
				std::string aaa = bbb + requestMsg.path;
				//std::cout << GREEN <<  aaa << RESET << std::endl;

				//serverConfig.RootDirectory = aaa;
				//std::cout << " ## Root directory changed to: " << serverConfig.RootDirectory << std::endl;
				server.setCompletePath(aaa);
				//std::cout << " ## CompletePath: " << server.getCompletePath2() << std::endl;

				int bodySizeBytes = server.getContentLength();
				std::cout << "  Content-Length : " << bodySizeBytes << std::endl;
				bodySizeBytes -= 200;
				//std::cout << "  Content-Length ALTERADO: " << bodySizeBytes << std::endl;
				//std::cout << "  Server Max Limit: " << serverConfig.maxRequestSize << std::endl;
				if(bodySizeBytes > serverConfig.maxRequestSize)
				{
					//std::cerr << "Error: Request size exceeds the maximum allowed size." << std::endl;
					//server.setResponse("HTTP/1.1 413 Request Entity Too Large\r\nContent-Type: text/plain\r\n\r\nRequest size exceeds the maximum allowed size\nERROR 413\n");
					//requestMsg.path = "Request Entity Too Large";
					//requestMsg.version = "";
					//printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
					handleError2(413, server, serverConfig, requestMsg);
					return false;
				}

				serverConfig.RootDirectory  = getNewPath(serverConfig.RootDirectory, it->second.RootDirectory);
				std::cout << "    ## Root FRED : " << serverConfig.RootDirectory << std::endl;

				std::string requestMethod = methodToString(requestMsg.method);
				std::transform(requestMethod.begin(), requestMethod.end(), requestMethod.begin(), ::toupper);//mudar para maiusculas
				bool methodAllowed = isMethodAllowed(it->second.allowedMethods, requestMethod);
				if (!methodAllowed)
				{
					// std::cerr << "Error: Forbidden method." << std::endl;
					// server.setResponse("HTTP/1.1 403 Forbidden\r\nContent-Type: text/plain\r\n\r\nMethod is Forbidden\nERROR 403\n");
					// requestMsg.path = "Forbidden";
					// requestMsg.version = "";
					// printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
					handleError2(405, server, serverConfig, requestMsg);
					return false;
				}

				return true;
			}
			//std::string requestMethod = methodToString(requestMsg.method);
			//std::transform(requestMethod.begin(), requestMethod.end(), requestMethod.begin(), ::toupper); // mudar para maiusculas
		}


	}


	//std::cout << " ## Root directory END**: " << serverConfig.RootDirectory << std::endl;
	//std::cout << " ## requestMsg.path END**: " << requestMsg.path << std::endl;
	std::string aaa = serverConfig.RootDirectory + requestMsg.path;
	//std::cout << " ## Path directory changed to: " << aaa << std::endl;
	std::string bbb = server.getCompletePath(aaa);
	server.setCompletePath(bbb);
	aaa = ifFileRmoveFile(aaa);
	serverConfig.RootDirectory = aaa;
	//std::cout << " ## ROOT directory changed to: " << aaa << std::endl;
	//std::cout << " ## [FINAL] CompletePath: " << server.getCompletePath2() << std::endl;

	//std::cout << " ## is_directory?: " << !is_directory(aaa) << std::endl;
	if (!is_directory(aaa) && methodToString(requestMsg.method) != "POST")
	{
		//std::cerr << "Root directory does not exist: " << serverConfig.RootDirectory << std::endl;
		//server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
		//requestMsg.path = serverConfig.RootDirectory + " is not found";
		//requestMsg.version = "TESTE-FRED";
		//printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
		handleError2(404, server, serverConfig, requestMsg);
		return false;
	}

	return true;
}

// Process the request and send the response
void processRequest(const std::string& request, ServerInfo& server)
{
	if (request.empty())
		return;

	std::string ParaCGI = request;
	std::string requestCopy = request;
	HTTrequestMSG requestMsg;
	HTTPParser parser;
	size_t maxSize = 100000; // Aumentar o tamanho máximo para 10MB
	if(maxSize > requestCopy.size())
		maxSize = requestCopy.size();

	if (parser.parseRequest(requestCopy, requestMsg, maxSize))
	{
		std::vector<int> ports = server.getPortList();

		if (ports.empty())
		{
			std::cerr << "Error: No ports found." << std::endl;//MUDAR ()
			return;
		}

		int listeningPort = ports[0];
		conf_File_Info &serverConfig = server.getConfig(listeningPort);

		// Salvar o diretório raiz original
		std::string originalRootDirectory = serverConfig.RootDirectory;

		//std::cout << " ##****** Root directory : " << originalRootDirectory << std::endl;

		if (processRulesRequest(requestMsg, server) == true)
		{
			if (!requestMsg.is_cgi) // ======ALTERAÇÂO======
			{
				std::string fileUploadDirectoryCopy = serverConfig.fileUploadDirectory;
				//int portListenCopy = serverConfig.portListen;
				std::string rootDirectoryCopy = serverConfig.RootDirectory;
				//std::cout << RED << "!!!!! config upload: " << fileUploadDirectoryCopy << RESET << std::endl;
				//std::cout << RED << "!!!!! config port: " << portListenCopy << RESET << std::endl;
				//std::cout << RED << "!!!!! config root: " << rootDirectoryCopy << RESET << std::endl;
				handleRequest(requestMsg, server, serverConfig);
			}
			else
			{
				try
				{
					CGI cgi(serverConfig, requestMsg);
					cgi.PerformCGI(server.clientSocket , ParaCGI);
				}
				catch(const std::exception& e)
				{
					std::cerr << e.what() << '\n';
				}
				printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
			}
		}

		// Reverter o diretório raiz para o original após o processamento
		serverConfig.RootDirectory = originalRootDirectory;
	}
	else
	{
		std::cout << RED << "Erro parser da requisição GET --> get.cpp.\n" << RESET;
		std::cout << "Error message: " << requestMsg.error << std::endl;
	}
}

// Checks if file exits
bool fileExists(const std::string& filePath)
{
	struct stat buffer;
	return (stat(filePath.c_str(), &buffer) == 0);
}

// Function to handle the request from the HTTP method
void handleRequest(HTTrequestMSG& request, ServerInfo& server, conf_File_Info &serverConfig)
{
	// int port = server.getPortList()[0];
	// std::string filePath = server.getConfig(port).RootDirectory + request.path;
	// std::cout << "Port HANDLE REQUEST : " << port << std::endl;
	// conf_File_Info &serverConfig = server.getConfig(port);

	if (request.path == "/favicon.ico")
	{
		std::string faviconPath = "resources/website/favicon.ico"; // if the solicitation is for favicon.ico, reads and send the file content
		//std::string faviconPath = serverConfig.RootDirectory + "/favicon.ico";
		std::string fileContent = readFileContent(faviconPath);
		if (!fileContent.empty())
		{
			std::string contentType = "image/x-icon"; // Define the content type based on the file extension
			server.setResponse("HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\n\r\n" + fileContent);
		}
		else
		{
			handleError2(404, server, serverConfig, request);
			//server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
		}
	}
	else
	{
		std::string filePath = "resources/website" + request.path; // adjust this to your actual file path
		//std::string filePath = server.configs[port].RootDirectory + request.path;
		if (request.method == HTTrequestMSG::GET)
		{
			server.handleGetRequest(request, server, serverConfig);
		}
		else if (request.method == HTTrequestMSG::POST)
		{
			server.handlePostRequest(request, server, serverConfig);
		}
		else if (request.method == HTTrequestMSG::DELETE)
		{
			server.handleDeleteRequest(request, server, serverConfig);
		}
		else if (request.method == HTTrequestMSG::UNKNOWN)
		{
			server.handleUnknownRequest(request, server, serverConfig);
		}
	}
}

// Handles unknown requests
void ServerInfo::handleUnknownRequest(HTTrequestMSG& requestMsg, ServerInfo &server, conf_File_Info &serverConfig)
{
	// std::string response = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
	// this->setResponse(response);
	// printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
	handleError2(501, server, serverConfig, requestMsg);
}

std::string readFileContent(const std::string& filePath)
{
	std::ifstream fileStream(filePath.c_str());
	if (!fileStream)
	{
		std::cerr << "File could not be opened: " << filePath << std::endl;
		return "";
	}
	std::string fileContent((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
	return fileContent;
}

// Checks if it is a directory
bool isDirectory(const std::string& path)
{
	struct stat buffer;
	if (stat(path.c_str(), &buffer) == 0)
	{
		return S_ISDIR(buffer.st_mode);
	}
	return false;
}

bool fileExistsInDirectory(const std::string& directory, const std::string& filename)
{

	//std::cout << "[fileExistsInDirectory] DIRECTORY: " << directory << std::endl;
	//std::cout << "[fileExistsInDirectory] FILENAME: " << filename << std::endl;
	std::string fullPath = directory + "/" + filename;
	//std::cout << "[fileExistsInDirectory] FULLPATH: " << fullPath << std::endl;
	if(access(fullPath.c_str(), F_OK) != -1)
		return true;
	else
		return false;
}

void handleError2(int errorCode, ServerInfo& server, conf_File_Info& serverConfig, const HTTrequestMSG& requestMsg)
{
	// Convert error code to string
	std::stringstream ss;
	ss << errorCode;
	std::string errorCodeStr = ss.str();
	std::string originalRootDirectory = server.getRootOriginalDirectory();
	//std::cout << "[error] originalRootDirectory: " << originalRootDirectory << std::endl;
	//std::cout << "[error] serverConfig.RootDirectory1 : " << serverConfig.RootDirectory << std::endl;

	// Resolve absolute path for root directory
	char realPath[PATH_MAX];
	char realPath2[PATH_MAX];
	// if (realpath(serverConfig.RootDirectory.c_str(), realPath))
	// {
	//     serverConfig.RootDirectory = std::string(realPath);
	// }
	// else
	// {
	//     std::cerr << "Error resolving absolute path for: " << serverConfig.RootDirectory << std::endl;
	//     return;
	// }

	realpath(serverConfig.RootDirectory.c_str(), realPath);
	serverConfig.RootDirectory = std::string(realPath);
	//std::cout << "[error] serverConfig.RootDirectory2 : " << serverConfig.RootDirectory << std::endl;

	realpath(originalRootDirectory.c_str(), realPath2);
	originalRootDirectory = std::string(realPath2);
	//std::cout << "[error] originalRootDirectory2 : " << originalRootDirectory << std::endl;

	// Check if the custom error file exists
	std::string errorFilePath2 = originalRootDirectory + "/" + errorCodeStr + ".html";
	ServerErrorHandler errorHandler;
	// Generate the error page content
	std::string errorPageContent = errorHandler.generateErrorPage(errorCode);
	std::string errorMessage = errorHandler.getErrorMessage(errorCode);
	//if (fileExists(errorFilePath))
	if (fileExists(errorFilePath2))
	{
		// If the custom error file exists, serve it
		std::ifstream errorFile(errorFilePath2.c_str());
		std::string errorFileContent;
		std::copy(std::istreambuf_iterator<char>(errorFile), std::istreambuf_iterator<char>(), std::back_inserter(errorFileContent));

		// Get the error message from the map
		server.setResponse("HTTP/1.1 " + errorCodeStr + " " + errorMessage + "\r\nContent-Type: text/html\r\n\r\n" + errorFileContent);
	}
	else
	{
		// If the custom error file does not exist, serve the default error message
		server.setResponse("HTTP/1.1 " + errorCodeStr + " " + errorMessage + "\r\nContent-Type: text/plain\r\n\r\n" + errorMessage + "\nERROR " + errorCodeStr + "\n");
	}

	printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
}


void ServerInfo::handleGetRequest(HTTrequestMSG& requestMsg, ServerInfo& server, conf_File_Info &serverConfig)
{


	//std::string fullPath = "resources/website" + requestMsg.path;
	std::string rootDirectory = serverConfig.RootDirectory;
	// if (!rootDirectory.empty() && rootDirectory[0] == '/') {
	// 	rootDirectory = rootDirectory.substr(1);
	// }
	if (!rootDirectory.empty() && rootDirectory[0] != '/') {
		rootDirectory = '/' + rootDirectory;
	}

	// std::string requestMsgFile = removeTrailingSlash(requestMsg.path);
	// std::string requestMsgFile2 = removeFirstDirectory(requestMsgFile);

	// std::string fullPath = rootDirectory + requestMsgFile2;
	//std::cout << "[handleGetRequest] rootDirectory: " << rootDirectory << std::endl;
	//std::cout << "[handleGetRequest] requestMSG PATH original: " << requestMsg.path << std::endl;
	//std::cout << "[handleGetRequest] requestMSG PATH1: " << requestMsgFile << std::endl;
	//std::cout << "[handleGetRequest] requestMSG PATH2: " << requestMsgFile2 << std::endl;
	//std::cout << "[handleGetRequest] Full path: " << fullPath << std::endl;

	std::string fullPath = getCompletePath2();
	//ifFileRmoveFile(fullPath);
	if (fullPath[fullPath.length() - 1] == '/') {
		fullPath.erase(fullPath.length() - 1);
	}
	//std::cout << "[handleGetRequest] @@ Full path: " << fullPath << std::endl;

	if (!fileExists(fullPath))
	{
		handleError2(404, server, serverConfig, requestMsg);
		return;
	}

	//std::cout << "serverConfig.defaultFile: " << serverConfig.defaultFile << std::endl;
	//std::cout << "serverConfig.directoryListingEnabled: " << serverConfig.directoryListingEnabled << std::endl;
	//std::cout << "fileExistsInDirectory: " << fileExistsInDirectory(fullPath, serverConfig.defaultFile) << std::endl;
	if (isDirectory(fullPath) && serverConfig.directoryListingEnabled == true && fileExistsInDirectory(fullPath, serverConfig.defaultFile) == false)
	{
		handleDirectoryListing(serverConfig, requestMsg, server);
		//std::cout << "ENTREI NO TESTE" << std::endl;
		server.getResponse();
		//std::cout << "FFFFFFF: " << response << std::endl;
		printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
		return;
	}

	struct stat buffer;
	if (stat(fullPath.c_str(), &buffer) == 0)
	{
		if (S_ISREG(buffer.st_mode))
		{
			std::string fileContent = readFileContent(fullPath);
			if (fileContent.empty())
			{
				std::cerr << "[DEBUG] File content is empty or could not be read: " << fullPath << std::endl;
				//server.setResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\nInternal server error\n");
				//printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
				handleError2(500, server, serverConfig, requestMsg);
				return;
			}
			std::string contentType = getContentType(fullPath);
			server.setResponse("HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\n\r\n" + fileContent);
		}
		else if (S_ISDIR(buffer.st_mode))
		{
			if (!requestMsg.path.empty() && requestMsg.path[requestMsg.path.length() - 1] != '/')
			{
				server.setResponse("HTTP/1.1 301 Moved Permanently\r\nLocation: " + requestMsg.path + "/\r\n\r\n");
				printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
				return;
			}

			std::string indexPath = fullPath;
			if (indexPath[indexPath.length() - 1] != '/')
				indexPath += '/';
			indexPath += serverConfig.defaultFile;
			//std::cout << "indexPath: " << indexPath << std::endl;
			//std::cout << "serverConfig.defaultFile: " << serverConfig.defaultFile << std::endl;
			//indexPath += "index.html";

			if (stat(indexPath.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode))
			{
				std::string fileContent = readFileContent(indexPath);
				if (fileContent.empty())
				{
					std::cerr << "[DEBUG] Index file content is empty or could not be read: " << indexPath << std::endl;
					//server.setResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\nInternal server error\n");
					//printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
					handleError2(500, server, serverConfig, requestMsg);
					return;
				}
				std::string contentType = getContentType(indexPath);
				server.setResponse("HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\n\r\n" + fileContent);
			}
			else
			{
				std::cerr << "[DEBUG] Index file not found or is not a regular file: " << indexPath << std::endl;
				//server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
				//printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
				handleError2(404, server, serverConfig, requestMsg);
				return;
			}
		}
	}
	else
	{
		//std::cerr << "[DEBUG] Error retrieving file stats: " << fullPath << std::endl;
		handleError2(500, server, serverConfig, requestMsg);
		//server.setResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\nInternal server error\n");
	}
	printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
}

std::string extractFileNameFromURL(const std::string& url)
{
	std::string fileName;
	std::istringstream iss(url);
	std::string token;

	// Procura pelo token 'file=' na URL
	while (std::getline(iss, token, '=')) {
		if (token == "file") {
			// Se encontrar 'file=', extrai o próximo token como o nome do arquivo
			if (std::getline(iss, fileName, '&')) {
				// Remove os caracteres de escape '%20' substituindo-os por espaços
				size_t pos;
				while ((pos = fileName.find("%20")) != std::string::npos) {
					fileName.replace(pos, 3, " ");
				}
				break;
			}
		}
	}

	return fileName;
}

//void ServerInfo::handleDeleteRequest(HTTrequestMSG& requestMsg, ServerInfo& server)
void ServerInfo::handleDeleteRequest(HTTrequestMSG& requestMsg, ServerInfo& server, conf_File_Info &serverConfig)
{
	//int port = server.portListen[0];
	//std::cout << "Port NO DELETE : " << port << std::endl;
	//conf_File_Info &serverConfig = server.getConfig(port);

	try
	{
		if(server.portListen.empty())
		{
			std::cerr << "No ports available." << std::endl;
			return;
		}

		if(serverConfig.fileUploadDirectory.empty())
		{
			// std::cout << "======>>>> File upload directory not set in the configuration file." << std::endl;
			// setResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\n\r\nFile upload directory not set.");
			// printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
			handleError2(500, server, serverConfig, requestMsg);
			return;
		}

		if (requestMsg.method != HTTrequestMSG::DELETE) {
			// setResponse("HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/html\r\n\r\nMethod not allowed.");
			// printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
			handleError2(405, server, serverConfig, requestMsg);
			return;
		}

		// Extract file name from query string
		std::string fileName = extractFileNameFromURL(requestMsg.query);

		//std::cout << "Request pathAAA: " << requestMsg.path << std::endl;
		//std::cout << "Query string: " << requestMsg.query << std::endl;
		//std::cout << "File nameAAA: " << fileName << std::endl;

		if (fileName.empty()) {
			// setResponse("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\nNo file specified.");
			// printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
			handleError2(400, server, serverConfig, requestMsg);
			return;
		}

		// Validate the file name format
		if (fileName.find("..") != std::string::npos || fileName.find("/") != std::string::npos) {
			// setResponse("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\nInvalid file name.");
			// printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
			handleError2(400, server, serverConfig, requestMsg);
			return;
		}

		// ----------------------//
		// std::string rootDirectory1 = server.configs[port].RootDirectory;
		// std::cout << "RootDirectory: " << rootDirectory1 << std::endl;
		// std::string fileUploadDirectory = server.configs[port].fileUploadDirectory;
		// std::cout << "FileUploadDirectory: " << fileUploadDirectory << std::endl;
		// std::string dataDirectory = rootDirectory1 + fileUploadDirectory + "/";
		std::string dataDirectory = "uploads/";
		//-------------------------//

	//	std::cout << "Data directory: " << dataDirectory << std::endl;
		server.getCompletePath2();
		//std::cout << "Complete path: " << server.getCompletePath2() << std::endl;
		std::string filePath = dataDirectory + fileName;
		//std::cout << "File path: " << filePath << std::endl;

		// Check if the file path is within the data directory
		if (filePath.substr(0, dataDirectory.size()) != dataDirectory) {
			// setResponse("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\nInvalid file name.");
			// printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
			handleError2(400, server, serverConfig, requestMsg);
			return;
		}

		// Check if the file exists and is accessible
		if (access(filePath.c_str(), F_OK) != -1)
		{
			// The file exists and is accessible, try to delete it
			if (remove(filePath.c_str()) == 0)
			{
				setResponse("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nFile deleted successfully.");
				printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
				return ;
			}
			else
			{
				//setResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\n\r\nError deleting file.");
				handleError2(501, server, serverConfig, requestMsg);
				return ;
			}

		}
		else
		{
			// The file does not exist or is not accessible
			//setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\nFile not found.");
			//printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
			handleError2(404, server, serverConfig, requestMsg);
			return ;
		}
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
		//setResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\n\r\nInternal Server Error.");
		//printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
		handleError2(500, server, serverConfig, requestMsg);
		return ;
	}
}


void ServerInfo::handlePostRequest(HTTrequestMSG& request, ServerInfo &server, conf_File_Info &serverConfig)
{
	std::string contentLengthStr = request.headers["Content-Length"];
	if (contentLengthStr.empty())
	{
		//this->setResponse("HTTP/1.1 411 Length Required\r\nContent-Type: text/plain\r\n\r\nError: Content-Length header is missing");
		handleError2(411, server, serverConfig, request);
		return;
	}

	size_t contentLength = atoi(contentLengthStr.c_str());

	if (request.body.size() != contentLength) // Verificando se o tamanho do corpo corresponde ao Content-Length
	{
		//this->setResponse("HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nError: Request body size does not match Content-Length");
		handleError2(411, server, serverConfig, request);
		return;
	}

	std::string body = request.body; // Parse the request body

	if (body.empty()) // Check if the body is empty
	{
		//this->setResponse("HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nError: Request body is empty");
		handleError2(400, server, serverConfig, request);
		return;
	}

	std::string response; // Process the data
	std::string delimiter = "&";
	size_t pos = 0;
	std::string token;

	// Parse the body and construct the response
	while ((pos = body.find(delimiter)) != std::string::npos)
	{
		token = body.substr(0, pos);
		response += token + "\n";
		body.erase(0, pos + delimiter.length());
	}
	response += body; // Add the last token

	if (response.empty()) // Check if the response is empty
	{
		std::cerr << "Error: Response is empty" << std::endl;
	}

	// Create HTTP response with HTML content
	std::vector<int> portList = server.getPortList();
	int port = portList[0];
	std::string serverAddress = "127.0.0.1";
	std::stringstream ss;
	ss << port;
	std::string portStr = ss.str();

	std::string httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
	httpResponse += "<html><head><style>body { background: #ADD8E6; }</style></head><body>";
	httpResponse += "<p>Received POST data:</p><pre><b>" + response + "</b></pre>";
	httpResponse += "<button onclick=\"location.href='http://" + serverAddress + ":" + portStr + "'\" type=\"button\">Go Home</button>";
	httpResponse += "</body></html>\n";

	// Set the response
	this->setResponse(httpResponse);

	// Log the request details
	printLog(methodToString(request.method), request.path, request.version, server.getResponse(), server);
}

void setupRunServer(std::vector<ServerInfo*>& servers, fd_set& read_fds, fd_set& write_fds, int& max_fd)
{

	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);

	max_fd = -1;
	for (std::vector<ServerInfo*>::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		int sockfd = (*it)->getSocketFD();
		FD_SET(sockfd, &read_fds);
		if (sockfd > max_fd)
			max_fd = sockfd;
	}

	std::cout << "\n<" << GREEN << "=+=+=+=+=+=+=+=+=+=" << RESET << " Waiting for client "
	<< GREEN << "=+=+=+=+=+=+=+=+=+=" << RESET << ">\n" << std::endl;

	time_t now = time(NULL);
	char timestamp[100];
	strftime(timestamp, sizeof(timestamp), "[%d/%b/%Y %T]", localtime(&now));

	for (std::vector<ServerInfo*>::iterator it = servers.begin(); it != servers.end(); ++it)
	{

			std::vector<int> ports = (*it)->getPortList();
			for (std::vector<int>::iterator portIt = ports.begin(); portIt != ports.end(); ++portIt)
				std::cout << BG_CYAN_BLACK << timestamp << RESET << " Listening on http://127.0.0.1:" << CYAN << *portIt << RESET;
			std::cout << std::endl;

	}
}



void runServer(std::vector<ServerInfo*>& servers, fd_set read_fds, fd_set write_fds, int max_fd)
{
	int newsockfd = -1;
	std::list<int> socketsToClose;
	bool cleanupAndExit = false;

	while (!flag && !cleanupAndExit)
	{
		fd_set temp_read_fds = read_fds;
		fd_set temp_write_fds = write_fds;
		if (select(max_fd + 1, &temp_read_fds, &temp_write_fds, NULL, NULL) < 0)
		{
			if (errno == EINTR)
				continue;
			perror("Error on select");
			servers[0]->sair = 1;
			return;
		}

		for (std::vector<ServerInfo*>::iterator it = servers.begin(); it != servers.end(); ++it)
		{
			int sockfd = (*it)->getSocketFD();
			if (FD_ISSET(sockfd, &temp_read_fds))
			{
				sockaddr_in cli_addr;
				socklen_t clilen = sizeof(cli_addr);
				newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
				if (newsockfd < 0)
				{
					perror("Error on accept");
					for (std::vector<ServerInfo*>::iterator server = servers.begin(); server != servers.end(); ++server)
					{
						if ((*server)->clientSocket >= 0)
						{
							FD_CLR((*server)->clientSocket, &read_fds);
							FD_CLR((*server)->clientSocket, &write_fds);
							close((*server)->clientSocket);
							(*server)->clientSocket = -1;
						}
					}
					(*it)->sair = 1;
					return;
				}
				std::string request = readRequest(newsockfd, **it);
				(*it)->clientSocket = newsockfd;
				processRequest(request, **it);

				FD_SET(newsockfd, &write_fds);
				if (newsockfd > max_fd)
				{
					max_fd = newsockfd;
				}
			}

			if ((*it)->clientSocket >= 0 && FD_ISSET((*it)->clientSocket, &temp_write_fds))
			{
				int clientSocket = (*it)->clientSocket;
				if (write(clientSocket, (*it)->getResponse().c_str(), (*it)->getResponse().length()) < 0)
					std::cerr << "Error writing to socket" << std::endl;
				FD_CLR(clientSocket, &read_fds);
				FD_CLR(clientSocket, &write_fds);
				socketsToClose.push_back(clientSocket);
				clientSocket = -1;
			}
		}

		for (std::list<int>::iterator it = socketsToClose.begin(); it != socketsToClose.end(); ++it)
			close(*it);
		socketsToClose.clear();
	}

	// Cleanup section
	for (std::vector<ServerInfo*>::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		if ((*it)->clientSocket >= 0) 
		{
			FD_CLR((*it)->clientSocket, &read_fds);
			FD_CLR((*it)->clientSocket, &write_fds);
			close((*it)->clientSocket);
			(*it)->clientSocket = -1;
		}
		if (newsockfd != -1)
			close(newsockfd);

		int sockfd = (*it)->getSocketFD();
		FD_CLR(sockfd, &read_fds);
		FD_CLR(sockfd, &write_fds);
		close(sockfd);
	}
	if (newsockfd != -1)
	{
		close(newsockfd);
		newsockfd = -1;
	}
}


bool ends_with(const std::string& value, const std::string& ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::string getContentType(const std::string& filePath)
{
	std::string contentType;
	if (ends_with(filePath, ".html"))
		contentType = "text/html";
	else if (ends_with(filePath, ".css"))
		contentType = "text/css";
	else if (ends_with(filePath, ".js"))
		contentType = "application/javascript";
	else // Default to text/plain for unknown file types
		contentType = "text/plain";
	return contentType;
}


// void runServer(std::vector<ServerInfo*>& servers, fd_set read_fds, fd_set write_fds, int max_fd)
// {
// 	int newsockfd = -1;
// 	std::list<int> socketsToClose;
// 	while (!flag)
// 	{
// 		fd_set temp_read_fds = read_fds;
// 		fd_set temp_write_fds = write_fds;
// 		if (select(max_fd + 1, &temp_read_fds, &temp_write_fds, NULL, NULL) < 0)
// 		{
// 			if (errno == EINTR)
// 			{
// 				continue; // Continue o loop se a chamada select() foi interrompida por um sinal
// 			}
// 			perror("Error on select");
// 			servers[0]->sair = 1;
// 			return ;
// 			//exit(EXIT_FAILURE);
// 		}

// 		for (std::vector<ServerInfo*>::iterator it = servers.begin(); it != servers.end(); ++it)
// 		{
// 			int sockfd = (*it)->getSocketFD();
// 			if (FD_ISSET(sockfd, &temp_read_fds))
// 			{
// 				sockaddr_in cli_addr;
// 				socklen_t clilen = sizeof(cli_addr);
// 				newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
// 				if (newsockfd < 0)
// 				{
// 					(*it)->sair = 1;
// 					perror("Error on accept");
// 					return ;
// 					//exit(EXIT_FAILURE);
// 				}
// 				std::string request = readRequest(newsockfd, **it);
// 				(*it)->clientSocket = newsockfd;
// 				processRequest(request, **it);

// 				// Add new socket to write_fds
// 				FD_SET(newsockfd, &write_fds);
// 				if (newsockfd > max_fd)
// 					max_fd = newsockfd;
// 			}

// 			if (it != servers.end() && (*it)->clientSocket >= 0 && FD_ISSET((*it)->clientSocket, &temp_write_fds))
// 			{
// 				// Write response to the client
// 				int clientSocket = (*it)->clientSocket;

// 				write(clientSocket, (*it)->getResponse().c_str(), (*it)->getResponse().length());
// 				// Remove client socket from read_fds and write_fds
// 				FD_CLR(clientSocket, &read_fds);
// 				FD_CLR(clientSocket, &write_fds);
// 				socketsToClose.push_back(clientSocket);
// 				(*it)->clientSocket = -1;

// 				if (newsockfd >= 0) {
// 					close(newsockfd);
// 					newsockfd = -1;
// 				}
// 			}

// 		}
// 		// std::cout << "sair[0]: " << servers[0]->sair << std::endl;
// 		// std::cout << "sair[1]: " << servers[1]->sair << std::endl;
// 		// std::cout << "sair[2]: " << servers[2]->sair << std::endl;
// 		for (std::list<int>::iterator it = socketsToClose.begin(); it != socketsToClose.end(); ++it)
// 		{
// 			close(*it);
// 		}
// 		socketsToClose.clear();
// 	}
// 	// Cleanup section
// 	for (std::vector<ServerInfo*>::iterator it = servers.begin(); it != servers.end(); ++it)
// 	{
// 		if ((*it)->clientSocket >= 0)
// 		{
// 			FD_CLR((*it)->clientSocket, &read_fds);
// 			FD_CLR((*it)->clientSocket, &write_fds);
// 			close((*it)->clientSocket);
// 			(*it)->clientSocket = -1;
// 		}
// 		if (newsockfd != -1)
// 			close(newsockfd);

// 		int sockfd = (*it)->getSocketFD();
// 		FD_CLR(sockfd, &read_fds);
// 		FD_CLR(sockfd, &write_fds);
// 		close(sockfd);
// 	}
// }