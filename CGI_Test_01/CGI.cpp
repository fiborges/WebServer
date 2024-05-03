/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/29 16:58:42 by brolivei          #+#    #+#             */
/*   Updated: 2024/05/03 12:25:50 by brolivei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

CGI::CGI()
{}

CGI::~CGI()
{}

// ==========PublicMethod==========

void	CGI::PerformCGI(const int ClientSocket)
{
	this->ClientSocket_ = ClientSocket;
	if (pipe(this->P_FD) == -1)
	{
		write(STDERR_FILENO, "Pipe error\n", 11);
		exit(EXIT_FAILURE);
	}

	this->pid = fork();

	if (pid < 0)
	{
		write(STDERR_FILENO, "Fork error\n", 11);
		exit(EXIT_FAILURE);
	}

	if (pid == 0)
	{
		close(this->P_FD[0]);
		dup2(this->P_FD[1], STDOUT_FILENO);

		const char*	args[2];

		args[0] = "./testExecutable/PmergeMe";
		args[1] = "5";
		args[2] = "2";
		args[3] = "4";
		args[4] = "1";
		args[5] = "1";
		args[6] = "0";
		args[7] = NULL;

		execve(args[0], const_cast<char**>(args), NULL);

		write(STDERR_FILENO, "Execve fail\n", 12);
		exit(EXIT_FAILURE);
	}

	else
	{
		close(this->P_FD[1]);

		char	buffer[1024];
		int		bytes;

		std::string	header = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
		write(this->ClientSocket_, header.c_str(), header.length());

		while ((bytes = read(this->P_FD[0], buffer, sizeof(buffer))) > 0)
		{
			write(this->ClientSocket_, buffer, strlen(buffer));
		}

		close(this->P_FD[0]);
		wait(NULL);
	}
}

