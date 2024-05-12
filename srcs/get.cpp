/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fde-carv <fde-carv@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/26 15:10:07 by fde-carv          #+#    #+#             */
/*   Updated: 2024/05/12 21:38:06 by fde-carv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/get.hpp"

ServerInfo::ServerInfo()
{
	//std::cout << "\nCriando o socket..." << std::endl; // *DEBUG*
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		handleError("Error opening socket.");
		exit(-1);
	}
	this->rootUrl = "resources";
	//std::cout << "Socket criado com sucesso." << std::endl; // *DEBUG*
}
ServerInfo::~ServerInfo()
{
	//std::cout << "Before closing socket, file descriptor: " << sockfd << std::endl; // *DEBUG*
	//close(sockfd);
	//std::cout << "After closing socket, file descriptor: " << sockfd << std::endl; // *DEBUG*
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

const sockaddr_in& ServerInfo::getAddress() const
{
	if (cli_addrs.empty()) {
		throw std::runtime_error("No addresses available");
	}
	return cli_addrs[0];
}

void	ServerInfo::setResponse(const std::string& response)
{
	this->response = response;
}

std::string	ServerInfo::getResponse() const
{
	return (response);
}

void ServerInfo::decodeAndStoreUrl(const std::string& url)
{
	rootUrl = decodeUrl(url);
}

std::string ServerInfo::getRootUrl() const
{
	return rootUrl;
}

std::vector<int>& ServerInfo::getSockets()
{
	return clientSockets;
}

std::vector<int>& ServerInfo::getClientSockets() {
	return clientSockets;
}

void ServerInfo::addSocketToList(int sockfd)
{
	clientSockets.push_back(sockfd);
}

void ServerInfo::addClient(int clientSocket, sockaddr_in clientAddress)
{
	clientSockets.push_back(clientSocket);
	cli_addrs.push_back(clientAddress);
}

void ServerInfo::removeSocketFromList(int sockfd)
{
	clientSockets.erase(std::remove(clientSockets.begin(), clientSockets.end(), sockfd), clientSockets.end());
}


// Function to decode the URL
std::string ServerInfo::decodeUrl(const std::string& url)
{
	std::ostringstream outSS; // output string stream for decoding the url
	for (std::string::const_iterator i = url.begin(); i != url.end(); ++i) // iterate over the url
	{
		if (*i == '%') // if the character is a '%' it means it's a special character
		{
			std::istringstream inSS("0x" + std::string(1, *(i + 1)) + std::string(1, *(i + 2))); // create a string stream with the hex value of the special character
			int temp;
			inSS >> std::hex >> temp; // convert the hex value to an integer
			outSS << static_cast<char>(temp); // convert the integer to a character and add it to the output string stream
			i += 2; // skip the next two characters
		}
		else if (*i == '+')// if the character is a '+' it means it's a space
			outSS << ' '; // add a space to the output string stream
		else
			outSS << *i; // add the character to the output string stream
	}
	return (outSS.str()); // return the decoded url
}

// =================================================================== //
// ======================== HELPER FUNCTIONS ========================= //
// =================================================================== //

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

// bool is_directory(const std::string& path) {
//     struct stat buf;
//     if (stat(path.c_str(), &buf) != 0) {
//         return false;
//     }
//     return S_ISDIR(buf.st_mode);
// }

void setupDirectory(ServerInfo& server, conf_File_Info& config)
{
	//std::cout << "\nRoot directory: " << config.RootDirectory << std::endl; // *DEBUG*
	//std::cout << "Root URL: " << server.getRootUrl() << std::endl; // *DEBUG*

	chmod("/resources/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	std::string rootDir = config.RootDirectory;
	std::string rootUrl = server.getRootUrl();
	if(rootDir.substr(0, rootUrl.length()) != rootUrl)
	{
		handleError("Error: Root URL should star with \'resources\' directory.");
		exit(-1);
	}

	std::string subDir = rootDir.substr(rootUrl.length());

	if(!subDir.empty() && subDir[0] == '/')
		subDir = subDir.substr(1);

	std::string path = "";
	std::stringstream ss(rootDir);
	std::string token;

	while(std::getline(ss, token, '/')) {
		path += token + "/";
		if(!is_directory(path)) {
			if(mkdir(path.c_str(), 0777) == -1) {
				perror("Error creating directory");
				exit(EXIT_FAILURE);
			}

			if(chmod(path.c_str(), 0777) == -1) {
				perror("Error changing directory permissions");
				exit(EXIT_FAILURE);
			}
		}
	}
}

int remove_file(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
	(void)sb;
	(void)ftwbuf;
	std::string filename(fpath);

	// std::cout << RED << "Processing file: " << filename << RESET << std::endl; // *DEBUG*
	if(filename.find("/website") == std::string::npos && filename.find("/upload") == std::string::npos) {
		// std::cout << RED << "Removing file or directory: " << filename << RESET << std::endl; // *DEBUG*
		if(typeflag == FTW_D) {
			return rmdir(fpath);
		} else {
			return remove(fpath);
		}
	}
	return 0;
}

// int print_file(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) // *DEBUG*
// {
// 	(void)sb;
// 	(void)ftwbuf;
// 	(void)typeflag;
// 	std::cout << RED << "Found file or directory: " << fpath << RESET << std::endl;
// 	return 0;
// }

int remove_directory(const char *path)
{
	//std::cout << RED << "Path provided to remove_directory: " << path << RESET << std::endl; // *DEBUG*
	//std::cout << RED << "Calling nftw for directory: " << path << RESET << std::endl; // *DEBUG*
	int result = nftw(path, remove_file, 64, FTW_DEPTH | FTW_PHYS);
	return result;
}


void setupServer(ServerInfo& server, conf_File_Info& config)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("Error on socket creation");
		exit(EXIT_FAILURE);
	}
	//std::cout << "Socket created, file descriptor: " << sockfd << std::endl; // *DEBUG*

	server.addSocketToList(sockfd);

	server.setSocketFD(sockfd);
	//std::cout << "Socket file descriptor stored in server object: " << server.getSocketFD() << std::endl; // *DEBUG*

	sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(config.portListen);

	//std::cout << "Setting up server on port: " << config.portListen << std::endl; // *DEBUG*

	if (config.portListen <= 0 || config.portListen > 65535) // acho que a Filipa ja trata
	{
		std::cerr << "Invalid port number: " << config.portListen << std::endl;
		exit(EXIT_FAILURE);
	}

	server.setAddress(serv_addr);

	setupDirectory(server,config);

	int opt = 1;
	if (setsockopt(server.getSocketFD(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		perror("setsockopt");
		handleError("Error on setsockopt.");
		exit(EXIT_FAILURE);
	}

	server.setSocketFD(sockfd);
	//std::cout << "Socket file descriptor stored in server object: " << server.getSocketFD() << std::endl; // *DEBUG*

	if (bind(server.getSocketFD(), (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("Error on binding");
		handleError("Error on binding.");
		exit(EXIT_FAILURE);
	}

	//std::cout << "Socket file descriptor before listen: " << server.getSocketFD() << std::endl; // *DEBUG*
	if (listen(server.getSocketFD(), 128) < 0)
	{
		perror("Error on listen");
		exit(EXIT_FAILURE);
	}
	
	std::cout << std::endl;
	std::cout << CYAN << "Server with port: " << RESET << config.portListen << CYAN << " is ready!" << RESET;// << std::endl;
}

// Read the request from the client and return it as a string
std::string readRequest(int sockfd)
{
	char buffer[1024];
	std::string request;

	while (1)
	{
		memset(buffer, 0, 1024);
		ssize_t bytesRead = read(sockfd, buffer, 1023);

		if (bytesRead < 0)
		{
			handleError("Error reading from socket.");
			exit(-1);
		}

		request += buffer;

		if (bytesRead < 1023)
			break;
	}

	return request;
}

// Process the request and send the response and gives the server info
void processRequest(const std::string& request, ServerInfo& server)
{
	std::string method;
	std::string path;

	size_t firstSpace = request.find(" ");
	size_t secondSpace = request.find(" ", firstSpace + 1);
	if (firstSpace != std::string::npos && secondSpace != std::string::npos)
	{
		method = request.substr(0, firstSpace);
		path = request.substr(firstSpace + 1, secondSpace - firstSpace - 1);
	}

	HTTrequestMSG requestMsg;
	if (method == "GET")
		requestMsg.method = HTTrequestMSG::GET;
	else if (method == "POST")
		requestMsg.method = HTTrequestMSG::POST;
	else if (method == "DELETE")
		requestMsg.method = HTTrequestMSG::DELETE;
	else
		requestMsg.method = HTTrequestMSG::UNKNOWN;

	handleRequest(requestMsg, path, server);
}

// Function to handle the request from the HTTP method
void handleRequest(HTTrequestMSG& request, const std::string& path, ServerInfo& server)
{
	if (request.method == HTTrequestMSG::GET)
	{
		server.handleGetRequest(path, server);
	}
	else if (request.method == HTTrequestMSG::POST)
	{
		server.handlePostRequest(path, request);
	}
	else if (request.method == HTTrequestMSG::DELETE)
	{
		// The request is a DELETE
	}
	else if (request.method == HTTrequestMSG::UNKNOWN)
	{
		server.handleUnknownRequest();
	}
}

void ServerInfo::handleUnknownRequest()
{
		std::string response = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
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

void* handleConnection(void* arg)
{
	int newsockfd = *(int*)arg;
	ServerInfo& server = *(ServerInfo*)arg;
	std::string request = readRequest(newsockfd);
	processRequest(request, server);
	write(newsockfd, server.getResponse().c_str(), server.getResponse().length());
	close(newsockfd);
	return NULL;
}

void ServerInfo::handleGetRequest(const std::string& path, ServerInfo &server)
{
	std::string fullPath = "resources/website" + path;
	//std::cout << "Full path: " << fullPath << std::endl; // Print the full path

	struct stat buffer;
	if (stat(fullPath.c_str(), &buffer) == 0)
	{
		if (S_ISDIR(buffer.st_mode))
		{
			if (path == "/") // Special case for root directory // comment to pass google test
			{
				fullPath += "index.html";
				std::string fileContent = readFileContent(fullPath);
				std::string contentType = getContentType(fullPath); // Determine the content type based on the file extension
				server.setResponse("HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\n\r\n" + fileContent);
			}
			else // Handle other directories
			{
				std::string directoryContent = readDirectoryContent(fullPath);
				if (!directoryContent.empty())
					server.setResponse("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + directoryContent);
				else
					server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nDirectory not found\nERROR 404\n");
			}
		}
		else if (S_ISREG(buffer.st_mode)) // Handle file
		{
			std::string fileContent = readFileContent(fullPath);
			std::string contentType = getContentType(fullPath); // Determine the content type based on the file extension
			server.setResponse("HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\n\r\n" + fileContent);
		}
	}
	else // Handle non-existent file or directory
		server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
}

void ServerInfo::handlePostRequest(const std::string& /*path*/, HTTrequestMSG& request)
{
    // Parse the request body
    std::string body = request.body; // This depends on how your HTTrequestMSG is structured

    // Process the data (this will depend on your application)
    // For example, let's say you're expecting two parts: text and file
    std::string text, file;
    parseMultipartFormData(body, text, file);

    // Do something with the parts
    // ...

    // Send a response
    std::string response = "HTTP/1.1 200 OK\r\n\r\n";
    response += "Received text: " + text + ", file: " + file;

    // Send the response
    // This will depend on how your server is set up
    // ...
}

void ServerInfo::parseMultipartFormData(const std::string& body, std::string& text, std::string& file)
{
    std::string boundary = "-----------------------------1234567890";
    size_t pos = body.find(boundary);
    while (pos != std::string::npos) {
        size_t endPos = body.find(boundary, pos + boundary.length());
        if (endPos != std::string::npos) {
            std::string part = body.substr(pos + boundary.length(), endPos - pos - boundary.length());

            size_t namePos = part.find("name=\"");
            if (namePos != std::string::npos) {
                size_t nameEndPos = part.find("\"", namePos + 6);
                if (nameEndPos != std::string::npos) {
                    std::string name = part.substr(namePos + 6, nameEndPos - namePos - 6);

                    size_t valuePos = part.find("\r\n\r\n", nameEndPos);
                    if (valuePos != std::string::npos) {
                        std::string value = part.substr(valuePos + 4);

                        if (name == "text") {
                            text = value;
                        } else if (name == "file") {
                            file = value;
                        }
                    }
                }
            }
        }

        pos = body.find(boundary, endPos + boundary.length());
    }
}

void runServer(std::vector<ServerInfo>& servers)
{
	fd_set read_fds;
	FD_ZERO(&read_fds);

	int max_fd = -1;
	for (std::vector<ServerInfo>::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		int sockfd = it->getSocketFD();
		FD_SET(sockfd, &read_fds);
		if (sockfd > max_fd)
			max_fd = sockfd;
	}
	
	std::cout << GREEN << "\n\n<========== Waiting for client ==========>\n" << RESET << std::endl;

	while (1)
	{
		fd_set temp_fds = read_fds;
		if (select(max_fd + 1, &temp_fds, NULL, NULL, NULL) < 0)
		{
			perror("Error on select");
			exit(EXIT_FAILURE);
		}

		for (std::vector<ServerInfo>::iterator it = servers.begin(); it != servers.end(); ++it)
		{
			int sockfd = it->getSocketFD();
			if (FD_ISSET(sockfd, &temp_fds))
			{
				sockaddr_in cli_addr;
				socklen_t clilen = sizeof(cli_addr);
				int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
				if (newsockfd < 0)
				{
					perror("Error on accept");
					exit(EXIT_FAILURE);
				}

				std::string request = readRequest(newsockfd);
				processRequest(request, *it);
				write(newsockfd, it->getResponse().c_str(), it->getResponse().length());
				close(newsockfd);
			}
		}
	}
}






std::string readDirectoryContent(const std::string& directoryPath)
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
		// Could not open directory
		std::cerr << "Could not open directory: " << directoryPath << std::endl;
		std::cout << "Failed to open directory: " << directoryPath << std::endl; // Print the directory path
		return "";
	}

	// Sort the files vector
	std::sort(files.begin(), files.end());

	// Convert the files vector to a string
	std::string directoryContent;
	for (std::vector<std::string>::const_iterator i = files.begin(); i != files.end(); ++i)
	{
		directoryContent += *i;
		directoryContent += "\n";
	}

	std::cout << "Directory content:\n" << directoryContent << std::endl; // Print directory content
	return directoryContent;
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

	//std::cout << "Content type for " << filePath << ": " << contentType << std::endl; // *DEBUG*
	return contentType;
}


