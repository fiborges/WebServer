/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CR.hpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/04 14:07:37 by brolivei          #+#    #+#             */
/*   Updated: 2024/07/10 15:06:20 by brolivei         ###   ########.fr       */
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
		CR(std::string Cleaned);

		static bool		ItIsChunked(std::string& Request);
		bool		TheRequestIsFinished();
		void		CheckTheChunk();

		std::string	HandleRequest(std::string RequestFromBegin);
		std::string	ProcessChunked();

		class	CR_ExceptionClass : public std::exception
		{
			private:
				int	ErrorCode_;
			public:
				CR_ExceptionClass(int ErrorCode) : ErrorCode_(ErrorCode) {}
				int	GetErrorCode() const { return (this->ErrorCode_); }
				virtual const char* what() const throw();
		};
};

#endif
