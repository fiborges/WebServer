/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fde-carv <fde-carv@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/26 15:10:07 by fde-carv          #+#    #+#             */
/*   Updated: 2024/05/09 16:00:51 by fde-carv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/get.hpp"
#include "../includes/librarie.hpp"
#include "../includes/parser.hpp"

#include <fcntl.h>

// abrir outro terminal e verificar as portas que estam a ouvir
// netstat -tuln | grep 8081

ServerInfo::ServerInfo()
{
    std::cout << "\nCriando o socket..." << std::endl; // *DEBUG*
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        handleError("Error opening socket.");
        exit(-1);
    }
	this->rootUrl = "resources/website";
    std::cout << "Socket criado com sucesso." << std::endl; // *DEBUG*
}
ServerInfo::~ServerInfo()
{
	std::cout << "Before closing socket, file descriptor: " << sockfd << std::endl; // *DEBUG*
	//close(sockfd);
	std::cout << "After closing socket, file descriptor: " << sockfd << std::endl; // *DEBUG*
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
	return serv_addr;
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
	std::cerr << RED1 << errorMessage << RESET << std::endl;
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

void setupServer(ServerInfo& server, conf_File_Info& config)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error on socket creation");
        exit(EXIT_FAILURE);
    }
    std::cout << "Socket created, file descriptor: " << sockfd << std::endl; // *DEBUG*

	server.addSocketToList(sockfd);

    server.setSocketFD(sockfd);
    std::cout << "Socket file descriptor stored in server object: " << server.getSocketFD() << std::endl; // *DEBUG*


	sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(config.portListen);

	std::cout << "Setting up server on port: " << config.portListen << std::endl; // *DEBUG*

	if (config.portListen <= 0 || config.portListen > 65535) {
	std::cerr << "Invalid port number: " << config.portListen << std::endl;
	exit(EXIT_FAILURE);
    }

	std::cout << GREEN << "\nWaiting for client...\n" << RESET << std::endl;

	server.setAddress(serv_addr);

	std::cout << "Root directory: " << config.RootDirectory << std::endl; // *DEBUG*
	std::cout << "Root URL: " << server.getRootUrl() << std::endl; // *DEBUG*

	// if(config.RootDirectory != server.getRootUrl())
	// {
	// 	handleError("Error in conf file root");
	// 	exit(-1);
	// }

	int opt = 1;
	if (setsockopt(server.getSocketFD(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		perror("setsockopt");
		handleError("Error on setsockopt.");
		exit(EXIT_FAILURE);
	}

	server.setSocketFD(sockfd);
	std::cout << "Socket file descriptor stored in server object: " << server.getSocketFD() << std::endl; // *DEBUG*

	if (bind(server.getSocketFD(), (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("Error on binding");
		exit(EXIT_FAILURE);
	}

	std::cout << "Socket file descriptor before listen: " << server.getSocketFD() << std::endl; // *DEBUG*
	if (listen(server.getSocketFD(), 128) < 0)
	{
		perror("Error on listen");
		exit(EXIT_FAILURE);
	}
}


int acceptConnection(std::vector<int>& sockets, ServerInfo& server, sockaddr_in& cli_addr)
{
    fd_set read_fds;
    struct timeval timeout;

    FD_ZERO(&read_fds);

    int max_fd = -1;
    for (std::vector<int>::iterator it = sockets.begin(); it != sockets.end(); ++it) {
        FD_SET(*it, &read_fds);
        if (*it > max_fd) {
            max_fd = *it;
        }
    }

    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    std::cout << "Before select" << std::endl;
	
    int selectResult = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
    if (selectResult < 0) {
        perror("Error on select");
        exit(EXIT_FAILURE);
    }
    std::cout << "After select, result: " << selectResult << std::endl;

    for (std::vector<int>::iterator it = sockets.begin(); it != sockets.end(); ++it) {
        if (FD_ISSET(*it, &read_fds)) {
            std::cout << "Socket file descriptor before accept: " << *it << std::endl;

            socklen_t clilen = sizeof(cli_addr);
            int newsockfd = accept(*it, (struct sockaddr *)&cli_addr, &clilen);
            if (newsockfd < 0) {
                perror("Error on accept");
                exit(EXIT_FAILURE);
            }
            server.addClient(newsockfd, cli_addr);
            std::cout << "New socket file descriptor: " << newsockfd << std::endl;
            return newsockfd;
        }
    }

    std::cout << "No client was ready to connect" << std::endl;
    return -1;
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
		//server.handlePostRequest(path, request);
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


void ServerInfo::handleGetRequest(const std::string& path, ServerInfo& server)
{
	std::string fullPath = "resources/website" + (path == "/" ? "/index.html" : path);
	std::ifstream file(fullPath.c_str());

	if (!file)
	{
		server.setResponse("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\nERROR 404\n");
	}
	else if (is_directory(fullPath))
	{
		// If the path is a directory, return a response with the directory content
		server.setResponse("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nDirectory content");
	}
	else
	{
		// If the file exists and is not a directory, return a response with the file content
		std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		server.setResponse("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + fileContent);
	}
}

void runServer(std::vector<ServerInfo>& servers)
{
    for(std::vector<ServerInfo>::iterator it = servers.begin(); it != servers.end(); ++it) {
        ServerInfo& server = *it;
        std::vector<int>& sockets = server.getSockets();
        std::cout << "Running server with socket file descriptors: ";
        for (std::vector<int>::iterator it = sockets.begin(); it != sockets.end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;
        sockaddr_in cli_addr;
        while (1)
        {
            int newsockfd = acceptConnection(sockets, server, cli_addr);
            if (newsockfd < 0)
                continue;
            std::string request = readRequest(newsockfd);
            //std::string request = receiveRequest(newsockfd);
			//std::cout << "Running server on port: " << server.getPortListen() << std::endl;
            processRequest(request, server);
            write(newsockfd, server.getResponse().c_str(), server.getResponse().length());
            std::cout << "Before close, socket file descriptor: " << newsockfd << std::endl; // *DEBUG*
			close(newsockfd);
			server.removeSocketFromList(newsockfd);
        }
    }
}
