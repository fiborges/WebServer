/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/06 14:01:17 by brolivei          #+#    #+#             */
/*   Updated: 2024/05/08 17:13:22 by brolivei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"
#include <fstream>

CGI::CGI()
{}

CGI::~CGI()
{}

void	CGI::PerformCGI(const int ClientSocket, std::string buffer_in)
{
	// Read the HTTP request headers
	std::string	header(buffer_in);

    // Open a file for writing
    std::ofstream outputFile("output_file.txt", std::ios::out | std::ios::binary);
    if (!outputFile.is_open())
    {
        std::cerr << "Error: Unable to open file for writing." << std::endl;
        return;
    }

    // Write the contents of buffer_in to the file
    outputFile.write(buffer_in.c_str(), buffer_in.size());

    // Close the file
    outputFile.close();
	//std::cout << "Buffer_in:\n\n";
	//std::cout << buffer_in;

// ===============================================================

	// Extract the boundary string from the content-Type header
	std::string	boundary;
	size_t	boundary_pos = header.find("boundary=");
	// if (boundary_pos != std::string::npos)
	// 	boundary = header.substr(boundary_pos + 9); //Length of "boundary="
	if (boundary_pos != std::string::npos)
	{
		while (header[boundary_pos + 9] != '\n' && header[boundary_pos + 9] != '\r')
		{
			boundary = boundary + header[boundary_pos + 9];
			boundary_pos++;
		}
	}
	//std::cout << "Here is the Boundary:\n\n";
	//std::cout << boundary;

	//std::cout << "\nFinish here!!!!\n";
	// Read the request body and extract file data

	std::string	file_data;
	//size_t	body_start = header.find("\r\n\r\n");
	size_t	bound_start = header.find(boundary);
	size_t	body_start = header.find(boundary, bound_start + boundary.length());

	if (body_start != std::string::npos)
	{
		//body_start += 4;
		//body_start += boundary.length();
		file_data = header.substr(body_start);
	}

	//std::cout << "Here is the File_Data:\n\n";
	//std::cout << file_data;
	//std::cout << "\nFinish here!!\n";
	// Construct the command-line argument containing the request body
    //std::string python_arg = "--request-body=" + file_data;
	std::string python_arg = file_data;
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

		//dup2(this->P_FD[1], STDOUT_FILENO);

		const char*	python_args[4];

		python_args[0] = "/usr/bin/python3";
		python_args[1] = "./testExecutable/U_File_test2.py";
		python_args[2] = python_arg.c_str(); // Pass request body as argument
		python_args[3] = NULL;

		//std::cout << "Request body argument sending to the script:\n";
		//std::cout << python_args[2];
		dup2(this->P_FD[1], STDOUT_FILENO);

		execve(python_args[0], const_cast<char**>(python_args), NULL);

		std::cout << "Error in execve\n";
		exit(EXIT_FAILURE);
	}
	else
	{
		int	read_bytes;
		char	buffer[1024];

		close(this->P_FD[1]);

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
