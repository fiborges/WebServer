// Server side C program to demonstarte Socket programming

// Source: https://medium.com/from-the-scratch/http-server-what-do-you-need-to-know-to-build-a-simple-http-server-from-scratch-d1ef8945e4fa

#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/wait.h>
#include <fstream>
#include <string>

#include "CGI.hpp"

#define PORT 8080

void send_html_page(int client_socket) {
    std::ifstream file("./html_test/index.html");
    if (!file.is_open()) {
        std::cerr << "Failed to open HTML file." << std::endl;
        return;
    }

    std::string line;
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    while (std::getline(file, line)) {
        response += line;
    }
	//std::cout << "==========\n\n" << response << "==========\n\n";
    send(client_socket, response.c_str(), response.size(), 0);
}

std::string	read_all_body(int client_socket)
{
	char	line[1024];
	std::string	until_body;

	//while (1)
	//{
	//	int bytes = read(client_socket, line, 1024);
	//	if (bytes == -1)
	//	{
	//		std::cerr << "Error on recv\n";
	//		exit(EXIT_FAILURE);
	//	}
	//	else if (bytes == 0)
	//		break;
	//	std::string	tmp(line);
	//	until_body.append(tmp);
	//	//if (until_body.find("\r\n\r\n"))
	//	//	break;
	//}


	while (1)
	{
		memset(line, 0, 1024);
		ssize_t bytesRead = read(client_socket, line, 1023);

		if (bytesRead < 0)
		{
			//handleError("Error reading from socket.");
			exit(-1);
		}

		//until_body += line;
		until_body.append(line, bytesRead);

		if (bytesRead < 1023)
			break;
	}

	std::cout << "\n\n=====What we extract from client request here:=====\n\n";
	std::cout << until_body;

	//if (until_body.find("Content-Length:") != std::string::npos)
	//{
	//	std::cout << "\nWe found the Content-Length header!\n";
	//	int	content_length;

	//	int i = 16;
	//	std::string	n_bytes;

	//	while(std::isdigit(until_body[until_body.find("Content-Length: ") + i]))
	//	{
	//		std::cout << until_body[until_body.find("Content-Length: ") + i] << std::endl;
	//		n_bytes = n_bytes + until_body[until_body.find("Content-Length: ") + i];
	//		i++;
	//	}

	//	content_length = atoi(n_bytes.c_str());
	//	std::cout << "\nn_bytes string = " << n_bytes << std::endl;
	//	std::cout << "\nContent length found = " << content_length << std::endl;

	//	char	rest_of_body[content_length];

	//	recv(client_socket, rest_of_body, content_length, 0);

	//	until_body.append(rest_of_body);

	//	std::cout << "\n\n=====ALL body=====\n\n";
	//	std::cout << until_body;
	//}

	return (until_body);
}

int	main(int argc, char **argv, char **envp)
{
	(void)argc;
	(void)argv;
	(void)envp;
	int		server_fd, new_socket;
	//long	valread = 1;
	struct	sockaddr_in	address;

	CGI	cgi;

	int		addrlen = sizeof(address);
	std::string	hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		write(2, "In socket", 9);
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	memset(address.sin_zero, '\0', sizeof(address.sin_zero));

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("In bind");
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, 10) < 0)
	{
		perror("In listen");
		exit(EXIT_FAILURE);
	}

	while (1)
	{
		printf("\n========== Waiting for new connection ==========\n\n");
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
		{
			perror("In accept");
			exit(EXIT_FAILURE);
		}

		//char	buffer[30000] = {0};
		std::string	buffer_in_string;

		buffer_in_string = read_all_body(new_socket);

		//read(new_socket, buffer, 30000);
		//printf("%s\n", buffer);

		if (strstr(buffer_in_string.c_str(), "GET /index.html") != NULL)
			send_html_page(new_socket);
		else if (strstr(buffer_in_string.c_str(), "POST /cgi-bin") != NULL)
			cgi.PerformCGI(new_socket, buffer_in_string);
		else
			write(new_socket, hello.c_str(), hello.length());
		printf("==========Response sent==========\n");
		close(new_socket);
	}
	close(server_fd);
	return (0);
}
