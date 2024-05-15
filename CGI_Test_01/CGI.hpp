/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/06 13:59:11 by brolivei          #+#    #+#             */
/*   Updated: 2024/05/15 15:47:46 by brolivei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
#define CGI_HPP

#include <iostream>
#include <sstream>

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <string.h>

class CGI
{
	private:
		int	ClientSocket_;
		int	P_FD[2];
		int	pid;

		void	Child_process(std::string& fileName, std::string& fileContent);
		void	Parent_process();
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
