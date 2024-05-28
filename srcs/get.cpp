/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fde-carv <fde-carv@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/26 15:10:07 by fde-carv          #+#    #+#             */
/*   Updated: 2024/05/28 23:19:17 by fde-carv         ###   ########.fr       */
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
	this->rootUrl = "resources";
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

	// Extract status code from HTTP response
	std::string statusCodeStr;
	size_t statusCodePos = httpResponse.find("HTTP/1.1") + 9; // Position after "HTTP/1.1"
	if (statusCodePos != std::string::npos && httpResponse.length() >= statusCodePos + 3)
		statusCodeStr = httpResponse.substr(statusCodePos, 3);
	int statusCode = (statusCodeStr.empty()) ? 0 : atoi(statusCodeStr.c_str());

	std::string statusColor = (statusCode == 200) ? GREEN : RED;
	std::vector<int> ports = server.getPortList();
	if (!ports.empty()) {
		int currentPort = ports.front();
		std::cout << BG_CYAN_BLACK << timestamp << RESET << GREEN << " [" << RESET << requestCount << GREEN << "] " << BLUE << \
		"Connected with client at 127.0.0.1:" << CYAN << currentPort << RESET << std::endl;
	}
	std::cout << BG_CYAN_BLACK << timestamp << RESET << BLUE << " [" << RESET << requestCount << BLUE << "] \"" << methodColor << method << " " << path << " ";
	std::cout << version << RESET << "\" " << statusColor << statusCode << RESET << " " << server.getResponse().length() << std::endl;
	std::cout << BG_CYAN_BLACK << timestamp << RESET << RED << " [" << RESET << requestCount << RED << "] " \
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

//void setupDirectory(ServerInfo& server, conf_File_Info& config)
void setupDirectory(ServerInfo& server, const conf_File_Info& config)
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
	if(filename.find("/website") == std::string::npos && filename.find("/upload") == std::string::npos)
	{
		// std::cout << RED << "Removing file or directory: " << filename << RESET << std::endl; // *DEBUG*
		if(typeflag == FTW_D)
			return rmdir(fpath);
		else
			return remove(fpath);
	}
	return 0;
}

// To use in main() to remove the temp directory
int remove_directory(const char *path)
{
	//std::cout << RED << "Path provided to remove_directory: " << path << RESET << std::endl; // *DEBUG*
	//std::cout << RED << "Calling nftw for directory: " << path << RESET << std::endl; // *DEBUG*
	int result = nftw(path, remove_file, 64, FTW_DEPTH | FTW_PHYS);
	return result;
}

// Setup the server
//void setupServer(ServerInfo& server, conf_File_Info& config)
void setupServer(ServerInfo& server, const conf_File_Info& config)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
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
	setupDirectory(server,config);
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
}



int	verify_body_content(std::string request)
{
	long unsigned int	content_length;
	size_t	pos = request.find("Content-Length: ");

	std::string	tmp;
	int	i = 0;

	while (isdigit(request[pos + 16]))
	{
		tmp[i] = request[pos + 16];
		//tmp + request[pos + 16]; // Changed here
		i++;
		pos++;
	}

	content_length = atoi(tmp.c_str());

	std::cout << "The content length found was " << content_length << std::endl;

	std::string	boundary;

	if ((pos = request.find("boundary=")) != std::string::npos)
	{
		tmp.append(request, (request.find("\r\n\r\n") + 4));

		//std::cout << "After boundary:]" << tmp << "[Finish";
		if (tmp.length() < content_length)
		 return (-1);
	}

	return (0);
}

std::string	read_all_body(int client_socket)
{
	char		line[1024];
	std::string	total_request;

	while (1)
	{
		while (1)
		{
			memset(line, 0, 1024);
			ssize_t bytesRead = read(client_socket, line, 1023);

			if (bytesRead < 0)
			{
				std::cerr << "Error in read\n";
				exit(-1);
			}

			total_request.append(line, bytesRead);

			if (bytesRead < 1023)
				break;
		}
		if (verify_body_content(total_request) < 0)
			continue;
		else
			break;
	}

	//print_the_request(total_request);

	return (total_request);
}	


//Read the request from the client and return it as a string
std::string readRequest(int sockfd)
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
        {
            break;
        }
        else
        {
            buffer[bytesRead] = '\0';
            //std::cout << "Header received: " << buffer << std::endl; // Print the header
        }

        request.append(buffer, bytesRead);

        // If we've reached the end of the header, break
        if (request.find("\r\n\r\n") != std::string::npos)
            break;
    }

    // Send 100 Continue response here
    // if (request.find("Expect: 100-continue") != std::string::npos)
    // {
    //     std::string response = "HTTP/1.1 100 Continue\r\n\r\n";
    //     send(sockfd, response.c_str(), response.size(), 0);
    
    // Read the body
	HTTPParser parser;
	if (parser.headerHasField("Content-Length", request))
	{
		size_t contentLength = parser.getContentLength(request);
		std::cout << "Content-Length: " << contentLength << std::endl; // Print Content-Length

		size_t actualDataSize = request.size();
		std::cout << "Actual Data Size: " << actualDataSize << std::endl; // Print Actual Data Size

		if (contentLength > actualDataSize)
		{
			size_t bytesReadTotal = actualDataSize;
			while (bytesReadTotal < contentLength)
			{
				memset(buffer, 0, 4096);
				ssize_t bytesRead = recv(sockfd, buffer, std::min(static_cast<size_t>(4095), contentLength - bytesReadTotal), 0);
				std::cout << "Bytes read: " << bytesRead << std::endl; // Print the number of bytes read

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
				std::cout << "Total Bytes Read: " << bytesReadTotal << std::endl; // Print Total Bytes Read

				// Check if the total bytes read is greater than the raw size
				if (bytesReadTotal > request.size())
				{
					std::cerr << "Read beyond the end of available data." << std::endl;
					break;
				}
			}
		}
	}
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
	
	//std::string method;
	//std::string path;
	//std::string body;
	std::string ParaCGI = request;
	std::string requestCopy = request;
	HTTrequestMSG requestMsg;
	HTTPParser parser;
	size_t maxSize = 100000; // Aumentar o tamanho máximo para 10MB
	if(maxSize > requestCopy.size())
		maxSize = requestCopy.size();
	//std::cout << "Max size: " << maxSize << std::endl;	
	if (parser.parseRequest(requestCopy, requestMsg, maxSize))
	{
		// 	// *DEBUG*
		// std::cout << GREEN << "certo\n" << RESET;
		// std::cout << "Method: " << requestMsg.method << std::endl;
		// std::cout << "State: " << requestMsg.state << std::endl;
		// std::cout << "Path: " << requestMsg.path << std::endl;
		// std::cout << "Version: " << requestMsg.version << std::endl;
		// std::cout << "Query: " << requestMsg.query << std::endl;
		// std::cout << "Headers:" << std::endl;
		// std::map<std::string, std::string>::const_iterator it;
		// for (it = requestMsg.headers.begin(); it != requestMsg.headers.end(); ++it)
		// 	std::cout << it->first << ": " << it->second << std::endl;
		// std::cout << "Body: " << requestMsg.body << std::endl;
		// std::cout << "Content-Length: " << requestMsg.content_length << std::endl;
		// std::cout << "Process Bytes: " << requestMsg.process_bytes << std::endl;
		// std::cout << "Error: " << requestMsg.error << std::endl;
		// std::cout << "Boundary: " << requestMsg.boundary << std::endl;
		// std::cout << "Is CGI: " << (requestMsg.is_cgi ? "true" : "false") << std::endl;
		// std::cout << "CGI Environment:" << std::endl;
		// std::map<std::string, std::string>::const_iterator cgi_it;
		// for (cgi_it = requestMsg.cgi_env.begin(); cgi_it != requestMsg.cgi_env.end(); ++cgi_it)
		// 	std::cout << cgi_it->first << ": " << cgi_it->second << std::endl;
		// std::cout << "Temp File Path: " << requestMsg.temp_file_path << std::endl;
		if (requestMsg.is_cgi == false)
			handleRequest(requestMsg, server);
		else
		{
			CGI cgi;
			//std::cout << server.getSockets << "\n\n";
			cgi.PerformCGI(server.clientSocket , ParaCGI);
			
			//std::cout << MAGENTA << "\t\t\t==> BRUNO Implementa 😁😁😁" << RESET << std::endl;
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
	else
	{
		std::string filePath = "resources/website" + request.path; // adjust this to your actual file path
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
			// Processar solicitação DELETE, se necessário
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

bool isValidFilePath(const std::string& path) {
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0);
}

bool isDirectory(const std::string& path) {
	struct stat buffer;
	if (stat(path.c_str(), &buffer) == 0) {
		return S_ISDIR(buffer.st_mode);
	}
	return false;
}



void ServerInfo::handleGetRequest(HTTrequestMSG& requestMsg, ServerInfo& server)
{
	std::string fullPath = "resources/website" + requestMsg.path;

	if (!fileExists(fullPath))
	{
		//std::cout << "[DEBUG] File does not exist: " << fullPath << std::endl;
		server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
		printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
		return;
	}

	struct stat buffer;
	if (stat(fullPath.c_str(), &buffer) == 0)
	{
		if (S_ISREG(buffer.st_mode)) // Se for um arquivo regular
		{
			std::string fileContent = readFileContent(fullPath);
			std::string contentType = getContentType(fullPath);
			server.setResponse("HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\n\r\n" + fileContent);
		}
		else if (S_ISDIR(buffer.st_mode)) // Se for um diretório
		{
			if (!requestMsg.path.empty() && requestMsg.path[requestMsg.path.length() - 1] != '/')
			{
				server.setResponse("HTTP/1.1 301 Moved Permanently\r\nLocation: " + requestMsg.path + "/\r\n\r\n");
				printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
				return;
			}

			// Tentar encontrar e servir index.html dentro do diretório
			std::string indexPath = fullPath;
			if (indexPath[indexPath.length() - 1] != '/')
				indexPath += '/';
			indexPath += "index.html";

			if (stat(indexPath.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode)) // Verificar se há index.html no diretório
			{
				std::string fileContent = readFileContent(indexPath);
				std::string contentType = getContentType(indexPath);
				server.setResponse("HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\n\r\n" + fileContent);
			}
			else if (errno == ENOENT) // Se o arquivo/diretório não existir
			{
				server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
			}
			else // Se não houver index.html, retornar erro 403
			{
				server.setResponse("HTTP/1.1 403 Forbidden\r\nContent-Type: text/plain\r\n\r\nAccess to directories is forbidden.");
				printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
			}
		}
	}
	else // Se o arquivo não existir, retornar erro 404
	{
		server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
		printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
	}
	printLog(methodToString(requestMsg.method), requestMsg.path, requestMsg.version, server.getResponse(), server);
}

void ServerInfo::handlePostRequest(HTTrequestMSG& request, ServerInfo &server)
{
	std::string body = request.body; // Parse the request body

	if (body.empty()) // Check if the body is empty
	{
		//std::cerr << "Error: Body is empty" << std::endl;
		this->setResponse("HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nError: Request body is empty");
		return;
	}

	// if (body.find("=") == std::string::npos || body.find("&") == std::string::npos) // Check if the body is in the correct format -> WALTER
	// {
	// 	//std::cerr << "Error: Body is not in the correct format.\n" << std::endl;
	// 	this->setResponse("HTTP/1.1 422 Unprocessable Entity\r\nContent-Type: text/plain\r\n\r\nError: Request body is not in the correct format");
	// 	return;
	// }

	std::string response; // Process the data
	std::string delimiter = "&";
	size_t pos = 0;
	std::string token;
	while ((pos = body.find(delimiter)) != std::string::npos) {
		token = body.substr(0, pos);
		response += token + "\n";
		body.erase(0, pos + delimiter.length());
	}
	response += body;

	if (response.empty()) // Check if the response is empty
	{
		std::cerr << "Error: Response is empty" << std::endl;
	}

	// std::string httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n"; // Send a response
	// httpResponse += "Received POST data:\n" + response;

	std::string httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"; // Send a response
	httpResponse += "<html><head><style>body { background: #ADD8E6;; }</style></head><body>";
	httpResponse += "<p>Received POST data:</p><pre>" + response + "</pre>";
	httpResponse += "<button onclick=\"location.href='index.html'\" type=\"button\">Go Home</button>";
	httpResponse += "</body></html>\n";

	this->setResponse(httpResponse);

	this->setResponse(httpResponse);

	printLog(methodToString(request.method), request.path, request.version, server.getResponse(), server);
}

void	runServer(std::vector<ServerInfo>& servers)
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
	
	std::cout << "\n<" << GREEN << "=+=+=+=+=+=+=+=+=+=" << RESET << " Waiting for client " \
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
				
				//=================
				it->clientSocket = newsockfd;
				//=================

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
		std::cerr << "Could not open directory: " << directoryPath << std::endl; // Could not open directory
		return "";
	}

	std::sort(files.begin(), files.end()); // Sort the files vector

	std::string directoryContent; // Convert the files vector to a string
	for (std::vector<std::string>::const_iterator i = files.begin(); i != files.end(); ++i)
	{
		directoryContent += *i;
		directoryContent += "\n";
	}

	//std::cout << "Directory content:\n" << directoryContent << std::endl; // Print directory content
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
