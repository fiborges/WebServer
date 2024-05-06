/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI_Test.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/29 16:53:21 by brolivei          #+#    #+#             */
/*   Updated: 2024/05/06 13:59:49 by brolivei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_TEST_HPP
#define CGI_TEST_HPP

#include <iostream>

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

class CGI
{
	private:
		int	ClientSocket_;
		int	P_FD[2];
		int	pid;
	public:
		// Orthodox
		CGI();
		//CGI(const CGI& other);
		//const CGI&	operator=(const CGI& other);
		~CGI();

		// Public Method
		void	PerformCGI(const int ClientSocket);
};

#endif
