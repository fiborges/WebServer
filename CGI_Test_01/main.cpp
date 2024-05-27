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

void send_html_page(int client_socket)
{
	std::ifstream file("./html_test/index.html");

	if (!file.is_open())
	{
		std::cerr << "Failed to open HTML file." << std::endl;
		return;
	}

	std::string line;
	std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";

	while (std::getline(file, line))
	{
		response += line;
	}

	send(client_socket, response.c_str(), response.size(), 0);
}

void	print_the_request(std::string& total_request)
{
	// std::cout << "\n\n===============END POINT OF THE TEST===============\n\n";

	// std::cout << "\n\n===============WRITING ALL THE REQUEST IN THE SCREEN===============\n\n";

	// std::cout << total_request;

	// std::cout << "\n\n===============END POINT OF THE REQUEST===============\n\n";

	std::ofstream	out("LOG.txt", std::ios::binary | std::ios::app);

	if (!out.is_open())
	{
		return ;
	}

	out.write(total_request.c_str(), total_request.size());

	out.close();

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

	print_the_request(total_request);

	return (total_request);
}

int	main(int argc, char **argv, char **envp)
{
	(void)argc;
	(void)argv;
	(void)envp;
	int		server_fd, new_socket;
	struct	sockaddr_in	address;

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

		std::string	buffer_in_string;

		buffer_in_string = read_all_body(new_socket);

		if (buffer_in_string.find("GET /index.html") != std::string::npos)
			send_html_page(new_socket);
		else if (buffer_in_string.find("POST /cgi-bin") != std::string::npos)
		{
			CGI	cgi;
			cgi.PerformCGI(new_socket, buffer_in_string);
		}
		else if (buffer_in_string.find("GET /close_server") != std::string::npos)
		{
			close(new_socket);
			close(server_fd);
			break;
		}
		else
			write(new_socket, hello.c_str(), hello.length());
		printf("==========Response sent==========\n");
		close(new_socket);
	}
	return (0);
}
