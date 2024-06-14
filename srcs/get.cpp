/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fde-carv <fde-carv@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/26 15:10:07 by fde-carv          #+#    #+#             */
/*   Updated: 2024/06/14 20:07:34 by fde-carv         ###   ########.fr       */
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
	//this->bytesReadTotal = 0;
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
void handleError(const std::string& errorMessage) //, int errorCode)
{
	std::cerr << RED1 << "\n" << errorMessage << RESET << std::endl;
	//exit(-1);
}

// check if the path is a directory
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
    
    std::cout << " @@@@@@ Full path: " << modifiedPath << std::endl;
    DIR *dir = opendir(modifiedPath.c_str());
    if (dir)
    {
        std::cout << " @@@@@@ Directory " << modifiedPath << " exists.\n";
        closedir(dir);
        return true;
    }
    else
    {
        std::cout << " @@@@@@ Directory " << modifiedPath << " does not exist.\n";
        return false;
    }
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

// void setupDirectory(ServerInfo& server, const conf_File_Info& config)
// {
// 	//chmod("/resources/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

// 	std::string rootDir = config.RootDirectory;
// 	std::string rootUrl = server.getRootUrl();
// 	if (rootDir.substr(0, rootUrl.length()) != rootUrl)
// 	{
// 		handleError("Error: Root URL should start with 'resources' directory.");
// 		exit(-1);
// 	}

// 	std::string subDir = rootDir.substr(rootUrl.length());

// 	if (!subDir.empty() && subDir[0] == '/')
// 		subDir = subDir.substr(1);

// 	std::string path = "";
// 	std::stringstream ss(rootDir);
// 	std::string token;

// 	while (std::getline(ss, token, '/'))
// 	{
// 		path += token + "/";
// 		if (!is_directory(path))
// 		{
// 			if (mkdir(path.c_str(), 0777) == -1)
// 			{
// 				perror("Error creating directory");
// 				exit(EXIT_FAILURE);
// 			}

// 			if (chmod(path.c_str(), 0777) == -1)
// 			{
// 				perror("Error changing directory permissions");
// 				exit(EXIT_FAILURE);
// 			}
// 		}
// 	}
// }


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
	std::cout << " +++++ Server Root: " << serverRoot << std::endl;
}


void ServerInfo::handleRedirectRequest(HTTrequestMSG& request, ServerInfo& server)
{

	std::vector<int> ports = server.getPortList();
	int listeningPort = ports[0];
	conf_File_Info &serverConfig = server.getConfig(listeningPort);

	
	std::cout << "  ==>>>>> httpStatusCode: " << serverConfig.redirectURL.httpStatusCode << std::endl;
	std::cout << "  ==>>>>> destinationURL: " << serverConfig.redirectURL.destinationURL << std::endl;
	if (serverConfig.redirectURL.destinationURL == "/redirect")
	{
		std::string newUrl = serverConfig.redirectURL.destinationURL;
		std::stringstream ss;
		ss << serverConfig.redirectURL.httpStatusCode;
		std::string response = request.version + 
							ss.str() +
							"Moved Permanently\r\n"
							"Location: " + newUrl + "\r\n"
							"\r\n";
		server.setResponse(response); 
	}
	
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
			perror("recv");
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


	size_t headerEnd = request.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
    {
        // Handle error: The request does not contain a valid HTTP header
    }




	

	// Read the Body
	HTTPParser parser;
	size_t contentLength = parser.getContentLength(request);
	// int raw = parser.get
	server.setContentLength(contentLength);
	//std::cout << "  Content-Length1: " << contentLength << std::endl; 
	
	size_t actualDataSize = request.size();
	size_t headerSize = request.find("\r\n\r\n") + 4;
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
				handleError("Error reading from socket2.");
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


// void processRulesRequest(ServerInfo& server)
// {
//     std::vector<int> ports = server.getPortList();
//     int listeningPort = ports[0];
//     conf_File_Info &serverConfig = server.getConfig(listeningPort);
//     for (Locations::const_iterator it = serverConfig.LocationsMap.begin(); it != serverConfig.LocationsMap.end(); ++it) {
//         std::cout << "LOCATION: " << it->first << std::endl;
//         std::cout << "Value (portListen): " << it->second.portListen << std::endl;
//         std::cout << "Value (ServerName): " << it->second.ServerName << std::endl;
//         std::cout << "Value (RootDirectory): " << it->second.RootDirectory << std::endl;
		
// 		// Adicione mais campos conforme necessário
//         std::cout << "std::set<std::string> allowedMethods: ";
//         for (std::set<std::string>::const_iterator method_it = it->second.allowedMethods.begin(); method_it != it->second.allowedMethods.end(); ++method_it) {
//             std::cout << *method_it << " ";
//         }
//         std::cout << std::endl;
//     }
// }

void printServerConfig(const conf_File_Info &serverConfig) {
	// ------- DENTRO DO PRINT ---------

	std::cout << BOLD << "int portListen: " << RESET << serverConfig.portListen << std::endl;
	std::cout << BOLD << "std::string ServerName: " << RESET << serverConfig.ServerName << std::endl;
	std::cout << BOLD << "std::string defaultFile: " << RESET << serverConfig.defaultFile << std::endl;
	std::cout << BOLD << "std::string RootDirectory: " << RESET << serverConfig.RootDirectory << std::endl;
	std::cout << BOLD << "std::string Path_CGI: " << RESET << serverConfig.Path_CGI << std::endl;
	std::cout << BOLD << "bool directoryListingEnabled: " << RESET << (serverConfig.directoryListingEnabled ? "true" : "false") << std::endl;
	std::cout << BOLD << "std::map<int, std::string> errorMap: " << RESET << std::endl;
	for (std::map<int, std::string>::const_iterator it = serverConfig.errorMap.begin(); it != serverConfig.errorMap.end(); ++it) {
		std::cout << "  " << it->first << " -> " << it->second << std::endl;
	}
	std::cout << BOLD << "ForwardingURL redirectURL: " << RESET << std::endl;
	std::cout << "  httpStatusCode: " << serverConfig.redirectURL.httpStatusCode << std::endl;
	std::cout << "  destinationURL: " << serverConfig.redirectURL.destinationURL << std::endl;
	std::cout << BOLD << "std::set<std::string> allowedMethods: " << RESET;
	for (std::set<std::string>::const_iterator it = serverConfig.allowedMethods.begin(); it != serverConfig.allowedMethods.end(); ++it) {
		std::cout << *it << " ";
	}
	std::cout << std::endl;
	std::cout << BOLD << "int maxRequestSize: " << RESET << serverConfig.maxRequestSize << std::endl;
	std::cout << BOLD << "std::string fileUploadDirectory: " << RESET << serverConfig.fileUploadDirectory << std::endl;
	std::cout << BOLD << "Locations LocationsMap: " << RESET << std::endl;
	for (Locations::const_iterator it = serverConfig.LocationsMap.begin(); it != serverConfig.LocationsMap.end(); ++it) {
		std::cout << BOLD << "==>LOCATION: " << RESET << it->first << std::endl;
		std::cout << "Value (portListen): " << RESET << it->second.portListen << std::endl;
		std::cout << "Value (ServerName): " << RESET << it->second.ServerName << std::endl;
		std::cout << "Value (RootDirectory): " << RESET << it->second.RootDirectory << std::endl;
		std::cout << "Value (defaultFile): " << RESET << it->second.defaultFile << std::endl;
		std::cout << "Value (Path_CGI): " << RESET << it->second.Path_CGI << std::endl;
		std::cout << "Value (directoryListingEnabled): " << RESET << (it->second.directoryListingEnabled ? "true" : "false") << std::endl;
		std::cout << "Value (errorMap): " << RESET << std::endl;
		for (std::map<int, std::string>::const_iterator it_err = it->second.errorMap.begin(); it_err != it->second.errorMap.end(); ++it_err) {
			std::cout << "  " << it_err->first << " -> " << it_err->second << std::endl;
		}
		std::cout << "Value (redirectURL): " << RESET << std::endl;
		std::cout << "  httpStatusCode: " << it->second.redirectURL.httpStatusCode << std::endl;
		std::cout << "  destinationURL: " << it->second.redirectURL.destinationURL << std::endl;
		std::cout << "Value (allowedMethods): " << RESET;
		for (std::set<std::string>::const_iterator it_meth = it->second.allowedMethods.begin(); it_meth != it->second.allowedMethods.end(); ++it_meth) {
			std::cout << *it_meth << " ";
		}
		std::cout << std::endl;
	}

	
	// ----------- FORA DO PRINT -------------------
}

// void getAllowedMethods(const char* allowedMethods)
// {
//     // Print the allowedMethods string before splitting
//     std::cout << "Allowed methods string: " << allowedMethods << std::endl;

//     char* methods = new char[strlen(allowedMethods) + 1];
//     std::strcpy(methods, allowedMethods);

//     char* method = std::strtok(methods, " ");
//     while (method != NULL) {
//         std::cout << method << '\n';
//         method = std::strtok(NULL, " ");
//     }

//     delete[] methods;
// }




// std::string getNewPath(const std::string& root, const std::string& path) {
// 	std::istringstream rootStream(root);
// 	std::string mutableRoot = root; // Create a copy of root that we can modify
// 	std::string mutablePath = path; // Create a copy of path that we can modify
// 	std::istringstream pathStream(mutablePath);
// 	std::string rootDir, pathDir, commonPart, differentPart;

// 	std::cout << "[getNewPath] root: " << mutableRoot << std::endl;
// 	std::cout << "[getNewPath] path: " << mutablePath << std::endl;

// 	if (!mutableRoot.empty() && mutableRoot[0] != '/'){
// 		mutableRoot = "/" + mutableRoot;
// 	}

// 	std::cout << "[getNewPath] root after pre /: " << mutableRoot << std::endl;
// 	std::cout << "[getNewPath] pathDir1: " << pathDir << std::endl;

// 	while (std::getline(rootStream, rootDir, '/') && std::getline(pathStream, pathDir, '/') && rootDir == pathDir) {
// 		commonPart += rootDir + "/";
// 	}

// 	std::cout << "[getNewPath] commonPart: " << commonPart << std::endl;

// 	if (!rootDir.empty() && !pathDir.empty()) {
// 	differentPart = rootDir + "/" + pathDir;
// 	} else {
// 		differentPart = rootDir + pathDir;
// 	}

// 	// differentPart = rootDir + pathDir;
// 	std::cout << "[getNewPath] pathDir2: " << pathDir << std::endl;

// 	while (std::getline(pathStream, pathDir, '/')) {
// 		differentPart += "/" + pathDir;
// 	}

// 	std::cout << "differentPart: " << differentPart << std::endl;

// 	std::string totalPath = commonPart + differentPart;

// 	if (totalPath.at(0) != '/')
// 	{
// 		totalPath = '/' + totalPath;
// 	}

// 	std::cout << "totalPath: " << totalPath << std::endl;

// 	return totalPath;
// }

std::vector<std::string> tokenize(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);

    while (std::getline(tokenStream, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }

    //std::cout << "Tokens: ";
    // for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
    //     std::cout << *it << " ";
    // }
    std::cout << std::endl;

    return tokens;
}

std::string getNewPath(const std::string& root, const std::string& path) {
    std::vector<std::string> rootTokens = tokenize(root, '/');
    std::vector<std::string> pathTokens = tokenize(path, '/');

    std::string newPath;
    std::string initialPath;
    size_t i = 0, j = 0;
    bool hasMatched = false;

    // Find common part
    while (i < rootTokens.size() && j < pathTokens.size()) {
        //std::cout << "==> Comparing rootTokens[" << i << "]: " << rootTokens[i] << " with pathTokens[" << j << "]: " << pathTokens[j] << std::endl;
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

    //std::cout << "==> Initial path: " << initialPath << std::endl;
   // std::cout << "==> Final path: " << newPath << std::endl;
    return initialPath + newPath;
}

std::string getDirectoryPath(const std::string& fullPath) {
	size_t found = fullPath.find_last_of("/\\");
	return fullPath.substr(0,found);
}

std::string getDirectoryPath2(const std::string& fullPath) {
	if (fullPath.empty() || fullPath[0] != '/') {
		// fullPath is empty or does not start with "/", return an empty string
		return "";
	}
	size_t start = 1; // Skip the first "/"
	size_t end = fullPath.find("/", start);
	if (end == std::string::npos) {
		// "/" not found after the first "/", return the substring from the first "/" to the end of the fullPath
		return fullPath.substr(0, fullPath.length());
	}
	// Return the substring from the first "/" to the next "/"
	return fullPath.substr(0, end);
}

bool isMethodAllowed(const std::set<std::string>& allowedMethods, const std::string& requestMethod)
{
	if (allowedMethods.empty()) {
		return true; // Todos os métodos são permitidos se o conjunto de métodos permitidos estiver vazio
	}
	
	std::string upperRequestMethod = requestMethod;
	std::transform(upperRequestMethod.begin(), upperRequestMethod.end(), upperRequestMethod.begin(), ::toupper);


	for (std::set<std::string>::const_iterator it_meth = allowedMethods.begin(); it_meth != allowedMethods.end(); ++it_meth)
	{
		std::string allowedMethods = *it_meth;
		std::cout << MAGENTA << "Current allowedMethods string: " << RESET << *it_meth << std::endl;
		std::cout << MAGENTA << "Checking method: " << RESET << allowedMethods << std::endl;
		std::cout << MAGENTA << "Request method: " << RESET << requestMethod << std::endl;
		
		std::string allowedMethod = *it_meth;
		std::transform(allowedMethod.begin(), allowedMethod.end(), allowedMethod.begin(), ::toupper);

		if (allowedMethod == upperRequestMethod)
		{
			return true;
		}
	}

	return false;
}



std::string ServerInfo::getCompletePath(const std::string& path)
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    std::string full_path2 = std::string(cwd);

    // Ensure there is a '/' between the cwd and the path
    if (!path.empty() && path[0] != '/') {
        full_path2 += '/';
    }

    full_path2 += path;
    std::cout << "[getCompletePath] @@ Full path: " << full_path2 << std::endl;
    
    return full_path2;
}

std::string ifFileRmoveFile(std::string path) {
    // Remove trailing slash if it exists
    if (!path.empty() && path[path.size() - 1] == '/') {
        path.erase(path.size() - 1);
    }

    std::string::size_type lastSlashPos = path.find_last_of('/');
    std::string lastPart = path.substr(lastSlashPos + 1);

    // Check if the last part of the path is a file
    if (lastPart.find('.') != std::string::npos) {
        // It's a file, remove the file from the path
        path = path.substr(0, lastSlashPos);
    }

    std::cout << "[isFileAndRemoveTrailingSlash] path: " << path << std::endl;
    return path;
}

std::string removeLastSlash(const std::string& fullPath) {
    if (fullPath == "/") {
        return fullPath;
    }
    if (!fullPath.empty() && fullPath[fullPath.size() - 1] == '/') {
        return fullPath.substr(0, fullPath.size() - 1);
    }
    return fullPath;
}

bool handleDirectoryListing(conf_File_Info& serverConfig, HTTrequestMSG& requestMsg, ServerInfo& server)
{
    if(serverConfig.directoryListingEnabled)
    {
        std::string rootDirectory = serverConfig.RootDirectory;
        if (!rootDirectory.empty() && rootDirectory[0] == '/')
        {
            rootDirectory = rootDirectory.substr(1);
        }

        //std::string fullPath = rootDirectory + requestMsg.path;
        std::string fullPath = server.getCompletePath2();
		std::cout << "[AUTOINDEX] rootDirectory PATH: " << rootDirectory << std::endl;
        std::cout << "[AUTOINDEX] requestMSG PATH original: " << requestMsg.path << std::endl;
        std::cout << "[AUTOINDEX] Full path: " << fullPath << std::endl;

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
        if(stat(full_path2.c_str(), &path_stat) != 0) {
            std::cerr << "Error accessing " << full_path2 << ": " << strerror(errno) << std::endl;
            server.setResponse("HTTP/1.1 500 Internal Server Error\r\n\r\n");
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
                perror ("");
                response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
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
                server.setResponse("HTTP/1.1 404 Not Found AUTOINDEX\r\n\r\n");
            }
        }
    }
    return true;
}

bool processRulesRequest(HTTrequestMSG& requestMsg, ServerInfo& server)
{
	std::vector<int> ports = server.getPortList();
	int listeningPort = ports[0];
	conf_File_Info &serverConfig = server.getConfig(listeningPort);

	
	std::map<std::string, conf_File_Info> locationConfigs = serverConfig.LocationsMap;

	// for (Locations::const_iterator it = locationConfigs.begin(); it != locationConfigs.end(); ++it)
	// {
	// 	std::cout << "LOCATION dentro : " << it->first << std::endl;
	// 	std::cout << "Value (portListen): " << it->second.portListen << std::endl;	
	// }

	//printServerConfig(server.getConfig(listeningPort));

	//bool methodAllowed = false;
	std::string ccc = removeLastSlash(requestMsg.path);
	std::cout << "$$ >>>>> requestMsg.path: " << ccc << std::endl;
	std::cout << "$$ >>>>> requestMsg.method: " << methodToString(requestMsg.method) << std::endl;


	// std::string fred = getNewPath(serverConfig.RootDirectory,serverConfig.RootDirectory);
	// std::string ddd = server.getCompletePath(fred);
	// if(requestMsg.path == "/")
	// 	requestMsg.path = "";	
	// std::string eee = ddd + requestMsg.path;
	// std::cout << YELLOW << eee << RESET << std::endl;




	// std::string fred = getNewPath(serverConfig.RootDirectory, it->second.RootDirectory);
	// std::string bbb = server.getCompletePath(fred);
	// if(requestMsg.path == "/")
	// 	requestMsg.path = "";	
	// std::string aaa = bbb + requestMsg.path;
	// std::cout << MAGENTA << aaa << RESET << std::endl;



	//std::cout << "------ AUTOINDEX -------" << std::endl;
	//std::cout << BOLD << "bool directoryListingEnabled: " << RESET << (serverConfig.directoryListingEnabled ? "true" : "false") << std::endl;
	// if(serverConfig.directoryListingEnabled == true)
	// {

	// 	//std::string fullPath = "resources/website" + requestMsg.path;
	// 	std::string rootDirectory = serverConfig.RootDirectory;
	// 	if (!rootDirectory.empty() && rootDirectory[0] == '/')
	// 	{
	// 		rootDirectory = rootDirectory.substr(1);
	// 	}
		
	// 	std::string requestMsgFile = removeTrailingSlash(requestMsg.path);
	// 	std::string requestMsgFile2 = removeFirstDirectory(requestMsgFile);
		
	// 	std::string fullPath = rootDirectory + requestMsgFile2;
	// 	std::cout << "[AUTOINDEX] rootDirectory PATH: " << rootDirectory << std::endl;
	// 	std::cout << "[AUTOINDEX] requestMSG PATH original: " << requestMsg.path << std::endl;
	// 	std::cout << "[AUTOINDEX] requestMSG PATH1: " << requestMsgFile << std::endl;
	// 	std::cout << "[AUTOINDEX] requestMSG PATH2: " << requestMsgFile2 << std::endl;
	// 	std::cout << "[AUTOINDEX] Full path: " << fullPath << std::endl;
		

	// 	char cwd[1024];
	// 	getcwd(cwd, sizeof(cwd));
	// 	std::string full_path2 = std::string(cwd) + "/" + fullPath;
		
	// 	struct stat path_stat;
	// 	stat(full_path2.c_str(), &path_stat);
	// 	bool is_directory = S_ISDIR(path_stat.st_mode);
		
	// 	std::string response;
	// 	if (is_directory)
	// 	{
	// 		DIR *dir;
	// 		struct dirent *ent;
	// 		if ((dir = opendir (full_path2.c_str())) != NULL)
	// 		{
	// 			response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
	// 			response += "<html><head><style>body { background: #ADD8E6; }</style></head><body>";
	// 			while ((ent = readdir (dir)) != NULL)
	// 			{
	// 				response += ent->d_name;
	// 				response += "<br>";
	// 				//std::cout << ent->d_name << std::endl;
	// 			}
	// 			response += "<button onclick=\"location.href='index.html'\" type=\"button\">Go Home</button>";
	// 			response += "</body></html>\n";
	// 			closedir (dir);
	// 		}
	// 		else
	// 		{
	// 			perror ("");
	// 			response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
	// 		}
	// 	}
	// 	else
	// 	{
	// 		//The path is a file, not a directory.
	// 		//Try to open the file and send it as the response.
	// 		std::ifstream file(full_path2.c_str());
	// 		if (file.is_open())
	// 		{
	// 			response = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    //     		server.setResponse(response);
	// 		}
	// 		else
	// 		{
	// 			server.setResponse("HTTP/1.1 404 Not Found AUTOINDEX\r\n\r\n");
	// 		}
	// 	}
	// 	std::cout << "AUTOINDEX response: " << response << std::endl;	 
	// 	server.setResponse(response);
	// 	return true;
	// }

	
	if (serverConfig.LocationsMap.size() > 0)
	{
		for (Locations::const_iterator it = serverConfig.LocationsMap.begin(); it != serverConfig.LocationsMap.end(); ++it)
		{
			std::cout << "$ >>>>> it->first: " << it->first << std::endl;	
			if (it->first == ccc)
			{
				if(it->first == "/")
				{
					std::cout << "ENTREI no /"	<< std::endl;
					//if (serverConfig.RootDirectory != it->second.RootDirectory)
					//{

					std::cout << "[processRules]: "<< requestMsg.path << std::endl;


					std::string fred = getNewPath(serverConfig.RootDirectory, it->second.RootDirectory);
					std::string bbb = server.getCompletePath(fred);
					if(requestMsg.path == "/")
						requestMsg.path = "";	
					std::string aaa = bbb + requestMsg.path;
					std::cout << YELLOW << aaa << RESET << std::endl;

					//std::cout << YELLOW << "First directory: " << getFirstDirectory(serverConfig.RootDirectory) << RESET << std::endl;
					//serverConfig.RootDirectory = "/" + getNewPath(serverConfig.RootDirectory, it->second.RootDirectory);
						//serverConfig.RootDirectory = "/" + serverConfig.RootDirectory;
					//std::cout << " ## Root directory1 : " << serverConfig.RootDirectory << std::endl;
					//std::cout << " ## Root directory2 : " << it->second.RootDirectory << std::endl;
						//serverConfig.RootDirectory = getNewPath(serverConfig.RootDirectory, it->second.RootDirectory);
					//serverConfig.RootDirectory = "/" + getFirstDirectory(serverConfig.RootDirectory) + it->second.RootDirectory;
					//std::cout << " ## Root directory changed to: " << serverConfig.RootDirectory << std::endl;
					
					//serverConfig.RootDirectory = aaa;
					std::cout << " ## Root directory changed to: " << serverConfig.RootDirectory << std::endl;
					server.setCompletePath(aaa);
					std::cout << " ## CompletePath: " << server.getCompletePath2() << std::endl;
					std::cout << " ## is_directory?: " << !is_directory(serverConfig.RootDirectory) << std::endl;
					// if (!is_directory(serverConfig.RootDirectory))
					// {
					// 	std::cerr << "Root directory does not exist: " << serverConfig.RootDirectory << std::endl;
					// 	server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
					// 	requestMsg.path = serverConfig.RootDirectory + " is not found";
					// 	requestMsg.version = "";
					// 	printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
					// 	return false;
					// }

					std::string requestMethod = methodToString(requestMsg.method);
					//std::transform(requestMethod.begin(), requestMethod.end(), requestMethod.begin(), ::toupper);//mudar para maiusculas
					bool methodAllowed = isMethodAllowed(it->second.allowedMethods, requestMethod);
					if (!methodAllowed)
					{
						std::cerr << "Error: Forbidden method." << std::endl;
						server.setResponse("HTTP/1.1 403 Forbidden\r\nContent-Type: text/plain\r\n\r\nMethod is Forbidden\nERROR 403\n");
						requestMsg.path = "Forbidden";
						requestMsg.version = "";
						printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
						return false;
					}
					return true;
					//}
				}
				std::cout << "it antes redirect: " << it->first << std::endl;
				if(it->first == "/redirect")
				{
					std::cout << "ENTREI NO REDIRECT" << std::endl;
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
					std::cout << "CONTINUANDO NO REDIRECT" << std::endl;
					printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
					//std::cout << "FIM DO REDIRECT" << std::endl;
					// std::cout << "[processRules]: "<< requestMsg.path << std::endl;
					// handleRequest(requestMsg, server);
					return false;
				}
				std::string requestMethod = methodToString(requestMsg.method);
				//std::transform(requestMethod.begin(), requestMethod.end(), requestMethod.begin(), ::toupper);//mudar para maiusculas
				bool methodAllowed = isMethodAllowed(it->second.allowedMethods, requestMethod);
				if (!methodAllowed)
				{
					std::cerr << "Error: Forbidden method." << std::endl;
					server.setResponse("HTTP/1.1 403 Forbidden\r\nContent-Type: text/plain\r\n\r\nMethod is Forbidden\nERROR 403\n");
					requestMsg.path = "Forbidden";
					requestMsg.version = "";
					printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
					return false;
				}
				
				// int bodySizeBytes = server.getContentLength();
				// std::cout << "  Content-Length : " << bodySizeBytes << std::endl;
				// bodySizeBytes -= 200;
				// std::cout << "  Content-Length ALTERADO: " << bodySizeBytes << std::endl;
				// std::cout << "  static cast: " << serverConfig.maxRequestSize << std::endl;
				// if(bodySizeBytes > serverConfig.maxRequestSize)
				// {
				// 	std::cerr << "Error: Request size exceeds the maximum allowed size." << std::endl;
				// 	server.setResponse("HTTP/1.1 413 Request Entity Too Large\r\nContent-Type: text/plain\r\n\r\nRequest size exceeds the maximum allowed size\nERROR 413\n");
				// 	requestMsg.path = "Request Entity Too Large";
				// 	requestMsg.version = "";
				// 	printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
				// 	return false;
				// }
				
				std::string fred = getNewPath(serverConfig.RootDirectory, it->second.RootDirectory);
				std::string zzz = server.getCompletePath(fred);
				if(requestMsg.path == "/")
					requestMsg.path = "";	
				std::string xxx = zzz + requestMsg.path;
				std::cout << MAGENTA << xxx << RESET << std::endl;
				server.setCompletePath(xxx);
				std::cout << "OLA\n";
				if (is_directory(xxx))
				{

					std::cout << "ENTREI Na directoria (IF)" << std::endl;
					if(serverConfig.directoryListingEnabled == true)
					{
						std::cout << it->second.directoryListingEnabled << std::endl;
						if(it->second.directoryListingEnabled == true)
						{
							if((handleDirectoryListing(serverConfig, requestMsg, server)))
							{
								// std::cerr << "Root directory does not exist: " << serverConfig.RootDirectory << std::endl;
								// server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
								// requestMsg.path = serverConfig.RootDirectory + " is not found";
								// requestMsg.version = "";
								//printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
								return true;
							}
							std::cout << "SAI NO IF" << std::endl;
						}
					
					}

					
				}


				

			}
			else if (it->first == getDirectoryPath2(requestMsg.path))
			{
				std::cout << " \n+++++++++++++++++ ENTREI NO ELSE IF +++++++++++++++" << std::endl;
				// std::string fred = getNewPath(serverConfig.RootDirectory, it->second.RootDirectory);
				// std::string aaa = server.getCompletePath(fred);
				// aaa = aaa + requestMsg.path;
				// std::cout << GREEN <<  aaa << RESET << std::endl;

				std::string fred = getNewPath(serverConfig.RootDirectory, it->second.RootDirectory);
				std::string bbb = server.getCompletePath(fred);	
				std::string aaa = bbb + requestMsg.path;
				std::cout << GREEN <<  aaa << RESET << std::endl;

				serverConfig.RootDirectory = aaa;
				std::cout << " ## Root directory changed to: " << serverConfig.RootDirectory << std::endl;
				server.setCompletePath(aaa);
				std::cout << " ## CompletePath: " << server.getCompletePath2() << std::endl;
				

				
				
				int bodySizeBytes = server.getContentLength();
				//std::cout << "  Content-Length : " << bodySizeBytes << std::endl;
				bodySizeBytes -= 200;
				//std::cout << "  Content-Length ALTERADO: " << bodySizeBytes << std::endl;
				//std::cout << "  static cast: " << serverConfig.maxRequestSize << std::endl;
				if(bodySizeBytes > serverConfig.maxRequestSize)
				{
					std::cerr << "Error: Request size exceeds the maximum allowed size." << std::endl;
					server.setResponse("HTTP/1.1 413 Request Entity Too Large\r\nContent-Type: text/plain\r\n\r\nRequest size exceeds the maximum allowed size\nERROR 413\n");
					requestMsg.path = "Request Entity Too Large";
					requestMsg.version = "";
					printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
					return false;
				}			


				// if(it->first == "/redirect")
				// {
				// 	std::cout << "ENTREI NO REDIRECT" << std::endl;
				// 	std::string newUrl = serverConfig.redirectURL.destinationURL;
				// 	std::cout << "  newUrl: " << newUrl << std::endl;
				// 	std::stringstream ss;
				// 	ss << serverConfig.redirectURL.httpStatusCode;
				// 	std::cout << "  ss: " << ss.str() << std::endl;
				// 	std::string response = requestMsg.version + 
				// 						ss.str() +
				// 						"Moved Permanently\r\n"
				// 						"Location: " + newUrl + "\r\n"
				// 						"\r\n";
				// 	server.setResponse(response); 
				// 	std::cout << "CONTINUANDO NO REDIRECT" << std::endl;
				// 	printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
				// 	//std::cout << "FIM DO REDIRECT" << std::endl;
				// 	return false;
				// }


			



				
				//std::string fred = getNewPath(serverConfig.RootDirectory, it->second.RootDirectory);
				//std::cout << " [elseif] it->first: " << it->first << std::endl;
				//std::cout << " [elseif] requestMsg: " << requestMsg.path << std::endl;
				serverConfig.RootDirectory  = getNewPath(serverConfig.RootDirectory, it->second.RootDirectory);
				std::cout << "    ## Root FRED : " << serverConfig.RootDirectory << std::endl;

				std::string requestMethod = methodToString(requestMsg.method);
				//std::transform(requestMethod.begin(), requestMethod.end(), requestMethod.begin(), ::toupper);//mudar para maiusculas
				bool methodAllowed = isMethodAllowed(it->second.allowedMethods, requestMethod);
				if (!methodAllowed)
				{
					std::cerr << "Error: Forbidden method." << std::endl;
					server.setResponse("HTTP/1.1 403 Forbidden\r\nContent-Type: text/plain\r\n\r\nMethod is Forbidden\nERROR 403\n");
					requestMsg.path = "Forbidden";
					requestMsg.version = "";
					printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
					return false;
				}



				
				return true;
			}
			else
			{
				//implmentar /dfsf/index.hlml que devera dar erro
			}
			std::string requestMethod = methodToString(requestMsg.method);
			std::transform(requestMethod.begin(), requestMethod.end(), requestMethod.begin(), ::toupper); // mudar para maiusculas
		}
		
		
	}
	// else if (!is_directory("/" + serverConfig.RootDirectory))
	// {
	// 	std::cerr << "Root directory does not exist: " << serverConfig.RootDirectory << std::endl;
	// 	server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
	// 	requestMsg.path = serverConfig.RootDirectory + " is not found";
	// 	requestMsg.version = "";
	// 	printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
	// 	return false;
	// }






	// std::string fred = getNewPath(serverConfig.RootDirectory,serverConfig.RootDirectory);
	// std::string bbb = server.getCompletePath(fred);
	// if(requestMsg.path == "/")
	// 	requestMsg.path = "";	
	// std::string aaa = bbb + requestMsg.path;
	// std::cout << BLUE << aaa << RESET << std::endl;
	std::cout << "\nFIM\n\n"; 

	std::cout << " ## Root directory END**: " << serverConfig.RootDirectory << std::endl;
	std::cout << " ## requestMsg.path END**: " << requestMsg.path << std::endl;
	std::string aaa = serverConfig.RootDirectory + requestMsg.path;
	std::cout << " ## Path directory changed to: " << aaa << std::endl;
	std::string bbb = server.getCompletePath(aaa);
	//ifFileRmoveFile(bbb);
	server.setCompletePath(bbb);
	aaa = ifFileRmoveFile(aaa);
	serverConfig.RootDirectory = aaa;
	std::cout << " ## ROOT directory changed to: " << aaa << std::endl;
	std::cout << " ## [FINAL] CompletePath: " << server.getCompletePath2() << std::endl;	

	std::cout << " ## is_directory?: " << !is_directory(aaa) << std::endl;
	if (!is_directory(aaa) && methodToString(requestMsg.method) != "POST")
	{
		std::cerr << "Root directory does not exist: " << serverConfig.RootDirectory << std::endl;
		server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
		requestMsg.path = serverConfig.RootDirectory + " is not found";
		requestMsg.version = "";
		printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
		return false;
	}
	

	

	

	//std::string fred = getNewPath(serverConfig.RootDirectory, it->second.RootDirectory);
	// std::string aaa = server.getCompletePath(serverConfig.RootDirectory);
	// //aaa = aaa + requestMsg.path;
	// std::cout << BLUE <<  aaa << RESET << std::endl;



	
	
	// if(server.getContentLength() > serverConfig.maxRequestSize)
	// {
	// 	std::cerr << "Error: Request size exceeds the maximum allowed size." << std::endl;
	// 	server.setResponse("HTTP/1.1 413 Request Entity Too Large\r\nContent-Type: text/plain\r\n\r\nRequest size exceeds the maximum allowed size\nERROR 413\n");
	// 	requestMsg.path = "Request Entity Too Large";
	// 	requestMsg.version = "";
	// 	printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
	// 	return false;
	// }

	return true;
}

// Process the request and send the response
void processRequest(const std::string& request, ServerInfo& server)
{
	if (request.empty())
	{
		//std::cout << "Received empty request, ignoring." << std::endl;
		return;
	}

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
			std::cerr << "Error: No ports found." << std::endl;
			return;
		}

		// Use the first port in the list
		int listeningPort = ports[0];
		conf_File_Info &serverConfig = server.getConfig(listeningPort);

		// Salvar o diretório raiz original
		std::string originalRootDirectory = serverConfig.RootDirectory;

		std::cout << " ##****** Root directory : " << serverConfig.RootDirectory << std::endl;

		if (processRulesRequest(requestMsg, server) == true)
		{
			if (!requestMsg.is_cgi) // ======ALTERAÇÂO======
			{
				std::string fileUploadDirectoryCopy = serverConfig.fileUploadDirectory;
				int portListenCopy = serverConfig.portListen;
				std::string rootDirectoryCopy = serverConfig.RootDirectory;
				std::cout << RED << "!!!!! config upload: " << fileUploadDirectoryCopy << RESET << std::endl;
				std::cout << RED << "!!!!! config port: " << portListenCopy << RESET << std::endl;
				std::cout << RED << "!!!!! config root: " << rootDirectoryCopy << RESET << std::endl;
				handleRequest(requestMsg, server);
			}
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
		server.handleRedirectRequest(request, server);
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

std::string removeFirstDirectory(const std::string& fullPath)
{
    size_t firstSlash = fullPath.find("/");
    size_t secondSlash = fullPath.find("/", firstSlash + 1);
    
    std::string firstDirectory = fullPath.substr(firstSlash + 1, secondSlash - firstSlash - 1);
    size_t dotInFirstDirectory = firstDirectory.find(".");
    
    if (dotInFirstDirectory != std::string::npos && dotInFirstDirectory < firstDirectory.rfind("/"))
        return fullPath;
    
    if (fullPath.empty() || fullPath[0] != '/')
        return fullPath;
    
    if (secondSlash == std::string::npos)
        return "";
    
    return fullPath.substr(secondSlash);
}
// std::string removeFirstDirectory(const std::string& fullPath)
// {
// 	size_t firstSlash = fullPath.find("/");
// 	size_t secondSlash = fullPath.find("/", firstSlash + 1);
	
// 	if (fullPath.substr(firstSlash + 1, secondSlash - firstSlash - 1).find(".") != std::string::npos)
// 		return fullPath;
	
// 	if (fullPath.empty() || fullPath[0] != '/')
// 		return fullPath;
	
// 	if (secondSlash == std::string::npos)
// 		return "";
	
// 	return fullPath.substr(secondSlash);
// }

std::string removeTrailingSlash(const std::string& path)
{
	if (!path.empty() && path.at(path.size() - 1) == '/')
		return path.substr(0, path.size() - 1);
	return path;
}

bool fileExistsInDirectory(const std::string& directory, const std::string& filename)
{

	std::cout << "[fileExistsInDirectory] DIRECTORY: " << directory << std::endl;
	std::cout << "[fileExistsInDirectory] FILENAME: " << filename << std::endl;
    std::string fullPath = directory + "/" + filename;
	std::cout << "[fileExistsInDirectory] FULLPATH: " << fullPath << std::endl;
    if(access(fullPath.c_str(), F_OK) != -1)
		return true;
	else
		return false;
}

void ServerInfo::handleGetRequest(HTTrequestMSG& requestMsg, ServerInfo& server)
{
	std::vector<int> ports = server.getPortList();
	int listeningPort = ports[0];
	conf_File_Info &serverConfig = server.getConfig(listeningPort);
	
	// if(serverConfig.directoryListingEnabled == true)
	// 	return;

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
	std::cout << "[handleGetRequest] rootDirectory: " << rootDirectory << std::endl;
	//std::cout << "[handleGetRequest] requestMSG PATH original: " << requestMsg.path << std::endl;
	//std::cout << "[handleGetRequest] requestMSG PATH1: " << requestMsgFile << std::endl;
	//std::cout << "[handleGetRequest] requestMSG PATH2: " << requestMsgFile2 << std::endl;
	//std::cout << "[handleGetRequest] Full path: " << fullPath << std::endl;
	
	// char cwd[1024];
	// getcwd(cwd, sizeof(cwd));
	// std::string full_path2 = std::string(cwd) + "/" + fullPath;
	// std::cout << "[handleGetRequest] @@ Full path: " << full_path2 << std::endl;

	std::string fullPath = getCompletePath2();
	//ifFileRmoveFile(fullPath);
	if (fullPath[fullPath.length() - 1] == '/') {
		fullPath.erase(fullPath.length() - 1);
	}
	std::cout << "[handleGetRequest] @@ Full path: " << fullPath << std::endl;
	if (!fileExists(fullPath))
	{
		std::cerr << "[DEBUG] File does not exist: " << fullPath << std::endl;
		server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
		printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
		return;
	}

	std::cout << "serverConfig.defaultFile: " << serverConfig.defaultFile << std::endl;
	std::cout << "serverConfig.directoryListingEnabled: " << serverConfig.directoryListingEnabled << std::endl;
	std::cout << "fileExistsInDirectory: " << fileExistsInDirectory(fullPath, serverConfig.defaultFile) << std::endl;
	if (isDirectory(fullPath) && serverConfig.directoryListingEnabled == true && fileExistsInDirectory(fullPath, serverConfig.defaultFile) == false)
	{
		handleDirectoryListing(serverConfig, requestMsg, server);
		std::cout << "ENTREI NO TESTE" << std::endl;
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
				// if(!(handleDirectoryListing(serverConfig, requestMsg, server)))

				std::cout << "ENTREI\n\\n";

				// if((handleDirectoryListing(serverConfig, requestMsg, server)) == true)
				// {
				// 	// std::cerr << "Root directory does not exist: " << serverConfig.RootDirectory << std::endl;
				// 	// server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
				// 	// requestMsg.path = serverConfig.RootDirectory + " is not found";
				// 	// requestMsg.version = "";
				// 	server.getResponse();
				// 	//std::cout << "FFFFFFF: " << response << std::endl; 
				// 	printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
				// 	return;
				// }

				
				// if(serverConfig.directoryListingEnabled == true)
				// {
				// 	//std::string fullPath = "resources/website" + requestMsg.path;
				// 	std::string rootDirectory = serverConfig.RootDirectory;
				// 	if (!rootDirectory.empty() && rootDirectory[0] == '/')
				// 	{
				// 		rootDirectory = rootDirectory.substr(1);
				// 	}
					
				// 	//std::string requestMsgFile = removeTrailingSlash(requestMsg.path);
				// 	//std::string requestMsgFile2 = removeFirstDirectory(requestMsgFile);
					
				// 	std::string fullPath = rootDirectory + requestMsg.path;
				// 	std::cout << "[AUTOINDEX] rootDirectory PATH: " << rootDirectory << std::endl;
				// 	std::cout << "[AUTOINDEX] requestMSG PATH original: " << requestMsg.path << std::endl;
				// 	// std::cout << "[AUTOINDEX] requestMSG PATH1: " << requestMsgFile << std::endl;
				// 	// std::cout << "[AUTOINDEX] requestMSG PATH2: " << requestMsgFile2 << std::endl;
				// 	std::cout << "[AUTOINDEX] Full path: " << fullPath << std::endl;

				// 	std::string full_path1 = "/" + fullPath;;
				// 	std::string full_path2;
				// 	if (full_path1.substr(0, 5) == "/home")
				// 	{
				// 		full_path2 = full_path1;
				// 	}
				// 	else
				// 	{
				// 		char cwd[1024];
				// 		getcwd(cwd, sizeof(cwd));
				// 		full_path2 = std::string(cwd) + full_path1;
				// 	}

				// 	// char cwd[1024];
				// 	// getcwd(cwd, sizeof(cwd));
				// 	// std::string full_path2 = std::string(cwd) + "/" + fullPath;
					
				// 	//struct stat path_stat;
				// 	//stat(full_path2.c_str(), &path_stat);
				// 	struct stat path_stat;
				// 	if(stat(full_path2.c_str(), &path_stat) != 0) {
				// 		std::cerr << "Error accessing " << full_path2 << ": " << strerror(errno) << std::endl;
				// 		server.setResponse("HTTP/1.1 500 Internal Server Error\r\n\r\n");
				// 		return;
				// 	}
				// 	bool is_directory = S_ISDIR(path_stat.st_mode);
					
				// 	std::string response;
				// 	if (is_directory)
				// 	{
				// 		DIR *dir;
				// 		struct dirent *ent;
				// 		if ((dir = opendir (full_path2.c_str())) != NULL)
				// 		{
				// 			response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
				// 			response += "<html><head><style>body { background: #ADD8E6; }</style></head><body>";
				// 			while ((ent = readdir (dir)) != NULL)
				// 			{
				// 				response += ent->d_name;
				// 				response += "<br>";
				// 				//std::cout << ent->d_name << std::endl;
				// 			}
				// 			response += "<button onclick=\"location.href='index.html'\" type=\"button\">Go Home</button>";
				// 			response += "</body></html>\n";
				// 			closedir (dir);
				// 		}
				// 		else
				// 		{
				// 			perror ("");
				// 			response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
				// 		}
				// 	}
				// 	else
				// 	{
				// 		//The path is a file, not a directory.
				// 		//Try to open the file and send it as the response.
				// 		std::ifstream file(full_path2.c_str());
				// 		if (file.is_open())
				// 		{
				// 			response = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
				// 			server.setResponse(response);
				// 		}
				// 		else
				// 		{
				// 			server.setResponse("HTTP/1.1 404 Not Found AUTOINDEX\r\n\r\n");
				// 		}
				// 	}
				// 	std::cout << "AUTOINDEX response: " << response << std::endl;	 
				// 	server.setResponse(response);
				// 	printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
            	// 	return;
				//}
				//else
				{					
					std::cerr << "[DEBUG] Index file not found or is not a regular file: " << indexPath << std::endl;
					server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
					printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
					return;
				}	
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
		server.getCompletePath2();
		std::cout << "Complete path: " << server.getCompletePath2() << std::endl;
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
				//std::cout << "Request no RUNSERVER: " << request << std::endl;
				it->clientSocket = newsockfd;
				processRequest(request, *it);
				
				// verificar map de configuração //	criar nova funcao
				//processRulesRequest(request, *it);

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
