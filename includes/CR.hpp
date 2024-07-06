/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CR.hpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/04 14:07:37 by brolivei          #+#    #+#             */
/*   Updated: 2024/07/05 16:01:15 by brolivei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CR_HPP
#define CR_HPP

#include <iostream>
#include <cmath>

#include <sys/types.h>
#include <sys/socket.h>

class	CR
{
	private:
		int			ClientSocket_;
		std::string	FullRequest_;
		std::string	CleanedRequest;

	public:
		CR();
		~CR();

		CR(int ClientSocket);

		bool		ItIsChunked(std::string& Request);
		bool		TheRequestIsFinished();

		void		HandleRequest();
		std::string	ProcessChunked();
};

#endif
