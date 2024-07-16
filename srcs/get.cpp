/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 21:07:55 by fde-carv          #+#    #+#             */
/*   Updated: 2024/07/16 14:27:32 by brolivei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/get.hpp"

// std::vector<std::string> createdFiles;
// volatile sig_atomic_t flag = 0;
// std::map<int, std::map<std::string, ParserConfig> > serversByPortAndHost;

ServerInfo::ServerInfo()
{
	this->sockfd = -1;
	memset(&serv_addr, 0, sizeof(serv_addr));
	this->clientSocket = -1;
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
	this->check_file = 0;
}

ServerInfo::~ServerInfo()
{
	for (std::vector<int>::iterator it = clientSockets.begin(); it != clientSockets.end(); ++it)
	{
		if (*it != -1)
			close(*it);
	}
	clientSockets.clear();

	if (sockfd >= 0)
	{
		close(sockfd);
		sockfd = -1;
	}

	portListen.clear();

	cli_addrs.clear();
	response.clear();
	rootUrl.clear();
	rootOriginalDirectory.clear();
	configs.clear();
	complete_path.clear();

	contentLength = 0;
	memset(&serv_addr, 0, sizeof(serv_addr));
}

void ServerInfo::setSocketFD(int socket)
{
	sockfd = socket;
}

int ServerInfo::getSocketFD() const
{
	return (sockfd);
}

void ServerInfo::setAddress(const sockaddr_in &address)
{
	serv_addr = address;
}

void ServerInfo::setResponse(const std::string &response)
{
	this->response = response;
}

std::string ServerInfo::getResponse() const
{
	return (response);
}

std::string ServerInfo::getRootUrl() const
{
	return rootUrl;
}

void ServerInfo::addSocketToList(int sockfd)
{
	clientSockets.push_back(sockfd);
}

void ServerInfo::addPortToList(int port)
{
	portListen.push_back(port);
}

std::vector<int> ServerInfo::getPortList() const
{
	return portListen;
}

std::vector<int> &ServerInfo::getSockets()
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

void ServerInfo::addConfig(int port, const conf_File_Info &config)
{
	configs[port] = config;
}

conf_File_Info &ServerInfo::getConfig(int port)
{
	return configs[port];
}

void ServerInfo::cleanup2()
{
	for (std::vector<int>::iterator it = clientSockets.begin(); it != clientSockets.end(); ++it)
	{
		close(*it);
	}
	clientSockets.clear();

	if (sockfd != -1)
	{
		close(sockfd);
		sockfd = -1;
	}

	cli_addrs.clear();
	response.clear();
	rootUrl.clear();
	portListen.clear();
	rootOriginalDirectory.clear();
	configs.clear();
	complete_path.clear();
}

void ServerInfo::setCompletePath(const std::string &path)
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

void ServerInfo::setRootOriginalDirectory(const std::string &dir)
{
	rootOriginalDirectory = dir;
}

// Takes a relative path ou transformes in absolute path
std::string ServerInfo::getCompletePath(const std::string &path)
{
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	std::string full_path2 = std::string(cwd);
	if (!path.empty() && path[0] != '/')
		full_path2 += '/';

	full_path2 += path;
	return full_path2;
}

int ServerInfo::getCheckFile()
{
	return check_file;
}

void ServerInfo::setCheckFile(int value)
{
	check_file = value;
}

// ================================================================================================= //
// ======================================= HELPER FUNCTIONS ======================================== //
// ================================================================================================= //

// Convert the method (int) to a string
std::string methodToString(HTTrequestMSG::Method method)
{
	switch (method)
	{
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
void printLog(const std::string &method, const std::string &path, const std::string &version, const std::string &httpResponse, ServerInfo &server)
{
	static int requestCount = -1;
	requestCount++;

	time_t now = time(NULL);
	char timestamp[100];
	strftime(timestamp, sizeof(timestamp), "[%d/%b/%Y %T]", localtime(&now));

	std::string methodColor = (method == "GET") ? YELLOW : (method == "POST") ? CYAN
																			  : MAGENTA;

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
		std::cout << BG_CYAN_BLACK << timestamp << RESET << GREEN << " [" << RESET << requestCount << GREEN << "] " << BLUE << "Connected with client at 127.0.0.1:" << CYAN << currentPort << RESET << std::endl;
	}

	std::cout << BG_CYAN_BLACK << timestamp << RESET << BLUE << " [" << RESET << requestCount << BLUE << "]"
			  << RESET << methodColor << " \"" << method << " " << path << " ";
	if (!version.empty())
		std::cout << version;
	std::cout << "\" " << RESET << statusColor << statusCode << RESET << " ";
	if (method == "GET")
		std::cout << server.getResponse().length();
	else if (method == "POST")
		std::cout << server.getContentLength();

	std::cout << "\n"
			  << BG_CYAN_BLACK << timestamp << RESET << RED << " [" << RESET << requestCount << RED << "] "
			  << BLUE << "Connection ended successfully" << RESET << std::endl;
}

// Function to handle errors without exiting the program
void ServerInfo::handleError(const std::string &errorMessage) //, int errorCode)
{
	std::cerr << RED1 << "\n"
			  << errorMessage << RESET << std::endl;
	cleanup2();
	// exit(-1);
}

void printError(const std::string &errorMessage)
{
	std::cerr << RED1 << "\n"
			  << errorMessage << RESET << std::endl;

	// exit(-1);
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
	DIR *dir = opendir(modifiedPath.c_str());
	if (dir)
	{
		closedir(dir);
		return true;
	}
	else
	{
		return false;
	}
}

// Setup the server
void setupServer(ServerInfo &server, const conf_File_Info &config)
{
	int sockfd = -1;
	try
	{
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
		{
			throw std::runtime_error("Error on socket creation");
		}
		server.addSocketToList(sockfd);
		server.setSocketFD(sockfd);
		sockaddr_in serv_addr;
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(config.portListen);
		server.setAddress(serv_addr);

		int opt = 1;
		if (setsockopt(server.getSocketFD(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		{
			throw std::runtime_error("Error on setsockopt");
		}

		if (bind(server.getSocketFD(), (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		{
			throw std::runtime_error("Error on binding");
			std::cout << RED << "Error on binding" << RESET << std::endl;
			server.sair = 1;
			return;
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

		if (serverRoot != actualRoot)
			throw std::runtime_error("404 Not Found: The requested server root does not match the actual server root.");

		if (serverRoot[serverRoot.size() - 1] != '/')
			serverRoot += "/";
	}
	catch (const std::runtime_error &e)
	{
		if (sockfd != -1)
			close(sockfd);
		std::cerr << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
}

// Read the request from the client and return it as a string
std::string readRequest(int sockfd, ServerInfo &server)
{
	CR ChunkedOBJ = CR(sockfd);
	char buffer[4096];
	std::string request;

	// Read the header
	while (1)
	{
		memset(buffer, 0, 4096);
		ssize_t bytesRead = recv(sockfd, buffer, 4095, 0);
		if (bytesRead < 0)
		{
			server.handleError("recv");
			server.handleError("Error reading from socket.");
			server.sair = 1;
			return request;
			close(sockfd);
		}
		else if (bytesRead == 0)
			break;
		else
			buffer[bytesRead] = '\0';

		request.append(buffer, bytesRead);
		if (request.find("\r\n\r\n") != std::string::npos)
			break;
	}

	if (ChunkedOBJ.ItIsChunked(request) == true)
	{
		//std::cout << "Chunked Request detected HERE\n";
		std::string requestCR = ChunkedOBJ.HandleRequest(request);
		return requestCR;
	}

	// Read the Body
	HTTPParser parser;
	size_t contentLength = parser.getContentLength(request);
	server.setContentLength(contentLength);
	size_t actualDataSize = request.size();
	size_t headerSize = request.find("\r\n\r\n") + 4;
	if (contentLength > actualDataSize - headerSize)
	{
		size_t bytesReadTotal = actualDataSize - headerSize;
		while (bytesReadTotal < contentLength)
		{
			memset(buffer, 0, 4096);
			ssize_t bytesRead = recv(sockfd, buffer, std::min(static_cast<size_t>(4095), contentLength - bytesReadTotal), 0);

			if (bytesRead < 0)
			{
				server.handleError("Error reading from socket.");
				break;
			}
			else if (bytesRead == 0)
			{
				server.handleError("Socket has been closed by the other end.");
				break;
			}
			else
				buffer[bytesRead] = '\0';

			if (bytesReadTotal > request.size() - headerSize)
			{
				std::cerr << "Read beyond the end of available data." << std::endl;
				break;
			}
			request.append(buffer, bytesRead);
			bytesReadTotal += bytesRead;
		}
	}
	return request;
}

// To check tokens size for determinate root directory
std::vector<std::string> tokenize(const std::string &str, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(str);

	while (std::getline(tokenStream, token, delimiter))
	{
		if (!token.empty())
			tokens.push_back(token);
	}
	return tokens;
}

// Make a new relative path from the root and the path of locations
std::string getNewPath(const std::string &root, const std::string &path)
{
	std::vector<std::string> rootTokens = tokenize(root, '/');
	std::vector<std::string> pathTokens = tokenize(path, '/');

	std::string newPath;
	std::string initialPath;
	if (pathTokens.empty())
		return root;
	else
	{
		size_t i = 0, j = 0;
		bool hasMatched = false;

		while (i < rootTokens.size() && j < pathTokens.size())
		{
			if (rootTokens[i] == pathTokens[j])
			{
				hasMatched = true;
				newPath += "/" + rootTokens[i];
				++j;
			}
			else if (hasMatched)
				break;
			else
				initialPath += "/" + rootTokens[i];
			++i;
		}
	}
	return initialPath + newPath;
}

// Take de first directory from a path
std::string getDirectoryPath(const std::string &fullPath)
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
bool isMethodAllowed(const std::set<std::string> &allowedMethods, const std::string &requestMethod)
{
	if (allowedMethods.empty())
		return true;
	std::string upperRequestMethod = requestMethod;
	for (std::set<std::string>::const_iterator it_meth = allowedMethods.begin(); it_meth != allowedMethods.end(); ++it_meth)
	{
		std::string allowedMethods = *it_meth;
		std::string allowedMethod = *it_meth;
		std::transform(allowedMethod.begin(), allowedMethod.end(), allowedMethod.begin(), ::toupper);
		if (allowedMethod == upperRequestMethod)
			return true;
	}
	return false;
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
	return path;
}

std::string removeLastSlash(const std::string &fullPath)
{
	if (fullPath == "/")
		return fullPath;
	if (!fullPath.empty() && fullPath[fullPath.size() - 1] == '/')
		return fullPath.substr(0, fullPath.size() - 1);
	return fullPath;
}

// Checks if AUTOINDEX is enabled and handles the directory listing
bool handleDirectoryListing(conf_File_Info &serverConfig, HTTrequestMSG &requestMsg, ServerInfo &server)
{
	(void)requestMsg;
	if (serverConfig.directoryListingEnabled)
	{
		std::string rootDirectory = serverConfig.RootDirectory;
		if (!rootDirectory.empty() && rootDirectory[0] == '/')
			rootDirectory = rootDirectory.substr(1);

		std::string fullPath = server.getCompletePath2();

		if (fullPath.at(0) != '/')
			fullPath = "/" + fullPath;

		std::string full_path1 = fullPath;

		std::string full_path2;
		if (full_path1.substr(0, 5) == "/home")
			full_path2 = full_path1;
		else
		{
			char cwd[1024];
			getcwd(cwd, sizeof(cwd));
			full_path2 = std::string(cwd) + full_path1;
		}

		struct stat path_stat;
		if (stat(full_path2.c_str(), &path_stat) != 0)
		{
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
			std::string serverAddress = "127.0.0.1";
			std::stringstream ss;
			ss << port;
			std::string portStr = ss.str();
			if ((dir = opendir(full_path2.c_str())) != NULL)
			{

				response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
				response += "<html><head><style>body { background: #ADD8E6; }</style></head><body>";

				std::vector<std::string> entries;
				while ((ent = readdir(dir)) != NULL)
				{
					entries.push_back(ent->d_name);
				}
				std::sort(entries.begin(), entries.end());

				for (size_t i = 0; i < entries.size(); i++)
				{
					response += entries[i];
					response += "<br>";
				}

				response += "<br>";
				response += "<button onclick=\"location.href='http://" + serverAddress + ":" + portStr + "'\" type=\"button\">HOME</button>";
				response += "</body></html>\n";
				closedir(dir);
			}
			else
			{
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
			}
		}
	}
	return true;
}

void processErrorPage(std::string second, int errorCode, const std::string &rootDirectory, ServerInfo &server)
{

	size_t pos = second.find('/');

    std::string nameAfterSlash;
    if (pos != std::string::npos)
    {
        nameAfterSlash = second.substr(pos + 1);
    }
	//std::cout << "nameAfterSlash :" << nameAfterSlash << std::endl;


	if (pos != std::string::npos)
		second.erase(pos, 1);

	if (second.size() >= 3)
	{
		std::string threeDigits = second.substr(0, 3);

		std::stringstream ss;
		ss << errorCode;
		std::string errorCodeStr = ss.str();

		if (threeDigits[0] == errorCodeStr[0])
		{
			threeDigits[1] = errorCodeStr[1];
			threeDigits[2] = errorCodeStr[2];
		}

		server.nameAfterSlashSets[threeDigits] = nameAfterSlash;
		server.threeDigitsSet.insert(threeDigits);
		if (std::atoi(threeDigits.c_str()) == errorCode)
		{
			std::string path = rootDirectory + "/" + nameAfterSlash;
			std::ifstream existingFile(path.c_str());
			if (!existingFile)
			{
				std::ofstream file(path.c_str());
				if (file)
				{
					ServerErrorHandler errorHandler;
					std::string firstThreeChars = nameAfterSlash.substr(0, 3);
					int digitCount = 0;
					for (size_t i = 0; i < firstThreeChars.length() && i < 3; ++i)
					{
						if (firstThreeChars[i] >= '0' && firstThreeChars[i] <= '9')
						{
							++digitCount;
						}
					}
					for (std::map<std::string, std::string>::const_iterator it = server.nameAfterSlashSets.begin(); it != server.nameAfterSlashSets.end(); ++it)
					{
						if (digitCount == 3)
						{
							std::string errorPageContent = errorHandler.generateErrorPage(errorCode);
							file << errorPageContent;
							file.close();
						}
						else
						{
							std::string errorPageContent = errorHandler.BgenerateErrorPage_2(it->second);
							file << errorPageContent;
							file.close();
						}
					}
				}
			}
		}
	}
}

void createHtmlFiles(const std::string &rootDirectory)
{
	ServerErrorHandler handler;
	std::string filenames[] = {"delete.html", "get.html", "post.html", "upload.html"};

	for (int i = 0; i < 4; ++i)
	{
		std::string path = rootDirectory + "/" + filenames[i];

		std::ifstream existingFile(path.c_str());
		if (!existingFile)
		{
			std::ofstream file(path.c_str());
			if (file)
			{
				std::string htmlContent;
				switch (i)
				{
				case 0:
					htmlContent = handler.generateDelete();
					break;
				case 1:
					htmlContent = handler.generateGet();
					break;
				case 2:
					htmlContent = handler.generatePost();
					break;
				case 3:
					htmlContent = handler.generateUpload();
					break;
				}
				file << htmlContent;
				file.close();
				createdFiles.push_back(path);
			}
		}
	}
}

// void createHtmlFiles(const std::string& rootDirectory)
// {
// 	ServerErrorHandler handler;
// 	std::string path = rootDirectory + "/" + "delete.html";
// 	std::ofstream file(path.c_str());
// 	if (file)
// 	{
// 		std::string htmlContent = handler.generateDelete();
// 		file << htmlContent;
// 		file.close();
// 		createdFiles.push_back(path);
// 	}
// 	path = rootDirectory + "/" + "get.html";
// 	std::ofstream file2(path.c_str());
// 	if (file2)
// 	{
// 		std::string htmlContent = handler.generateGet();
// 		file2 << htmlContent;
// 		file2.close();
// 		createdFiles.push_back(path);
// 	}
// 	path = rootDirectory + "/" + "post.html";
// 	std::ofstream file3(path.c_str());
// 	if (file3)
// 	{
// 		std::string htmlContent = handler.generatePost();
// 		file3 << htmlContent;
// 		file3.close();
// 		createdFiles.push_back(path);
// 	}
// 	path = rootDirectory + "/" + "upload.html";
// 	std::ofstream file4(path.c_str());
// 	if (file4)
// 	{
// 		std::string htmlContent = handler.generateUpload();
// 		file4 << htmlContent;
// 		file4.close();
// 		createdFiles.push_back(path);
// 	}
// }

// void createIndexFile(conf_File_Info &serverConfig, const std::string &rootDirectory)
// {
// 	std::string name;
// 	if (serverConfig.defaultFile.empty())
// 		name = "index.html";
// 	else
// 		name = serverConfig.defaultFile;
// 	std::string path = rootDirectory + "/" + name;
// 	std::ifstream existingFile(path.c_str());
// 	if (!existingFile)
// 	{
// 		std::ofstream file(path.c_str());
// 		if (file)
// 		{
// 			ServerErrorHandler handler;
// 			std::string base = handler.generateIndex(name);
// 			file << base;
// 			file.close();
// 			createdFiles.push_back(path);
// 		}
// 	}
// }

// Principal Function to deal with rules from .conf file
bool processRulesRequest(HTTrequestMSG &requestMsg, ServerInfo &server)
{
	std::vector<int> ports = server.getPortList();
	int listeningPort = ports[0];
	conf_File_Info &serverConfig = server.getConfig(listeningPort);

	server.setRootOriginalDirectory(serverConfig.RootDirectory);
	std::string BALA = requestMsg.path;
	std::string browserRelativePath = removeLastSlash(requestMsg.path);

	std::map<int, std::string> errorMap;
	for (std::map<int, std::string>::const_iterator it = serverConfig.errorMap.begin(); it != serverConfig.errorMap.end(); ++it)
	{
		if (it != serverConfig.errorMap.end())
			errorMap[it->first] = it->second;
		processErrorPage(it->second, it->first, serverConfig.RootDirectory, server);
	}

	//createIndexFile(serverConfig, serverConfig.RootDirectory);
	//createHtmlFiles(serverConfig.RootDirectory);

	if (serverConfig.LocationsMap.size() > 0)
	{
		for (Locations::const_iterator it = serverConfig.LocationsMap.begin(); it != serverConfig.LocationsMap.end(); ++it)
		{
			if (it->first == browserRelativePath)
			{
				int bodySizeBytes = server.getContentLength();
				bodySizeBytes -= 200;
				if (bodySizeBytes > serverConfig.maxRequestSize)
				{
					handleError2(413, server, serverConfig, requestMsg);
					return false;
				}
				if (it->first == "/")
				{
					std::string newMixedPath = getNewPath(serverConfig.RootDirectory, it->second.RootDirectory);
					std::string completeNewMixedPath = server.getCompletePath(newMixedPath);
					std::string completeFullPath = completeNewMixedPath + requestMsg.path;
					server.setCompletePath(completeFullPath);
					std::string requestMethod = methodToString(requestMsg.method);
					bool methodAllowed = isMethodAllowed(it->second.allowedMethods, requestMethod);
					if (!methodAllowed)
					{
						//std::cout << "aqui" << std::endl;
						handleError2(405, server, serverConfig, requestMsg); // ALTERACAO ANTES ESTAVA ERRO 403
						return false;
					}
					return true;
				}
				if (it->first == "/redirect")
				{
					std::string newUrl = serverConfig.redirectURL.destinationURL;
					std::stringstream ss;
					ss << serverConfig.redirectURL.httpStatusCode;
					std::string response = requestMsg.version + " " +
										   ss.str() + " " +
										   "Moved Permanently\r\n"
										   "Location: " +
										   newUrl + "\r\n"
													"\r\n";
					// std::cout << "RESPONSE: " << response << std::endl;
					server.setResponse(response);
					printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
					return false;
				}
				std::string requestMethod = methodToString(requestMsg.method);
				bool methodAllowed = isMethodAllowed(it->second.allowedMethods, requestMethod);
				if (!methodAllowed)
				{
					handleError2(405, server, serverConfig, requestMsg);
					return false;
				}


				std::string newMixedPath = getNewPath(serverConfig.RootDirectory, it->second.RootDirectory);
				std::string completeNewMixedPath = server.getCompletePath(newMixedPath);
				std::string completeFullPath = completeNewMixedPath + requestMsg.path;

				serverConfig.RootDirectory = newMixedPath;
				server.setCompletePath(completeFullPath);

				if (completeFullPath.rfind('/') != completeFullPath.length() - 1)
					completeFullPath += '/';
				std::string tryFilePath = completeFullPath + it->second.tryFile;
				//std::cout << "TRYFILEPATH: " << tryFilePath << std::endl;
				struct stat buffer;
				if (is_directory(completeFullPath))
				{
					if (!(it->second.tryFile.empty()))
					{
						if (stat(tryFilePath.c_str(), &buffer) == 0)
						{
							std::string fileContent = readFileContent(tryFilePath);
							std::string contentType = getContentType(tryFilePath);
							server.setResponse("HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\n\r\n" + fileContent);
							server.setCheckFile(1);
							return true;
						}
					}
				}

				if (is_directory(completeFullPath))
				{
					if ((it->second.autoindexPresent == true))
					{
						if ((it->second.directoryListingEnabled == true) && (!fileExistsInDirectory(completeFullPath, serverConfig.defaultFile)))
						{
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
								if (handleDirectoryListing(serverConfig, requestMsg, server))
									return true;
							}
						}
					}
				}

			}
			else if (it->first == getDirectoryPath(requestMsg.path))
			{
				std::string fred = getNewPath(serverConfig.RootDirectory, it->second.RootDirectory);
				std::string bbb = server.getCompletePath(fred);
				std::string aaa = bbb + requestMsg.path;
				//std::cout << "AAA [][] : "<< aaa << std::endl;
				server.setCompletePath(aaa);

				int bodySizeBytes = server.getContentLength();
				bodySizeBytes -= 200;
				if (bodySizeBytes > serverConfig.maxRequestSize)
				{
					handleError2(413, server, serverConfig, requestMsg);
					return false;
				}
				serverConfig.RootDirectory = getNewPath(serverConfig.RootDirectory, it->second.RootDirectory);

				std::string requestMethod = methodToString(requestMsg.method);
				std::transform(requestMethod.begin(), requestMethod.end(), requestMethod.begin(), ::toupper); // mudar para maiusculas
				bool methodAllowed = isMethodAllowed(it->second.allowedMethods, requestMethod);
				if (!methodAllowed)
				{
					handleError2(405, server, serverConfig, requestMsg);
					return false;
				}
				return true;
			}
		}
	}
	std::string aaa = serverConfig.RootDirectory + requestMsg.path;
	//std::cout << "aaa: " << aaa << std::endl;
	std::string bbb = server.getCompletePath(aaa);
	server.setCompletePath(bbb);
	aaa = ifFileRmoveFile(aaa);
	serverConfig.RootDirectory = aaa;

	if (!is_directory(aaa) && methodToString(requestMsg.method) != "POST")
	{
		handleError2(404, server, serverConfig, requestMsg);
		return false;
	}
	return true;
}

// Process the request and send the response
void processRequest(const std::string &request, ServerInfo &server)
{
	if (request.empty())
		return;

	std::string ParaCGI = request;
	std::string requestCopy = request;
	HTTrequestMSG requestMsg;
	HTTPParser parser;
	size_t maxSize = 100000;
	if (maxSize > requestCopy.size())
		maxSize = requestCopy.size();

	if (parser.parseRequest(requestCopy, requestMsg, maxSize))
	{
		std::vector<int> ports = server.getPortList();

		if (ports.empty())
		{
			std::cerr << "Error: No ports found." << std::endl;
			return;
		}

		std::string::size_type colonPos = requestMsg.hostname.find(":");
		std::string portStr;
		int porta = 0;
		if (colonPos != std::string::npos)
		{
			portStr = requestMsg.hostname.substr(colonPos + 1);
			porta = std::atoi(portStr.c_str());
		}
		std::string nameHost;
		if (colonPos != std::string::npos)
			nameHost = requestMsg.hostname.substr(0, colonPos);

		std::map<int, std::map<std::string, ParserConfig> >::iterator portIt;
		for (portIt = serversByPortAndHost.begin(); portIt != serversByPortAndHost.end(); ++portIt)
		{
			std::map<std::string, ParserConfig>::iterator hostIt;
			for (hostIt = portIt->second.begin(); hostIt != portIt->second.end(); ++hostIt)
			{
				if (portIt->first == porta && hostIt->first == nameHost)
				{
					bool alreadyExists = false;
					if (!alreadyExists)
					{
						ParserConfig &config = hostIt->second;
						const conf_File_Info configInfoPtr = config.getServerConfigurations();
						conf_File_Info configInfo = configInfoPtr;
						server.addConfig(portIt->first, configInfo);
						alreadyExists = true;
						//std::cout << "==> Port: " << configInfo.portListen << std::endl;
						//std::cout << "==> Host: " << configInfo.host << std::endl;
						//std::cout << "==> ServerName: " << configInfo.ServerName << std::endl;
					}
					else
						std::cout << "Configuration already exists for this host and port." << std::endl;
				}
			}
		}
		conf_File_Info &serverConfig = server.getConfig(porta);

		std::string originalRootDirectory = serverConfig.RootDirectory;

		CR Chunked = CR(ParaCGI);

		if (processRulesRequest(requestMsg, server) == true)
		{
			if (Chunked.ItIsChunked(ParaCGI))
			{
				try
				{
					Chunked.CheckTheChunk();
					if (requestMsg.is_cgi == true)
					{
						try
						{
							CGI cgi(serverConfig, requestMsg);
							std::string httpResponse = cgi.PerformCGI(server.clientSocket, ParaCGI);

							server.setResponse(httpResponse);
							printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
						}
						catch (const CGI::CGI_ExceptionClass &e)
						{
							int error = e.GetErrorCode();
							handleError2(error, server, serverConfig, requestMsg);
							std::cerr << e.what() << "\n";
						}
					}
				}
				catch (const CR::CR_ExceptionClass &e)
				{
					int error = e.GetErrorCode();
					handleError2(error, server, serverConfig, requestMsg);
					std::cerr << e.what() << "\n";
				}
			}
			else if (requestMsg.is_cgi == true)
			{
				try
				{
					//std::cout << "Request is CGI\n";
					CGI cgi(serverConfig, requestMsg);
					std::string httpResponse = cgi.PerformCGI(server.clientSocket, ParaCGI);

					server.setResponse(httpResponse);
					printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
				}
				catch (const CGI::CGI_ExceptionClass &e)
				{
					int error = e.GetErrorCode();
					handleError2(error, server, serverConfig, requestMsg);
					std::cerr << e.what() << "\n";
				}
			}
			else if (requestMsg.is_cgi == false)
			{
				std::string fileUploadDirectoryCopy = serverConfig.fileUploadDirectory;
				std::string rootDirectoryCopy = serverConfig.RootDirectory;
				handleRequest(requestMsg, server, serverConfig);
			}
		}
		serverConfig.RootDirectory = originalRootDirectory;
	}
	else
	{
		std::cout << RED << "Erro parser da requisição GET --> get.cpp.\n"
				  << RESET;
		std::cout << "Error message: " << requestMsg.error << std::endl;
	}
}

// Checks if file exits
bool fileExists(const std::string &filePath)
{
	struct stat buffer;
	return (stat(filePath.c_str(), &buffer) == 0);
}

// Function to handle the request from the HTTP method
void handleRequest(HTTrequestMSG &request, ServerInfo &server, conf_File_Info &serverConfig)
{
	if (request.path == "/favicon.ico")
	{
		//std::string faviconPath = "./resources/website/favicon.ico";
		std::string faviconPath = serverConfig.RootDirectory + "/favicon.ico";
		std::string fileContent = readFileContent(faviconPath);
		if (!fileContent.empty())
		{
			std::string contentType = "image/x-icon"; // Define the content type based on the file extension
			server.setResponse("HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\n\r\n" + fileContent);
		}
		else
			handleError2(404, server, serverConfig, request);
	}
	else
	{
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
void ServerInfo::handleUnknownRequest(HTTrequestMSG &requestMsg, ServerInfo &server, conf_File_Info &serverConfig)
{
	handleError2(501, server, serverConfig, requestMsg);
}

std::string readFileContent(const std::string &filePath)
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
bool isDirectory(const std::string &path)
{
	struct stat buffer;
	if (stat(path.c_str(), &buffer) == 0)
	{
		return S_ISDIR(buffer.st_mode);
	}
	return false;
}

bool fileExistsInDirectory(const std::string &directory, const std::string &filename)
{
	std::string fullPath = directory + "/" + filename;

	if (access(fullPath.c_str(), F_OK) != -1)
		return true;
	else
		return false;
}

void handleError2(int errorCode, ServerInfo &server, conf_File_Info &serverConfig, const HTTrequestMSG &requestMsg)
{
	std::stringstream ss;
	ss << errorCode;
	std::string errorCodeStr = ss.str();
	std::string originalRootDirectory = server.getRootOriginalDirectory();

	char realPath[PATH_MAX];
	char realPath2[PATH_MAX];

	realpath(serverConfig.RootDirectory.c_str(), realPath);
	serverConfig.RootDirectory = std::string(realPath);
	// std::cout << "[error] serverConfig.RootDirectory2 : " << serverConfig.RootDirectory << std::endl;

	realpath(originalRootDirectory.c_str(), realPath2);
	originalRootDirectory = std::string(realPath2);
	// std::cout << "[error] originalRootDirectory2 : " << originalRootDirectory << std::endl;

	std::string errorFilePath2;
	std::string dois;

	for (std::map<std::string, std::string>::const_iterator it = server.nameAfterSlashSets.begin(); it != server.nameAfterSlashSets.end(); ++it)
	{
		if (it->first == errorCodeStr)
		{
			errorFilePath2 = originalRootDirectory + "/" + it->second;
			dois = it->second;
		}
	}
	//std::cout << "errorFilePath2: " << errorFilePath2 << std::endl;
	ServerErrorHandler errorHandler;
	if (fileExists(errorFilePath2))
	{
		if (server.nameAfterSlashSets.size() > 1)
		{
			if (server.nameAfterSlashSets.find(errorCodeStr) != server.nameAfterSlashSets.end())
			{
				std::string errorMessage = errorHandler.getErrorMessage(errorCode);
				std::ifstream errorFile(errorFilePath2.c_str());
				std::string errorFileContent;
				std::copy(std::istreambuf_iterator<char>(errorFile), std::istreambuf_iterator<char>(), std::back_inserter(errorFileContent));
				server.setResponse("HTTP/1.1 " + errorCodeStr + " " + errorMessage + "\r\nContent-Type: text/html\r\n\r\n" + errorFileContent);
			}
		}
		else
		{
			std::string errorMessage = errorHandler.getErrorMessage(errorCode);
			std::string errorFilePath2 = originalRootDirectory + "/" + errorCodeStr + ".html";
			std::ifstream errorFile(errorFilePath2.c_str());
			std::string errorFileContent;
			std::copy(std::istreambuf_iterator<char>(errorFile), std::istreambuf_iterator<char>(), std::back_inserter(errorFileContent));
			server.setResponse("HTTP/1.1 " + errorCodeStr + " " + errorMessage + "\r\nContent-Type: text/html\r\n\r\n" + errorFileContent);
		}
	}
	else
	{
		std::string errorMessage = errorHandler.getErrorMessage(errorCode);
		server.setResponse("HTTP/1.1 " + errorCodeStr + " " + errorMessage + "\r\nContent-Type: text/plain\r\n\r\n" + errorMessage + "\nERROR " + errorCodeStr + "\n");
	}

	printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
}

std::string checkForCgiBin(const std::string &path, const std::string &filePath, ServerInfo &server)
{
	std::string resultPath = path;
	std::string aaa;
	if (!(path.find("cgi-bin") == std::string::npos))
	{
		resultPath = filePath;
		aaa = server.getCompletePath(resultPath);
		resultPath = aaa;
	}
	return resultPath;
}

void ServerInfo::handleGetRequest(HTTrequestMSG &requestMsg, ServerInfo &server, conf_File_Info &serverConfig)
{
	std::string rootDirectory = serverConfig.RootDirectory;

	if (!rootDirectory.empty() && rootDirectory[0] != '/')
		rootDirectory = '/' + rootDirectory;

	std::string fullPath = getCompletePath2();
	if (fullPath[fullPath.length() - 1] == '/')
		fullPath.erase(fullPath.length() - 1);

	std::string filePath = requestMsg.path;
	std::string fullPath2 = checkForCgiBin(fullPath, filePath, server);
	fullPath = fullPath2;
	//std::cout << "____fullPath :"<< fullPath << std::endl;
	if (!fileExists(fullPath))
	{
		//std::cout << "AQUI" << std::endl;
		handleError2(404, server, serverConfig, requestMsg);
		return;
	}

	int chechFILE = server.getCheckFile();
	if (isDirectory(fullPath) && chechFILE == 1)
	{
		server.getResponse();
		printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
		server.setCheckFile(0);
		return;
	}

	if (isDirectory(fullPath) && serverConfig.directoryListingEnabled == true && fileExistsInDirectory(fullPath, serverConfig.defaultFile) == false)
	{
		handleDirectoryListing(serverConfig, requestMsg, server);
		server.getResponse();
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

			if (stat(indexPath.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode))
			{
				std::string fileContent = readFileContent(indexPath);
				if (fileContent.empty())
				{
					std::cerr << "[DEBUG] Index file content is empty or could not be read: " << indexPath << std::endl;
					handleError2(500, server, serverConfig, requestMsg);
					return;
				}
				std::string contentType = getContentType(indexPath);
				server.setResponse("HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\n\r\n" + fileContent);
			}
			else
			{
				std::cerr << "[DEBUG] Index file not found or is not a regular file: " << indexPath << std::endl;
				handleError2(403, server, serverConfig, requestMsg);
				return;
			}
		}
	}
	else
	{
		handleError2(500, server, serverConfig, requestMsg);
	}

	printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
}

std::string extractFileNameFromURL(const std::string &url)
{
	std::string fileName;
	std::istringstream iss(url);
	std::string token;

	// Procura pelo token 'file=' na URL
	while (std::getline(iss, token, '='))
	{
		if (token == "file")
		{
			if (std::getline(iss, fileName, '&'))
			{
				size_t pos;
				while ((pos = fileName.find("%20")) != std::string::npos)
				{
					fileName.replace(pos, 3, " ");
				}
				break;
			}
		}
	}

	return fileName;
}

std::string extractFileNameFromPath(const std::string &path)
{
	size_t pos = path.find_last_of("/\\");
	if (pos != std::string::npos)
	{
		return path.substr(pos + 1);
	}
	else
	{
		return path;
	}
}

// void ServerInfo::handleDeleteRequest(HTTrequestMSG& requestMsg, ServerInfo& server)
void ServerInfo::handleDeleteRequest(HTTrequestMSG &requestMsg, ServerInfo &server, conf_File_Info &serverConfig)
{
	try
	{
		if (server.portListen.empty())
		{
			std::cerr << "No ports available." << std::endl;
			return;
		}
		if (requestMsg.method != HTTrequestMSG::DELETE)
		{
			handleError2(405, server, serverConfig, requestMsg);
			return;
		}

		std::string fullPath = getCompletePath2();
		//std::cout << "fullPath after entrada: " << fullPath << std::endl;
		if (fullPath[fullPath.length() - 1] == '/')
			fullPath.erase(fullPath.length() - 1);

		std::string filePath = requestMsg.path;
		std::string fullPath2 = checkForCgiBin(fullPath, filePath, server);
		fullPath = fullPath2;

		//std::cout << "fullPath after cgi_check: " << fullPath << std::endl;
		//std::cout << "Request path: " << requestMsg.path << std::endl;

		std::string fileName = extractFileNameFromPath(requestMsg.path);
		//std::cout << "File name: " << fileName << std::endl;

		int port = server.portListen[0];
		std::string rootDirectory1 = server.configs[port].RootDirectory;
		//std::cout << "RootDirectory: " << rootDirectory1 << std::endl;
		std::string fileUploadDirectory = server.configs[port].fileUploadDirectory;
		//std::cout << "FileUploadDirectory: " << fileUploadDirectory << std::endl;
		std::string dataDirectory = rootDirectory1 + fileUploadDirectory + "/";
		//std::cout << "dataDirectory :" << dataDirectory << std::endl;

		//std::cout << "File path_1: " << filePath << std::endl;
		if (!filePath.empty() && filePath[0] == '/')
		{
			filePath.erase(0, 1);
		}
		//std::cout << "File path_2: " << filePath << std::endl;

		if (fileName.empty())
		{
			handleError2(400, server, serverConfig, requestMsg);
			return;
		}

		if (fileName.find("..") != std::string::npos || fileName.find("/") != std::string::npos)
		{
			handleError2(400, server, serverConfig, requestMsg);
			return;
		}

		filePath = fullPath;
		if (access(filePath.c_str(), F_OK) != -1)
		{
			if (remove(filePath.c_str()) == 0)
			{
				setResponse("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nFile deleted successfully.\n");
				printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
				return;
			}
			else
			{
				handleError2(501, server, serverConfig, requestMsg);
				return;
			}
		}
		else
		{
			handleError2(404, server, serverConfig, requestMsg);
			return;
		}
	}
	catch (const std::runtime_error &e)
	{
		std::cerr << e.what() << std::endl;
		//std::cout << "--[2]--" << std::endl;
		handleError2(500, server, serverConfig, requestMsg);
		return;
	}
}

std::map<std::string, std::pair<std::string, std::string> > parseMultipartFormData(const std::string &bodyConst, const std::string &boundary)
{
	std::map<std::string, std::pair<std::string, std::string> > parts;
	std::string body = bodyConst;
	size_t pos = 0;
	std::string delimiter = "--" + boundary;

	while ((pos = body.find(delimiter, pos)) != std::string::npos)
	{
		size_t endPos = body.find(delimiter, pos + delimiter.length());
		if (endPos == std::string::npos)
			break;

		std::string part = body.substr(pos + delimiter.length() + 2, endPos - pos - delimiter.length() - 4);
		size_t headerEndPos = part.find("\r\n\r\n");
		if (headerEndPos != std::string::npos)
		{
			std::string headers = part.substr(0, headerEndPos);
			std::string content = part.substr(headerEndPos + 4);
			std::istringstream headerStream(headers);
			std::string headerLine;
			std::string name, filename;
			while (std::getline(headerStream, headerLine))
			{
				size_t namePos = headerLine.find("name=\"");
				if (namePos != std::string::npos)
				{
					size_t nameEndPos = headerLine.find("\"", namePos + 6);
					if (nameEndPos != std::string::npos)
						name = headerLine.substr(namePos + 6, nameEndPos - namePos - 6);
				}
				size_t filenamePos = headerLine.find("filename=\"");
				if (filenamePos != std::string::npos)
				{
					size_t filenameEndPos = headerLine.find("\"", filenamePos + 10);
					if (filenameEndPos != std::string::npos)
						filename = headerLine.substr(filenamePos + 10, filenameEndPos - filenamePos - 10);
				}
			}
			parts[name] = std::make_pair(content, filename);
		}
		pos = endPos + delimiter.length();
	}
	return parts;
}

bool isExtensionValid(const std::string& fileName, const std::set<std::string>& allowedExtensions)
{
	size_t dotPos = fileName.rfind('.');
	if (dotPos == std::string::npos) return false; // Sem extensão
	std::string extension = fileName.substr(dotPos + 1);
	return allowedExtensions.find(extension) != allowedExtensions.end();
}

bool isFileNameValid(const std::string& fileName)
{
	return !fileName.empty() && fileName.find('/') == std::string::npos && fileName.find('\\') == std::string::npos;
}

void ServerInfo::handlePostRequest(HTTrequestMSG &request, ServerInfo &server, conf_File_Info &serverConfig)
{
	if (!request.is_cgi)
	{
		std::string contentLengthStr = request.headers["Content-Length"];
		if (contentLengthStr.empty())
		{
			handleError2(411, server, serverConfig, request);
			return;
		}

		size_t contentLength = atoi(contentLengthStr.c_str());
		if (request.body.size() != contentLength)
		{
			handleError2(411, server, serverConfig, request);
			return;
		}

		std::string body = request.body;
		if (body.empty())
		{
			handleError2(400, server, serverConfig, request);
			return;
		}

		std::string contentType = request.headers["Content-Type"];
		if (contentType.find("multipart/form-data") != std::string::npos)
		{
			size_t boundaryPos = contentType.find("boundary=");
			if (boundaryPos == std::string::npos)
			{
				handleError2(400, server, serverConfig, request);
				return;
			}

			const std::string extensionsArray[] = {"jpg", "png", "txt", "pdf", "html", "py", "doc", "docx", "xls", "xlsx", "ppt", "pptx", "mp3", "mp4", "avi", "mkv", "gif", "csv", "json", "xml"};
			std::set<std::string> allowedExtensions(extensionsArray, extensionsArray + sizeof(extensionsArray) / sizeof(extensionsArray[0]));

			std::string boundary = contentType.substr(boundaryPos + 9);
			std::map<std::string, std::pair<std::string, std::string> > parts = parseMultipartFormData(body, boundary);
			std::map<std::string, std::pair<std::string, std::string> >::iterator it = parts.find("file");
			if (it != parts.end())
			{
				std::string fullPath = server.getCompletePath2();
				std::string fileContent = it->second.first;
				std::string fileName = it->second.second;

				if (!isFileNameValid(fileName))
				{
					handleError2(400, server, serverConfig, request);
					return;
				}
				if (!isExtensionValid(fileName, allowedExtensions))
				{
					handleError2(415, server, serverConfig, request);
					return;
				}

				std::ofstream outFile((fullPath + fileName).c_str(), std::ios::binary);
				if (!outFile)
				{
					handleError2(500, server, serverConfig, request);
					return;
				}
				outFile << fileContent;
				outFile.close();

				std::ifstream inFile((fullPath + fileName).c_str(), std::ios::binary);
				if (!inFile)
				{
					handleError2(500, server, serverConfig, request);
					return;
				}
				std::string fileContentForResponse((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
				inFile.close();

				std::string httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\n\r\n";
				httpResponse += fileContentForResponse;

				setResponse(httpResponse);
			}
			else
			{
				handleError2(400, server, serverConfig, request);
				return;
			}
		}
		else
		{
			std::string response;
			std::string delimiter = "&";
			size_t pos = 0;
			std::string token;

			while ((pos = body.find(delimiter)) != std::string::npos)
			{
				token = body.substr(0, pos);
				response += token + "\n";
				body.erase(0, pos + delimiter.length());
			}
			response += body;
			if (response.empty())
				std::cerr << "Error: Response is empty" << std::endl;

			//Create HTTP response with HTML content
			std::vector<int> portList = server.getPortList();
			int port = portList[0];
			std::string serverAddress = "127.0.0.1";
			std::stringstream ss;
			ss << port;
			std::string portStr = ss.str();

			std::string httpResponse;
			httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
			httpResponse += "<html><head><style>body { background: #ADD8E6; }</style></head><body>";
			httpResponse += "<p>Received POST data:</p><pre><b>" + response + "</b></pre>";
			httpResponse += "<button onclick=\"location.href='http://" + serverAddress + ":" + portStr + "'\" type=\"button\">Go Home</button>";
			httpResponse += "</body></html>\n";

			setResponse(httpResponse);
		}
	}
	printLog(methodToString(request.method), request.path, request.version, server.getResponse(), server);
}

void setupRunServer(std::vector<ServerInfo *> &servers, fd_set &read_fds, fd_set &write_fds, int &max_fd)
{

	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);

	max_fd = -1;
	for (std::vector<ServerInfo *>::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		int sockfd = (*it)->getSocketFD();
		FD_SET(sockfd, &read_fds);
		if (sockfd > max_fd)
			max_fd = sockfd;
	}

	std::cout << "\n<" << GREEN << "=+=+=+=+=+=+=+=+=+=" << RESET << " Waiting for client "
			  << GREEN << "=+=+=+=+=+=+=+=+=+=" << RESET << ">\n"
			  << std::endl;

	time_t now = time(NULL);
	char timestamp[100];
	strftime(timestamp, sizeof(timestamp), "[%d/%b/%Y %T]", localtime(&now));

	for (std::vector<ServerInfo *>::iterator it = servers.begin(); it != servers.end(); ++it)
	{

		std::vector<int> ports = (*it)->getPortList();
		for (std::vector<int>::iterator portIt = ports.begin(); portIt != ports.end(); ++portIt)
			std::cout << BG_CYAN_BLACK << timestamp << RESET << " Listening on http://127.0.0.1:" << CYAN << *portIt << RESET;
		std::cout << std::endl;
	}
}

bool ends_with(const std::string &value, const std::string &ending)
{
	if (ending.size() > value.size())
		return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::string getContentType(const std::string &filePath)
{
	std::string contentType;
	if (ends_with(filePath, ".html"))
		contentType = "text/html";
	else if (ends_with(filePath, ".css"))
		contentType = "text/css";
	else if (ends_with(filePath, ".js"))
		contentType = "application/javascript";
	else
		contentType = "text/plain";
	return contentType;
}

void runServer(std::vector<ServerInfo *> &servers, fd_set read_fds, fd_set write_fds, int max_fd)
{
	int newsockfd = -1;
	std::list<int> socketsToClose;

	while (!flag)
	{
		fd_set temp_read_fds = read_fds;
		fd_set temp_write_fds = write_fds;

		if (select(max_fd + 1, &temp_read_fds, &temp_write_fds, NULL, NULL) < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}
			printError("Error on select");
			return;
		}

		for (std::vector<ServerInfo *>::iterator it = servers.begin(); it != servers.end(); ++it)
		{
			int sockfd = (*it)->getSocketFD();
			if (FD_ISSET(sockfd, &temp_read_fds))
			{
				sockaddr_in cli_addr;
				socklen_t clilen = sizeof(cli_addr);
				newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
				if (newsockfd < 0)
				{
					(*it)->sair = 1;
					printError("Error on accept");
					return;
				}
				std::string request = readRequest(newsockfd, **it);

				(*it)->clientSocket = newsockfd;
				processRequest(request, **it);
				// Add new socket to write_fds
				FD_SET(newsockfd, &write_fds);
				if (newsockfd > max_fd)
					max_fd = newsockfd;
			}

			if (it != servers.end() && (*it)->clientSocket >= 0 && FD_ISSET((*it)->clientSocket, &temp_write_fds))
			{
				// Write response to the client
				int clientSocket = (*it)->clientSocket;
				//std::cout << "RESPONSE:\n" << (*it)->getResponse() << "[END]\n";
				//std::cout << "RESPONSE:\n" << (*it)->getResponse() << "[END]\n"; // CHECKING THE RESPONSE
				ssize_t bytesWritten = write(clientSocket, (*it)->getResponse().c_str(), (*it)->getResponse().length());
				if (bytesWritten < 0)
				{
					printError("Error writing to socket");
					FD_CLR(clientSocket, &read_fds);
					FD_CLR(clientSocket, &write_fds);
					close(clientSocket);
					(*it)->clientSocket = -1;
				}
				else if (bytesWritten == 0)
				{
					close(clientSocket);
					FD_CLR(clientSocket, &read_fds);
					FD_CLR(clientSocket, &write_fds);
					(*it)->clientSocket = -1;
				}
				else
				{
					FD_CLR(clientSocket, &read_fds);
					FD_CLR(clientSocket, &write_fds);
					socketsToClose.push_back(clientSocket);
					(*it)->clientSocket = -1;
				}
				(*it)->clientSocket = -1;
				if (newsockfd >= 0)
				{
					close(newsockfd);
					newsockfd = -1;
				}
			}
		}
		for (std::list<int>::iterator it = socketsToClose.begin(); it != socketsToClose.end(); ++it)
		{
			close(*it);
		}
		socketsToClose.clear();
	}
	// Cleanup section
	for (std::vector<ServerInfo *>::iterator it = servers.begin(); it != servers.end(); ++it)
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
}
