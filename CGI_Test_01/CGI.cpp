/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/06 14:01:17 by brolivei          #+#    #+#             */
/*   Updated: 2024/05/06 17:01:20 by brolivei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

CGI::CGI()
{}

CGI::~CGI()
{}

void	CGI::PerformCGI(const int ClientSocket, char buffer_in[30000])
{
	// Read the HTTP request headers
	std::string	header(buffer_in);

// ===============================================================

	// Extract the boundary string from the content-Type header
	std::string	boundary;
	size_t	boundary_pos = header.find("boundary=");
	if (boundary_pos != std::string::npos)
		boundary = header.substr(boundary_pos + 9); //Length of "boundary="

	// Read the request body and extract file data

	std::string	file_data;
	size_t	body_start = header.find("\r\n\r\n");

	if (body_start != std::string::npos)
	{
		body_start += 4;
		file_data = header.substr(body_start);
	}

	// Construct the command-line argument containing the request body
    std::string python_arg = "--request-body=" + file_data;
// ===============================================================

	// Pipe creation
	if (pipe(this->P_FD) == -1)
	{
		std::cout << "Error in pipe\n";
		exit(EXIT_FAILURE);
	}

	// Fork
	this->pid = fork();
	if (this->pid == -1)
	{
		std::cout << "Error in fork\n";
		exit(EXIT_FAILURE);
	}

	if (this->pid == 0)
	{
		close(this->P_FD[0]);

		dup2(this->P_FD[1], STDOUT_FILENO);

		const char*	python_args[4];

		python_args[0] = "/usr/bin/python3";
		python_args[1] = "./testExecutable/U_File_test2.py";
		python_args[2] = python_arg.c_str(); // Pass request body as argument
		python_args[3] = NULL;

		execve(python_args[0], const_cast<char**>(python_args), NULL);

		std::cout << "Error in execve\n";
		exit(EXIT_FAILURE);
	}

	else
	{
		int	read_bytes;
		char	buffer[1024];

		close(this->P_FD[1]);

		// Write file data to the read end of the pipe
		//write(this->P_FD[0], file_data.c_str(), file_data.length());
		//size_t	bytes_written = 0;

		//while (bytes_written < file_data.length())
		//{
		//	ssize_t	bytes_written_now = write(this->P_FD[0], file_data.c_str() + bytes_written, file_data.length() - bytes_written);
		//	if (bytes_written_now < 0)
		//	{
		//		std::cerr << "Error writing to pipe\n";
		//		exit(EXIT_FAILURE);
		//	}
		//	bytes_written += bytes_written_now;
		//}

		// Read output of the Python script from the pipe
		std::string	pyOutPut;

		while ((read_bytes = read(this->P_FD[0], buffer, 1024)) > 0)
		{
			pyOutPut.append(buffer);
		}
		wait(NULL);
		close(this->P_FD[0]);

		// Send the Python script output back to the client

		std::stringstream	s;

		s << "HTTP/1.1 200 OK\r\nContent-Length: " << pyOutPut.length() << "\r\n\r\n" << pyOutPut;

		std::string	response = s.str();

		write(ClientSocket, response.c_str(), response.length());
	}
}
