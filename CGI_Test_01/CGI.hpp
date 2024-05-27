/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/06 13:59:11 by brolivei          #+#    #+#             */
/*   Updated: 2024/05/27 14:30:54 by brolivei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
#define CGI_HPP

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <string>

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>

#include "LOG_CLASS.hpp"

class CGI
{
	private:
		int	ClientSocket_;
		int	P_FD[2];
		int	C_FD[2];
		int	pid;

		std::string	FinalBoundary_;
		std::string	Body_;
		std::string	FileName_;
		std::string	FileContent_;

		std::vector<std::string>	EnvStrings_;
		std::vector<char*>			Env_;

		void	Child_process();
		void	Parent_process();

		void	FindFinalBoundary(std::string& buffer);
		void	ExtractBody(std::string& buffer);
		void	ExtractFileName();
		void	ExtractFileContent();

		void	SendContentToScript();

		void	CreateEnv();

	public:
		// Orthodox
		CGI();
		//CGI(const CGI& other);
		//const CGI&	operator=(const CGI& other);
		~CGI();

		// Public Method
		void	PerformCGI(const int ClientSocket, std::string& buffer_in);
};

#endif
