/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fde-carv <fde-carv@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/26 15:10:07 by fde-carv          #+#    #+#             */
/*   Updated: 2024/06/06 12:39:17 by fde-carv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/get.hpp"

ServerInfo::ServerInfo()
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		handleError("Error opening socket.");
		exit(-1);
	}
	memset(&serv_addr, 0, sizeof(serv_addr));
	this->clientSocket = -1;
	this->rootUrl = "resources";
	this->response = "";
	this->clientSockets.clear();
	this->portListen.clear();
	this->bytesReadTotal = 0;
	this->cli_addrs.clear();
	for (std::vector<sockaddr_in>::iterator it = this->cli_addrs.begin(); it != this->cli_addrs.end(); ++it)
		memset(&(*it), 0, sizeof(*it));
	this->contentLength = 0;
	this->RootDirectory = "";
	this->configs = std::map<int, conf_File_Info>();
}

ServerInfo::~ServerInfo()
{
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


// ================================================================================================= //
// ======================================= HELPER FUNCTIONS ======================================== //
// ================================================================================================= //

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

	std::string methodColor = (method == "GET") ? YELLOW : CYAN;

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
	std::cout << BG_CYAN_BLACK << timestamp << RESET << BLUE << " [" << RESET << requestCount << BLUE << "] \"" << methodColor << method << " " << path << " ";
	std::cout << version << RESET << "\" " << statusColor << statusCode << RESET << " ";

	if (method == "GET")
		std::cout << server.getResponse().length();
	else if(method == "POST")
		std::cout << server.getContentLength();

	std::cout << "\n" << BG_CYAN_BLACK << timestamp << RESET << RED << " [" << RESET << requestCount << RED << "] " \
	<< BLUE << "Connection ended successfully" << RESET << std::endl;
}

// Function to handle errors without exiting the program
void handleError(const std::string& errorMessage) //, int errorCode)
{
	std::cerr << RED1 << "\n" << errorMessage << RESET << std::endl;
	//exit(-1);
}

// check if the path is a directory
bool is_directory(const std::string &path)
{
	DIR *dir = opendir(path.c_str());
	if (dir)
	{
		closedir(dir);
		return true;
	}
	else
		return (false);
}


std::vector<std::string> readDirectoryContent(const std::string& directoryPath)
{
	DIR* dir;
	struct dirent* ent;
	std::vector<std::string> files;

	if ((dir = opendir(directoryPath.c_str())) != NULL)
	{
		// Add all the files and directories within directory to the files vector
		while ((ent = readdir(dir)) != NULL)
		{
			std::string filename = ent->d_name;
			if (filename != "." && filename != "..") // Skip the current directory and parent directory
				files.push_back(filename);
		}
		closedir(dir);
	}
	else
	{
		std::cerr << "Could not open directory: " << directoryPath << std::endl; // Could not open directory
		return files;
	}

	std::sort(files.begin(), files.end()); // Sort the files vector

	return files;
}

void setupDirectory(ServerInfo& server, const conf_File_Info& config)
{
	chmod("/resources/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	std::string rootDir = config.RootDirectory;
	std::string rootUrl = server.getRootUrl();
	if (rootDir.substr(0, rootUrl.length()) != rootUrl)
	{
		handleError("Error: Root URL should start with 'resources' directory.");
		exit(-1);
	}

	std::string subDir = rootDir.substr(rootUrl.length());

	if (!subDir.empty() && subDir[0] == '/')
		subDir = subDir.substr(1);

	std::string path = "";
	std::stringstream ss(rootDir);
	std::string token;

	while (std::getline(ss, token, '/'))
	{
		path += token + "/";
		if (!is_directory(path))
		{
			if (mkdir(path.c_str(), 0777) == -1)
			{
				perror("Error creating directory");
				exit(EXIT_FAILURE);
			}

			if (chmod(path.c_str(), 0777) == -1)
			{
				perror("Error changing directory permissions");
				exit(EXIT_FAILURE);
			}
		}
	}
}


// bool mkdirs(const std::string& path)
// {
// 	size_t pos = 0;
// 	std::string dir;
// 	int mdret;

// 	if(path[path.size() - 1] != '/') {
// 		// force trailing / so we can handle everything in loop
// 		dir = path + "/";
// 	} else {
// 		dir = path;
// 	}

// 	while((pos = dir.find_first_of('/', pos)) != std::string::npos) {
// 		std::string subDir = dir.substr(0, pos++);
// 		if(subDir.size() > 0) { // if leading / first time is 0 length
// 			mdret = mkdir(subDir.c_str(), 0777);
// 			if(mdret && errno != EEXIST) {
// 				return false;
// 			}
// 		}
// 	}

// 	return true;
// }

// void setupUploadDirectory(const std::string& serverRoot, const std::string& uploadDirectory) {
// 	// Verificar se o diretório raiz do servidor existe
// 	struct stat rootInfo;
// 	if (stat(serverRoot.c_str(), &rootInfo) == 0 && S_ISDIR(rootInfo.st_mode)) {
// 		// O diretório raiz do servidor existe
// 		std::string fullUploadDirectory = serverRoot;
// 		if (!uploadDirectory.empty() && uploadDirectory[0] != '/') {
// 			// Adiciona uma barra apenas se o uploadDirectory não for vazio e não começar com uma barra
// 			fullUploadDirectory += "/";
// 		}
// 		fullUploadDirectory += uploadDirectory;

// 		std::cout << "Attempting to create upload directory: " << fullUploadDirectory << std::endl;

// 		// Verificar se o diretório de upload existe
// 		struct stat uploadInfo;
// 		if (stat(fullUploadDirectory.c_str(), &uploadInfo) != 0) {
// 			std::cerr << "Error: Upload directory '" << fullUploadDirectory << "' does not exist. Creating it..." << std::endl;
// 			if (mkdirs(fullUploadDirectory.c_str()) != 0) {
// 				std::cerr << "Error creating upload directory '" << fullUploadDirectory << "': " << strerror(errno) << std::endl;
// 			} else {
// 				std::cout << "Upload directory '" << fullUploadDirectory << "' created successfully." << std::endl;
// 			}
// 		} else if (!(uploadInfo.st_mode & S_IFDIR)) {
// 			std::cerr << "Error: '" << fullUploadDirectory << "' is not a directory." << std::endl;
// 		} else {
// 			std::cout << "Upload directory validated: " << fullUploadDirectory << std::endl;
// 		}
// 	} else {
// 		// O diretório raiz do servidor não existe, não é possível criar o diretório de upload
// 		std::cerr << "Error: Server root directory '" << serverRoot << "' does not exist." << std::endl;
// 	}
// }









//void setupDirectory(ServerInfo& server, conf_File_Info& config)
// void setupDirectory(ServerInfo& server, const conf_File_Info& config)
// {
// 	//std::cout << "\nRoot directory: " << config.RootDirectory << std::endl; // *DEBUG*
// 	//std::cout << "Root URL: " << server.getRootUrl() << std::endl; // *DEBUG*
// 	chmod("/resources/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

// 	std::string rootDir = config.RootDirectory;
// 	std::string rootUrl = server.getRootUrl();
// 	if(rootDir.substr(0, rootUrl.length()) != rootUrl)
// 	{
// 		handleError("Error: Root URL should star with \'resources\' directory.");
// 		exit(-1);
// 	}

// 	std::string subDir = rootDir.substr(rootUrl.length());

// 	if(!subDir.empty() && subDir[0] == '/')
// 		subDir = subDir.substr(1);

// 	std::string path = "";
// 	std::stringstream ss(rootDir);
// 	std::string token;

// 	while(std::getline(ss, token, '/')) {
// 		path += token + "/";
// 		if(!is_directory(path)) {
// 			if(mkdir(path.c_str(), 0777) == -1) {
// 				perror("Error creating directory");
// 				exit(EXIT_FAILURE);
// 			}

// 			if(chmod(path.c_str(), 0777) == -1) {
// 				perror("Error changing directory permissions");
// 				exit(EXIT_FAILURE);
// 			}
// 		}
// 	}
// }

int remove_file(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
	(void)sb;
	(void)ftwbuf;
	std::string filename(fpath);

	// If the path starts with "resources/website", do not remove it
	if(filename.substr(0, 17) == "resources/website")
	{
		return 0;
	}

	// Remove other files or directories
	if(typeflag == FTW_D)
		return rmdir(fpath);
	else
		return remove(fpath);
}

int remove_directory(const char *path)
{
	int result = nftw(path, remove_file, 64, FTW_DEPTH | FTW_PHYS | FTW_DEPTH);
	return result;
}

// Setup the server
void setupServer(ServerInfo& server, const conf_File_Info& config)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("Error on socket creation");
		exit(EXIT_FAILURE);
	}
	server.addSocketToList(sockfd);
	server.setSocketFD(sockfd);
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
		perror("setsockopt");
		handleError("Error on setsockopt.");
		exit(EXIT_FAILURE);
	}
	server.setSocketFD(sockfd);
	if (bind(server.getSocketFD(), (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("Error on binding");
		handleError("Error on binding.");
		exit(EXIT_FAILURE);
	}
	if (listen(server.getSocketFD(), 128) < 0)
	{
		perror("Error on listen");
		exit(EXIT_FAILURE);
	}
	server.addPortToList(config.portListen);
	server.addConfig(config.portListen, config);
}


void ServerInfo::setRedirectResponse(const std::string &location, const conf_File_Info &config)
{
	std::string statusMessage;

	if (config.redirectURL.httpStatusCode == 301)
		statusMessage = "301 Moved Permanently";
	else if (config.redirectURL.httpStatusCode == 302)
		statusMessage = "302 Found";
	else
		statusMessage = "302 Found"; // Default to 302 if an invalid status code is provided

	std::string response = "HTTP/1.1 " + statusMessage + "\r\n"
						   "Location: " + location + "\r\n"
						   "Content-Length: 0\r\n"
						   "\r\n";
	setResponse(response);
}

void ServerInfo::handleRedirectRequest(ServerInfo &server, const conf_File_Info &config)
{
	std::cout << "  ==>>>>> httpStatusCode: " << config.redirectURL.httpStatusCode << std::endl;
	std::cout << "  ==>>>>> destinationURL: " << config.redirectURL.destinationURL << std::endl;
	if (config.redirectURL.destinationURL == "/redirect-permanent")
	{
		server.setRedirectResponse("http://www.permanent-example.com", config);
	}
	else if (config.redirectURL.destinationURL == "/redirect-temporary")
	{
		server.setRedirectResponse("http://www.temporary-example.com", config);
	}
	// else if (parser.redirectURL == "/redirect-if-query")
	// {
	//     if (query == "token=12345")
	// 	{
	//         server.setRedirectResponse("http://www.example.com/valid-token", 302);
	//     } else
	// 	{
	//         server.setRedirectResponse("http://www.example.com/invalid-token", 302);
	//     }
	// }
	else if (config.redirectURL.destinationURL.find("/rewrite-redirect/oldpath/") == 0)
	{
		std::string newPath = config.redirectURL.destinationURL;
		newPath.replace(0, 19, "/newpath/");
		server.setRedirectResponse("http://www.rewrite-example.com" + newPath, config);
	}
	// else if (parser.redirectURL == "/")
	// {
	//     std::string userAgent = server.getUserAgent();
	//     if (userAgent.find("Mobile") != std::string::npos ||
	//         userAgent.find("Android") != std::string::npos ||
	//         userAgent.find("webOS") != std::string::npos ||
	//         userAgent.find("iPhone") != std::string::npos ||
	//         userAgent.find("iPad") != std::string::npos ||
	//         userAgent.find("iPod") != std::string::npos ||
	//         userAgent.find("BlackBerry") != std::string::npos ||
	//         userAgent.find("IEMobile") != std::string::npos ||
	//         userAgent.find("Opera Mini") != std::string::npos) {
	//         server.setRedirectResponse("http://m.example.com", 302);
	//     }
	// 	else
	// 	{
	//         server.setRedirectResponse("https://example.com" + uri, 301);
	//     }
	// }
	else
	{
		server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404");
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
			handleError("Error reading from socket.");
			exit(-1);
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
	server.setContentLength(contentLength);

	size_t actualDataSize = request.size();
	size_t headerSize = request.find("\r\n\r\n") + 4;
	if (contentLength > actualDataSize - headerSize)
	{
		size_t bytesReadTotal = actualDataSize - headerSize;
		while (bytesReadTotal < contentLength)
		{
			memset(buffer, 0, 4096);
			ssize_t bytesRead = recv(sockfd, buffer, std::min(static_cast<size_t>(4095), contentLength - bytesReadTotal), MSG_DONTWAIT);
			//std::cout << "Bytes read: " << bytesRead << std::endl; // Print the number of bytes read

			if (bytesRead < 0)
			{
				handleError("Error reading from socket.");
				exit(-1);
			}
			else if (bytesRead == 0)
			{
				std::cerr << "Socket has been closed by the other end." << std::endl;
				break;
			}
			else
			{
				buffer[bytesRead] = '\0';
			}
			request.append(buffer, bytesRead);
			bytesReadTotal += bytesRead;
			if (bytesReadTotal > request.size() - headerSize)
			{
				std::cerr << "Read beyond the end of available data." << std::endl;
				break;
			}
		}
	}
	//std::cout << "Request received: \n" << request << std::endl; // Print the request
	return request;
}


// Process the request and send the response
void processRequest(const std::string& request, ServerInfo& server)
{
	if (request.empty())
	{
		//std::cout << "Received empty request, ignoring." << std::endl;
		return;
	}

	// verificar map de configuração //	criar nova funcao



	std::string ParaCGI = request;
	std::string requestCopy = request;
	HTTrequestMSG requestMsg;
	HTTPParser parser;
	size_t maxSize = 100000; // Aumentar o tamanho máximo para 10MB
	if(maxSize > requestCopy.size())
		maxSize = requestCopy.size();
	if (parser.parseRequest(requestCopy, requestMsg, maxSize))
	{
		//if (requestMsg.is_cgi == false)
		//{
			std::vector<int> ports = server.getPortList();
			for (std::vector<int>::iterator it = ports.begin(); it != ports.end(); ++it) {
				std::cout << CYAN << "VECTOR Port: "<< *it << RESET << " ";
			}
			std::cout << std::endl;


			// Check if there are any ports in the list
			if (ports.empty()) {
				std::cerr << "Error: No ports found." << std::endl;
				return;
			}

			// Use the first port in the list
			int listeningPort = ports[0];
			std::cout << "Listening port: " << listeningPort << std::endl;

			// Get the configuration for the listening port
			conf_File_Info &serverConfig = server.getConfig(listeningPort);

		if (requestMsg.is_cgi == false) // ======ALTERAÇÂO======
		{
			std::string fileUploadDirectoryCopy = serverConfig.fileUploadDirectory;
			int portListenCopy = serverConfig.portListen;
			std::cout << RED << "!!!!! config upload: " << fileUploadDirectoryCopy << RESET << std::endl;
			std::cout << RED << "!!!!! config port: " << portListenCopy << RESET << std::endl;
			handleRequest(requestMsg, server);
		}
		//}
		else
		{
			try
			{
				CGI cgi;
				cgi.PerformCGI(server.clientSocket , ParaCGI, serverConfig);
			}
			catch(const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}

			// std::string dataDir = "./DATA";

			// // Check if /DATA directory exists
			// if (!is_directory(dataDir))
			// {
			// 	std::cout << "Directory /DATA does not exist yet. It will be created." << std::endl;
			// 	// Create /DATA directory
			// 	if (mkdir(dataDir.c_str(), 0777) == -1)
			// 	{
			// 		perror("Error creating /DATA directory");
			// 		exit(EXIT_FAILURE);
			// 	}

			// 	if (chmod(dataDir.c_str(), 0777) == -1)
			// 	{
			// 		perror("Error changing /DATA directory permissions");
			// 		exit(EXIT_FAILURE);
			// 	}
			// }

			// // Read the content of the /DATA directory and print it for debugging purposes
			// std::vector<std::string> fileList = readDirectoryContent(dataDir);
			// // Write the file list to a file
			// std::string fileListPath = "cgi-bin/fileList.txt";
			// std::ofstream outFile(fileListPath.c_str());
			// for (std::vector<std::string>::iterator it = fileList.begin(); it != fileList.end(); ++it) {
			// 	outFile << *it << "\n";
			// }
			// outFile.close();

			//config
			printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
		}
	}
	else
	{
		std::cout << RED << "Erro parser da requisição GET --> get.cpp.\n" << RESET;
		std::cout << "Error message: " << requestMsg.error << std::endl;
	}
}

bool fileExists(const std::string& filePath)
{
	struct stat buffer;
	return (stat(filePath.c_str(), &buffer) == 0);
}

// Function to handle the request from the HTTP method
void handleRequest(HTTrequestMSG& request, ServerInfo& server)
{
	// int port = server.getPortList()[0];
	// std::string filePath = server.getConfig(port).RootDirectory + request.path;
	// std::cout << "Port HANDLE REQUEST : " << port << std::endl;
	// conf_File_Info &serverConfig = server.getConfig(port);

	
	if (request.path == "/favicon.ico")
	{
		std::string faviconPath = "resources/website/favicon.ico"; // if the solicitation is for favicon.ico, reads and send the file content
		std::string fileContent = readFileContent(faviconPath);
		if (!fileContent.empty())
		{
			std::string contentType = "image/x-icon"; // Define the content type based on the file extension
			server.setResponse("HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\n\r\n" + fileContent);
		}
		else
			server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
	}
	else if (request.path == "/redirect")
	{
		//server.handleRedirectRequest(server, config);
	}
	else
	{
		std::string filePath = "resources/website" + request.path; // adjust this to your actual file path
		//std::string filePath = server.configs[port].RootDirectory + request.path;
		if (request.method == HTTrequestMSG::GET)
		{
			server.handleGetRequest(request, server);
		}
		else if (request.method == HTTrequestMSG::POST)
		{
			server.handlePostRequest(request, server);
		}
		else if (request.method == HTTrequestMSG::DELETE)
		{
			server.handleDeleteRequest(request, server);
		}
		else if (request.method == HTTrequestMSG::UNKNOWN)
		{
			server.handleUnknownRequest(request, server);
		}
	}
}

// Handles unknown requests
void ServerInfo::handleUnknownRequest(HTTrequestMSG& requestMsg, ServerInfo &server)
{
	std::string response = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
	this->setResponse(response);
	printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
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

bool isValidFilePath(const std::string& path)
{
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0);
}

bool isDirectory(const std::string& path)
{
	struct stat buffer;
	if (stat(path.c_str(), &buffer) == 0)
	{
		return S_ISDIR(buffer.st_mode);
	}
	return false;
}

void ServerInfo::handleGetRequest(HTTrequestMSG& requestMsg, ServerInfo& server)
{
	std::string fullPath = "resources/website" + requestMsg.path;

	if (!fileExists(fullPath))
	{
		std::cerr << "[DEBUG] File does not exist: " << fullPath << std::endl;
		server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
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
				server.setResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\nInternal server error\n");
				printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
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
			indexPath += "index.html";

			if (stat(indexPath.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode))
			{
				std::string fileContent = readFileContent(indexPath);
				if (fileContent.empty())
				{
					std::cerr << "[DEBUG] Index file content is empty or could not be read: " << indexPath << std::endl;
					server.setResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\nInternal server error\n");
					printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
					return;
				}
				std::string contentType = getContentType(indexPath);
				server.setResponse("HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\n\r\n" + fileContent);
			}
			else
			{
				std::cerr << "[DEBUG] Index file not found or is not a regular file: " << indexPath << std::endl;
				server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
				printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
				return;
			}
		}
	}
	else
	{
		std::cerr << "[DEBUG] Error retrieving file stats: " << fullPath << std::endl;
		server.setResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\nInternal server error\n");
	}
	printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
}





// void sendResponse(int sockfd, const std::string& response)
// {
//     ssize_t result = send(sockfd, response.c_str(), response.size(), MSG_NOSIGNAL);

//     if (result == -1)
//     {
//         if (errno == EPIPE)
//         {
//             // Broken pipe error, handle it here
//             std::cerr << "Broken pipe error when sending response." << std::endl;
//         }
//         else
//         {
//             // Other error, handle it here
//             std::cerr << "Error sending response: " << strerror(errno) << std::endl;
//         }
//     }
//     else if (result == 0)
//     {
//         // Socket closed by peer
//         std::cerr << "Socket closed by peer." << std::endl;
//     }
// }


std::string extractFileNameFromURL(const std::string& url) {
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

void ServerInfo::handleDeleteRequest(HTTrequestMSG& requestMsg, ServerInfo& server){
	try {
		if(server.portListen.empty()) {
			std::cerr << "No ports available." << std::endl;
			return;
		}

		int port = server.portListen[0];
		std::cout << "Port NO DELETE : " << port << std::endl;
		conf_File_Info &serverConfig = server.getConfig(port);

		if(serverConfig.fileUploadDirectory.empty()) {
			std::cout << "======>>>> File upload directory not set in the configuration file." << std::endl;
			setResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\n\r\nFile upload directory not set.");
			printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
			return;
		}

		if (requestMsg.method != HTTrequestMSG::DELETE) {
			setResponse("HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/html\r\n\r\nMethod not allowed.");
			printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
			return;
		}

		// Extract file name from query string
		std::string fileName = extractFileNameFromURL(requestMsg.query);

		std::cout << "Request pathAAA: " << requestMsg.path << std::endl;
		std::cout << "Query string: " << requestMsg.query << std::endl;
		std::cout << "File nameAAA: " << fileName << std::endl;

		if (fileName.empty()) {
			setResponse("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\nNo file specified.");
			printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
			return;
		}

		// Validate the file name format
		if (fileName.find("..") != std::string::npos || fileName.find("/") != std::string::npos) {
			setResponse("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\nInvalid file name.");
			printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
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
		
		std::cout << "Data directory: " << dataDirectory << std::endl;
		std::string filePath = dataDirectory + fileName;
		std::cout << "File path: " << filePath << std::endl;

		// Check if the file path is within the data directory
		if (filePath.substr(0, dataDirectory.size()) != dataDirectory) {
			setResponse("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\nInvalid file name.");
			printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
			return;
		}

		// Check if the file exists and is accessible
		if (access(filePath.c_str(), F_OK) != -1) {
			// The file exists and is accessible, try to delete it
			if (remove(filePath.c_str()) == 0) {
				setResponse("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nFile deleted successfully.");
			} else {
				setResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\n\r\nError deleting file.");
			}
			printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
		} else {
			// The file does not exist or is not accessible
			setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\nFile not found.");
			printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
		}
	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		setResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\n\r\nInternal Server Error.");
		printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
	}
}






void ServerInfo::handlePostRequest(HTTrequestMSG& request, ServerInfo &server)
{
	std::string contentLengthStr = request.headers["Content-Length"];
	if (contentLengthStr.empty())
	{
		this->setResponse("HTTP/1.1 411 Length Required\r\nContent-Type: text/plain\r\n\r\nError: Content-Length header is missing");
		return;
	}

	size_t contentLength = atoi(contentLengthStr.c_str());

	if (request.body.size() != contentLength) // Verificando se o tamanho do corpo corresponde ao Content-Length
	{
		this->setResponse("HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nError: Request body size does not match Content-Length");
		return;
	}

	std::string body = request.body; // Parse the request body

	if (body.empty()) // Check if the body is empty
	{
		this->setResponse("HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nError: Request body is empty");
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
	std::string httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
	httpResponse += "<html><head><style>body { background: #ADD8E6; }</style></head><body>";
	httpResponse += "<p>Received POST data:</p><pre>" + response + "</pre>";
	httpResponse += "<button onclick=\"location.href='index.html'\" type=\"button\">Go Home</button>";
	httpResponse += "</body></html>\n";

	// Set the response
	this->setResponse(httpResponse);

	// Log the request details
	printLog(methodToString(request.method), request.path, request.version, server.getResponse(), server);
}


void runServer(std::vector<ServerInfo>& servers)
{
	fd_set read_fds, write_fds;
	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);

	int max_fd = -1;
	for (std::vector<ServerInfo>::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		int sockfd = it->getSocketFD();
		FD_SET(sockfd, &read_fds);
		if (sockfd > max_fd)
			max_fd = sockfd;
	}

	std::cout << "\n<" << GREEN << "=+=+=+=+=+=+=+=+=+=" << RESET << " Waiting for client "
	<< GREEN << "=+=+=+=+=+=+=+=+=+=" << RESET << ">\n" << std::endl;

	time_t now = time(NULL);
	char timestamp[100];
	strftime(timestamp, sizeof(timestamp), "[%d/%b/%Y %T]", localtime(&now));

	for (std::vector<ServerInfo>::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		std::vector<int> ports = it->getPortList();
		for (std::vector<int>::iterator portIt = ports.begin(); portIt != ports.end(); ++portIt)
			std::cout << BG_CYAN_BLACK << timestamp << RESET << " Listening on http://127.0.0.1:" << CYAN << *portIt << RESET;
		std::cout << std::endl;
	}

	 while (1)
	{
		fd_set temp_read_fds = read_fds;
		fd_set temp_write_fds = write_fds;
		if (select(max_fd + 1, &temp_read_fds, &temp_write_fds, NULL, NULL) < 0)
		{
			perror("Error on select");
			exit(EXIT_FAILURE);
		}

		for (std::vector<ServerInfo>::iterator it = servers.begin(); it != servers.end(); ++it)
		{
			int sockfd = it->getSocketFD();
			if (FD_ISSET(sockfd, &temp_read_fds))
			{
				sockaddr_in cli_addr;
				socklen_t clilen = sizeof(cli_addr);
				int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
				if (newsockfd < 0)
				{
					perror("Error on accept");
					exit(EXIT_FAILURE);
				}

				std::string request = readRequest(newsockfd, *it);
				it->clientSocket = newsockfd;
				processRequest(request, *it);


				// Add new socket to write_fds
				FD_SET(newsockfd, &write_fds);
				if (newsockfd > max_fd)
					max_fd = newsockfd;
			}

			if (it != servers.end() && it->clientSocket >= 0 && FD_ISSET(it->clientSocket, &temp_write_fds))
			{
				// Write response to the client
				int clientSocket = it->clientSocket;

				write(clientSocket, it->getResponse().c_str(), it->getResponse().length());
				// Remove client socket from read_fds and write_fds
				FD_CLR(clientSocket, &read_fds);
				FD_CLR(clientSocket, &write_fds);
				close(clientSocket);
				it->clientSocket = -1; // Set clientSocket to -1 after closing it
			}
		}
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





// std::string readDirectoryContent(const std::string& directoryPath)
// {
// 	DIR* dir;
// 	struct dirent* ent;
// 	std::vector<std::string> files;

// 	if ((dir = opendir(directoryPath.c_str())) != NULL)
// 	{
// 		// Add all the files and directories within directory to the files vector
// 		while ((ent = readdir(dir)) != NULL)
// 		{
// 			std::string filename = ent->d_name;
// 			if (filename != "." && filename != "..") // Skip the current directory and parent directory
// 				files.push_back(filename);
// 		}
// 		closedir(dir);
// 	}
// 	else
// 	{
// 		std::cerr << "Could not open directory: " << directoryPath << std::endl; // Could not open directory
// 		return "";
// 	}

// 	std::sort(files.begin(), files.end()); // Sort the files vector

// 	std::string directoryContent; // Convert the files vector to a string
// 	for (std::vector<std::string>::const_iterator i = files.begin(); i != files.end(); ++i)
// 	{
// 		directoryContent += *i;
// 		directoryContent += "\n";
// 	}

// 	std::cout << "Directory content:\n" << directoryContent << std::endl; // Print directory content
// 	return directoryContent;
// }



// void ServerInfo::handleGetRequest(HTTrequestMSG& requestMsg, ServerInfo& server)
// {
// 	std::string fullPath = "resources/website" + requestMsg.path;

// 	if (!fileExists(fullPath))
// 	{
// 		//std::cout << "[DEBUG] File does not exist: " << fullPath << std::endl;
// 		server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
// 		printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
// 		return;
// 	}

// 	struct stat buffer;
// 	if (stat(fullPath.c_str(), &buffer) == 0)
// 	{
// 		if (S_ISREG(buffer.st_mode)) // Se for um arquivo regular
// 		{
// 			std::string fileContent = readFileContent(fullPath);
// 			std::string contentType = getContentType(fullPath);
// 			server.setResponse("HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\n\r\n" + fileContent);
// 		}
// 		else if (S_ISDIR(buffer.st_mode)) // Se for um diretório
// 		{
// 			if (!requestMsg.path.empty() && requestMsg.path[requestMsg.path.length() - 1] != '/')
// 			{
// 				server.setResponse("HTTP/1.1 301 Moved Permanently\r\nLocation: " + requestMsg.path + "/\r\n\r\n");
// 				printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
// 				return;
// 			}

// 			// Tentar encontrar e servir index.html dentro do diretório
// 			std::string indexPath = fullPath;
// 			if (indexPath[indexPath.length() - 1] != '/')
// 				indexPath += '/';
// 			indexPath += "index.html";

// 			if (stat(indexPath.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode)) // Verificar se há index.html no diretório
// 			{
// 				std::string fileContent = readFileContent(indexPath);
// 				std::string contentType = getContentType(indexPath);
// 				server.setResponse("HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\n\r\n" + fileContent);
// 			}
// 			else if (errno == ENOENT) // Se o arquivo/diretório não existir
// 			{
// 				server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
// 			}
// 			else // Se não houver index.html, retornar erro 403
// 			{
// 				server.setResponse("HTTP/1.1 403 Forbidden\r\nContent-Type: text/plain\r\n\r\nAccess to directories is forbidden.");
// 				printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
// 			}
// 		}
// 	}
// 	else // Se o arquivo não existir, retornar erro 404
// 	{
// 		server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
// 		printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
// 	}
// 	printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
// }



// void	runServer(std::vector<ServerInfo>& servers)
// {
// 	fd_set read_fds;
// 	FD_ZERO(&read_fds);

// 	int max_fd = -1;
// 	for (std::vector<ServerInfo>::iterator it = servers.begin(); it != servers.end(); ++it)
// 	{
// 		int sockfd = it->getSocketFD();
// 		FD_SET(sockfd, &read_fds);
// 		if (sockfd > max_fd)
// 			max_fd = sockfd;
// 	}

// 	std::cout << "\n<" << GREEN << "=+=+=+=+=+=+=+=+=+=" << RESET << " Waiting for client "
// 	<< GREEN << "=+=+=+=+=+=+=+=+=+=" << RESET << ">\n" << std::endl;

// 	time_t now = time(NULL);
// 	char timestamp[100];
// 	strftime(timestamp, sizeof(timestamp), "[%d/%b/%Y %T]", localtime(&now));

// 	for (std::vector<ServerInfo>::iterator it = servers.begin(); it != servers.end(); ++it)
// 	{
// 		std::vector<int> ports = it->getPortList();
// 		for (std::vector<int>::iterator portIt = ports.begin(); portIt != ports.end(); ++portIt)
// 			std::cout << BG_CYAN_BLACK << timestamp << RESET << " Listening on http://127.0.0.1:" << CYAN << *portIt << RESET;
// 		std::cout << std::endl;
// 	}

// 	while (1)
// 	{
// 		fd_set temp_fds = read_fds;
// 		if (select(max_fd + 1, &temp_fds, NULL, NULL, NULL) < 0)
// 		{
// 			perror("Error on select");
// 			exit(EXIT_FAILURE);
// 		}

// 		for (std::vector<ServerInfo>::iterator it = servers.begin(); it != servers.end(); ++it)
// 		{
// 			int sockfd = it->getSocketFD();
// 			if (FD_ISSET(sockfd, &temp_fds))
// 			{
// 				sockaddr_in cli_addr;
// 				socklen_t clilen = sizeof(cli_addr);
// 				int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
// 				if (newsockfd < 0)
// 				{
// 					perror("Error on accept");
// 					exit(EXIT_FAILURE);
// 				}

// 				std::string request = readRequest(newsockfd);

// 				//=================
// 				it->clientSocket = newsockfd;
// 				//=================

// 				processRequest(request, *it);
// 				write(newsockfd, it->getResponse().c_str(), it->getResponse().length());
// 				close(newsockfd);
// 			}
// 		}
// 	}
// }



// #include <vector>
// #include <iostream>
// #include <poll.h>
// #include <ctime>
// #include <cstring>
// #include <unistd.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <algorithm>

// // Functor para encontrar um servidor pelo socket FD
// struct FindServerBySocketFD
// {
//     int fd;
//     FindServerBySocketFD(int fd) : fd(fd) {}
//     bool operator()(const ServerInfo& server) const
//     {
//         return server.getSocketFD() == fd;
//     }
// };

// // Functor para encontrar um servidor pelo client socket
// struct FindServerByClientSocket
// {
//     int fd;
//     FindServerByClientSocket(int fd) : fd(fd) {}
//     bool operator()(ServerInfo& server) const
//     {
//         return server.clientSocket == fd;
//     }
// };

// // Functor para remover pollfd com fd == -1
// struct RemovePollfdWithNegativeFD
// {
//     bool operator()(const struct pollfd& pfd) const
//     {
//         return pfd.fd == -1;
//     }
// };

// void runServer(std::vector<ServerInfo>& servers)
// {
//     std::vector<struct pollfd> pollfds;
//     for (std::vector<ServerInfo>::iterator it = servers.begin(); it != servers.end(); ++it)
//     {
//         struct pollfd pfd;
//         pfd.fd = it->getSocketFD();
//         pfd.events = POLLIN;
//         pollfds.push_back(pfd);
//     }

//     std::cout << "\n<" << GREEN << "=+=+=+=+=+=+=+=+=+=" << RESET << " Waiting for client "
//               << GREEN << "=+=+=+=+=+=+=+=+=+=" << RESET << ">\n" << std::endl;

//     time_t now = time(NULL);
//     char timestamp[100];
//     strftime(timestamp, sizeof(timestamp), "[%d/%b/%Y %T]", localtime(&now));

//     for (std::vector<ServerInfo>::iterator it = servers.begin(); it != servers.end(); ++it)
//     {
//         std::vector<int> ports = it->getPortList();
//         for (std::vector<int>::iterator portIt = ports.begin(); portIt != ports.end(); ++portIt)
//             std::cout << BG_CYAN_BLACK << timestamp << RESET << " Listening on http://127.0.0.1:" << CYAN << *portIt << RESET;
//         std::cout << std::endl;
//     }

//     while (1)
//     {
//         int ret = poll(&pollfds[0], pollfds.size(), -1);
//         if (ret < 0)
//         {
//             perror("Error on poll");
//             exit(EXIT_FAILURE);
//         }

//         for (std::vector<struct pollfd>::iterator pfdIt = pollfds.begin(); pfdIt != pollfds.end(); ++pfdIt)
//         {
//             if (pfdIt->revents & POLLIN)
//             {
//                 std::vector<ServerInfo>::iterator it = std::find_if(servers.begin(), servers.end(), FindServerBySocketFD(pfdIt->fd));

//                 if (it != servers.end())
//                 {
//                     sockaddr_in cli_addr;
//                     socklen_t clilen = sizeof(cli_addr);
//                     int newsockfd = accept(pfdIt->fd, (struct sockaddr *)&cli_addr, &clilen);
//                     if (newsockfd < 0)
//                     {
//                         perror("Error on accept");
//                         exit(EXIT_FAILURE);
//                     }

//                     std::string request = readRequest(newsockfd);
//                     it->clientSocket = newsockfd;
//                     processRequest(request, *it);

//                     struct pollfd client_pfd;
//                     client_pfd.fd = newsockfd;
//                     client_pfd.events = POLLOUT;
//                     pollfds.push_back(client_pfd);
//                 }
//             }

//             if (pfdIt->revents & POLLOUT)
//             {
//                 std::vector<ServerInfo>::iterator it = std::find_if(servers.begin(), servers.end(), FindServerByClientSocket(pfdIt->fd));

//                 if (it != servers.end())
//                 {
//                     int clientSocket = it->clientSocket;
//                     write(clientSocket, it->getResponse().c_str(), it->getResponse().length());

//                     close(clientSocket);
//                     it->clientSocket = -1;

//                     pfdIt->fd = -1;
//                 }
//             }
//         }

//         std::vector<struct pollfd>::iterator new_end = std::remove_if(pollfds.begin(), pollfds.end(), RemovePollfdWithNegativeFD());

//         pollfds.erase(new_end, pollfds.end());
//     }
// }
