/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fde-carv <fde-carv@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/26 15:10:07 by fde-carv          #+#    #+#             */
/*   Updated: 2024/05/22 11:10:18 by fde-carv         ###   ########.fr       */
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

// const sockaddr_in& ServerInfo::getAddress() const
// {
// 	if (cli_addrs.empty()) {
// 		throw std::runtime_error("No addresses available");
// 	}
// 	return cli_addrs[0];
// }

void	ServerInfo::setResponse(const std::string& response)
{
	this->response = response;
}

std::string	ServerInfo::getResponse() const
{
	return (response);
}

// void ServerInfo::decodeAndStoreUrl(const std::string& url)
// {
// 	rootUrl = decodeUrl(url);
// }

std::string ServerInfo::getRootUrl() const
{
	return rootUrl;
}

// std::vector<int>& ServerInfo::getSockets()
// {
// 	return clientSockets;
// }

// std::vector<int>& ServerInfo::getClientSockets()
// {
// 	return clientSockets;
// }

void ServerInfo::addSocketToList(int sockfd)
{
	clientSockets.push_back(sockfd);
}

// ================================================================================================= //
// ======================================= HELPER FUNCTIONS ======================================== //
// ================================================================================================= //

// Function to get the content type based on the file extension
void printLog(const std::string& method, const std::string& path, const std::string& version, const std::string& httpResponse, ServerInfo& server)
{
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

	std::cout << BG_CYAN_BLACK << timestamp << RESET << " \"" << methodColor << method << " " << path << " ";
	std::cout << version << RESET << "\" " << statusColor << statusCode << RESET << " " << server.getResponse().length() << std::endl;
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

	// if (config.portListen <= 0 || config.portListen > 65535) // acho que a Filipa ja trata -> Perguntar?
	// {
	// 	std::cerr << "Invalid port number: " << config.portListen << std::endl;
	// 	exit(EXIT_FAILURE);
	// }

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
	std::cout << CYAN << "Server with port: " << RESET << config.portListen << CYAN << " is ready!" << RESET;
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

        if (bytesRead == 0)
        {
            //std::cout << "Received empty request, ignoring." << std::endl;
            break;
        }

        request += buffer;

        //std::cout << "Bytes read: " << bytesRead << ", Buffer content: " << buffer << std::endl; // Debug statement

        if (bytesRead < 1023)
            break;
    }

    //std::cout << "Final request: " << request << std::endl; // Debug statement

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

	//std::cout << "Processing request: " << request << std::endl; // Debug statement
	
	std::string method;
	std::string path;
	std::string body;

	std::string requestCopy = request;
	// size_t firstSpace = request.find(" ");
	// size_t secondSpace = request.find(" ", firstSpace + 1);
	// if (firstSpace != std::string::npos && secondSpace != std::string::npos)
	// {
	// 	method = request.substr(0, firstSpace);
	// 	path = request.substr(firstSpace + 1, secondSpace - firstSpace - 1);
	// }

	HTTrequestMSG requestMsg;
	HTTPParser parser;
	if (parser.parseRequest(requestCopy, requestMsg, 10000))
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
			handleRequest(requestMsg, requestMsg.path, server);
		else
			std::cout << MAGENTA << "BRUNO implementa" << RESET << std::endl;
    }
	else
	{
        std::cout << RED << "Erro parser da requisição GET --> get.cpp.\n" << RESET;
		std::cout << "Error message: " << requestMsg.error << std::endl;
    }
}

		// if (method == "GET")
		// 	requestMsg.method = HTTrequestMSG::GET;
		// else if (method == "POST") {
		// 	requestMsg.method = HTTrequestMSG::POST;
		// 	size_t bodyPos = request.find("\r\n\r\n");
		// 	if (bodyPos != std::string::npos) {
		// 		body = request.substr(bodyPos + 4);
		// 		requestMsg.body = body;
		// 	}
		// }
		// else if (method == "DELETE")
		// 	requestMsg.method = HTTrequestMSG::DELETE;
		// else
		// 	requestMsg.method = HTTrequestMSG::UNKNOWN;


bool fileExists(const std::string& filePath)
{
    struct stat buffer;
    return (stat(filePath.c_str(), &buffer) == 0);
}

// Function to handle the request from the HTTP method
void handleRequest(HTTrequestMSG& request, const std::string& path, ServerInfo& server)
{
	// if (!isValidHeader(request)) {
	//     // Enviar uma resposta de erro 400 Bad Request
	//     std::string response = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
	//     server.setResponse(response);
	//     return;
	// }

	if (path == "/favicon.ico")
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

        std::string filePath = "resources/website" + path; // adjust this to your actual file path

        // if (!fileExists(filePath))
        // {
        //     // The file does not exist, return an error or handle this case as needed
        //     std::cout << "File does not exist: FRED: " << filePath << std::endl;
        //     server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
        //     return;
        // }
		//std::cout << RED << "\nResposta: " << server.getResponse() << RESET; // *DEBUG*
		//std::cout << RED << "Tamanho: " << server.getResponse().length() << RESET << std::endl; // *DEBUG*
		if (request.method == HTTrequestMSG::GET)
		{
			server.handleGetRequest(path, server);
		}
		else if (request.method == HTTrequestMSG::POST)
		{
			server.handlePostRequest(path, request, server);
		}
		else if (request.method == HTTrequestMSG::DELETE)
		{
			// Processar solicitação DELETE, se necessário
		}
		else if (request.method == HTTrequestMSG::UNKNOWN)
		{
			server.handleUnknownRequest();
		}
	}
}

// Handles unknown requests
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



void ServerInfo::handleGetRequest(const std::string& path, ServerInfo &server)
{
    std::string fullPath = "resources/website" + path;

    if (!fileExists(fullPath))
    {
        //std::cout << "[DEBUG] File does not exist: " << fullPath << std::endl;
        server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
		printLog("GET", fullPath, "HTTP/1.1", server.getResponse(), server);
        return;
    }
    
    // std::cout << "[DEBUG] Request path: " << path << std::endl;
    // std::cout << "[DEBUG] Full path: " << fullPath << std::endl;

    // std::cout << "[DEBUG] Trying to read file: " << fullPath << std::endl;

    // std::cout << "[DEBUG] Redirecting to directory: " << path << "/" << std::endl;

    struct stat buffer;
    if (stat(fullPath.c_str(), &buffer) == 0)
    {
        if (S_ISREG(buffer.st_mode)) // Se for um arquivo regular
        {
            //std::cout << "[DEBUG] File is a regular file." << std::endl;
            std::string fileContent = readFileContent(fullPath);
            std::string contentType = getContentType(fullPath);
            server.setResponse("HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\n\r\n" + fileContent);
        }
        else if (S_ISDIR(buffer.st_mode)) // Se for um diretório
        {
           // std::cout << "[DEBUG] File is a directory." << std::endl;
            // Redirecionar para o diretório adicionando uma barra no final, se necessário
            if (!path.empty() && path[path.length() - 1] != '/') {
                server.setResponse("HTTP/1.1 301 Moved Permanently\r\nLocation: " + path + "/\r\n\r\n");
				printLog("GET", fullPath, "HTTP/1.1", server.getResponse(), server);
                return;
            }

            // Tentar encontrar e servir index.html dentro do diretório
            std::string indexPath = fullPath;
            if (indexPath[indexPath.length() - 1] != '/') {
                indexPath += '/';
            }
            indexPath += "index.html";

            if (stat(indexPath.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode)) // Verificar se há index.html no diretório
            {
               // std::cout << "[DEBUG] Found index.html in directory." << std::endl;
                std::string fileContent = readFileContent(indexPath);
                std::string contentType = getContentType(indexPath);
                server.setResponse("HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\n\r\n" + fileContent);
				printLog("GET", fullPath, "HTTP/1.1", server.getResponse(), server);
            }
			else if (errno == ENOENT) // Se o arquivo/diretório não existir
			{
				//std::cout << "[DEBUG] File or directory does not exist: " << fullPath << std::endl;
				server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
			}
            else // Se não houver index.html, retornar erro 403
            {
               // std::cout << "[DEBUG] No index.html found in directory." << std::endl;
                server.setResponse("HTTP/1.1 403 Forbidden\r\nContent-Type: text/plain\r\n\r\nAccess to directories is forbidden.");
				printLog("GET", fullPath, "HTTP/1.1", server.getResponse(), server);
			}
        }
    }
    else // Se o arquivo não existir, retornar erro 404
    {
        //std::cout << "[DEBUG] File does not exist: " << fullPath << std::endl;
        server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
		printLog("GET", fullPath, "HTTP/1.1", server.getResponse(), server);
	}

    printLog("GET", fullPath, "HTTP/1.1", server.getResponse(), server);
}















// void ServerInfo::handleGetRequest(const std::string& path, ServerInfo &server)
// {
// 	std::string fullPath = "resources/website" + path;
// 	//std::cout << "Full path: " << fullPath << std::endl; *DEBUG*

// 	struct stat buffer;
// 	if (stat(fullPath.c_str(), &buffer) == 0)
// 	{
// 		if (S_ISDIR(buffer.st_mode))
// 		{
// 			if (path == "/") // Special case for root directory
// 			{
// 				fullPath += "index.html";
// 				std::string fileContent = readFileContent(fullPath);
// 				std::string contentType = getContentType(fullPath); // Determine the content type based on the file extension
// 				server.setResponse("HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\n\r\n" + fileContent);
// 			}
// 			else // Handle other directories
// 			{
// 				std::string directoryContent = readDirectoryContent(fullPath);
// 				if (!directoryContent.empty())
// 					server.setResponse("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + directoryContent);
// 				else
// 					server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nDirectory not found\nERROR 404\n");
// 			}
// 		}
// 		else if (S_ISREG(buffer.st_mode)) // Handle file
// 		{
// 			std::string fileContent = readFileContent(fullPath);
// 			std::string contentType = getContentType(fullPath); // Determine the content type based on the file extension
// 			server.setResponse("HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\n\r\n" + fileContent);
// 		}
// 	}
// 	else // Handle non-existent file or directory
// 		server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");

// 	//int statusCode = 200; // *DEBUG*
// 	// std::string response = it->getResponse(); // *DEBUG*
// 	// size_t response_length = response.length(); // *DEBUG*
// 	// std::cout << "Sending response to client (Length: " << response_length << "): " << response << std::endl; // *DEBUG*
// 	// write(newsockfd, response.c_str(), response_length); //	// *DEBUG*; // Código de status OK
// 	//int contentLength = server.getResponse().length(); // *DEBUG*
// 	printLog("GET", fullPath, "HTTP/1.1", server.getResponse(), server);
// }


// void ServerInfo::handlePostRequest(const std::string& path, HTTrequestMSG& request, ServerInfo &server)
// {
// 	// Parse the request body
// 	std::string body = request.body;

// 	// *DEBUG*
// 	// std::cout << "Method: " << request.method << std::endl;
// 	// std::cout << "State: " << request.state << std::endl;
// 	// std::cout << "Path: " << request.path << std::endl;
// 	// std::cout << "Version: " << request.version << std::endl;
// 	// std::cout << "Query: " << request.query << std::endl;
// 	// std::cout << "Headers:" << std::endl;
// 	// std::map<std::string, std::string>::const_iterator it;
// 	// for (it = request.headers.begin(); it != request.headers.end(); ++it)
// 	// 	std::cout << it->first << ": " << it->second << std::endl;
// 	// std::cout << "Body: " << request.body << std::endl;
// 	// std::cout << "Content-Length: " << request.content_length << std::endl;
// 	// std::cout << "Process Bytes: " << request.process_bytes << std::endl;
// 	// std::cout << "Error: " << request.error << std::endl;
// 	// std::cout << "Boundary: " << request.boundary << std::endl;
// 	// std::cout << "Is CGI: " << (request.is_cgi ? "true" : "false") << std::endl;
// 	// std::cout << "CGI Environment:" << std::endl;
// 	// std::map<std::string, std::string>::const_iterator cgi_it;
// 	// for (cgi_it = request.cgi_env.begin(); cgi_it != request.cgi_env.end(); ++cgi_it)
// 	// 	std::cout << cgi_it->first << ": " << cgi_it->second << std::endl;
// 	// std::cout << "Temp File Path: " << request.temp_file_path << std::endl;
	
// 	// Process the data (this will depend on your application)
// 	// For example, let's say you're expecting form data in the format of key=value&key2=value2
// 	std::string response;
// 	size_t pos = 0;
// 	while ((pos = body.find("&")) != std::string::npos) {
// 		std::string token = body.substr(0, pos);
// 		response += token + "\n";
// 		body.erase(0, pos + 1);
// 	}
// 	response += body;

// 	// Send a response
// 	std::string httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
// 	httpResponse += "Received POST data:\n" + response;

// 	this->setResponse(httpResponse);

// 	std::string fullPath = "resources/website" + path;

// 	//int statusCode = 200; // Código de status OK
// 	//int contentLength = server.getResponse().length();

// 	printLog("POST", fullPath, "HTTP/1.1", server.getResponse(), server);
// }


void ServerInfo::handlePostRequest(const std::string& path, HTTrequestMSG& request, ServerInfo &server)
{
	std::string body = request.body; // Parse the request body

	if (body.empty()) // Check if the body is empty
	{
		this->setResponse("Your expected response here");
		return;
	}

	if (body.find("=") == std::string::npos || body.find("&") == std::string::npos) // Check if the body is in the correct format
	{
		this->setResponse("Your expected response here");
		return;
	}

	std::string fullPath = "resources/website" + path; // Check if the path exists
	struct stat buffer;
	int statResult = stat(fullPath.c_str(), &buffer);

	if (statResult != 0)
	{
		std::string httpResponse = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n";
		httpResponse += "The requested resource could not be found.";
		this->setResponse(httpResponse);
		return;
	}

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

	std::string httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n"; // Send a response
	httpResponse += "Received POST data:\n" + response;

	this->setResponse(httpResponse);

	printLog("POST", fullPath, "HTTP/1.1", server.getResponse(), server);
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
	
	std::cout << "\n\n<" << GREEN << "=+=+=+=+=+=+=+=+=+=" << RESET << " Waiting for client " \
	<< GREEN << "=+=+=+=+=+=+=+=+=+=" << RESET << ">\n" << std::endl;

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
				//std::string response = it->getResponse(); // *DEBUG*
				//std::cout << MAGENTA << "Sending response to client: " << response << RESET << std::endl; // *DEBUG*
				//write(newsockfd, response.c_str(), response.length()); // *DEBUG*

				// std::string response = it->getResponse(); // *DEBUG*
				// size_t response_length = response.length(); // *DEBUG*
				// std::cout << "Sending response to client (Length: " << response_length << "): " << response << std::endl; // *DEBUG*
				// write(newsockfd, response.c_str(), response_length); //	// *DEBUG*

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
