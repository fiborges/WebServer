// Server side C program to demonstarte Socket programming

// Source: https://medium.com/from-the-scratch/http-server-what-do-you-need-to-know-to-build-a-simple-http-server-from-scratch-d1ef8945e4fa

#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
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
	std::cout << "==========\n\n" << response << "==========\n\n";
    send(client_socket, response.c_str(), response.size(), 0);
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

		char	buffer[30000] = {0};
		std::string	buffer_in_string;
		//while (valread > 0)
		//{
		read(new_socket, buffer, 30000);
		printf("%s\n", buffer);
		//}

		if (strstr(buffer, "GET /index.html") != NULL)
			send_html_page(new_socket);
		else if (strstr(buffer, "POST /cgi-bin") != NULL)
			cgi.PerformCGI(new_socket, buffer);
		else
			write(new_socket, hello.c_str(), hello.length());
		printf("==========Response sent==========\n");
		close(new_socket);
	}
	close(server_fd);
	return (0);
}