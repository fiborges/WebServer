/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/06 13:59:11 by brolivei          #+#    #+#             */
/*   Updated: 2024/07/15 11:08:07 by brolivei         ###   ########.fr       */
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
#include <sys/stat.h>

#include "../includes/conf_info.hpp"
#include "../includes/RequestParser.hpp"
#include "CR.hpp"
//#include "LOG_CLASS.hpp"

class CGI
{
	private:
		int	ClientSocket_;
		int	P_FD[2];
		int	C_FD[2];
		int	pid;

		CR	Chunks;

		std::map<std::string, std::string>::iterator it_;

		conf_File_Info	Info_;
		HTTrequestMSG	Request_;

		std::string	FinalBoundary_;
		std::string	Body_;
		std::string	FileName_;
		std::string	FileContent_;
		// PATH_INFO:
		std::string	Path_Info_;
		std::string	ScriptURI_;
		std::string	TotalRequest_;

		std::string	FinalResponse;

		std::vector<std::string>	EnvStrings_;
		std::vector<char*>			Env_;

		void	Child_process();
		std::string&	Parent_process();

		void	CreateScriptURI();
		void	ExtractPathInfo(std::string& buffer, conf_File_Info& info);
		void	FindFinalBoundary(std::string& buffer);
		void	ExtractBody(std::string& buffer);
		void	ExtractFileName();
		void	ExtractFileContent();
		void	ExtractChunkBody();
		void	ExtractFormFromBody();

		void	SendContentToScript();
		void	SendAllRequestToScript();

		void	CreateEnv();

		bool	FileExists(const std::string& path);
		bool	DirExists(const std::string& path);

		void	WaitFiveSeconds();

		std::string	GetUploadDir(const std::string& path);

	public:
		// Orthodox
		CGI();
		~CGI();

		CGI(conf_File_Info info, HTTrequestMSG request);

		// Public Method
		std::string&	PerformCGI(const int ClientSocket, std::string& buffer_in);

		class	CGI_ExceptionClass : public std::exception
		{
			private:
				int	ErrorCode_;
			public:
				CGI_ExceptionClass(int ErrorCode) : ErrorCode_(ErrorCode) {}
				int	GetErrorCode() const { return (this->ErrorCode_); }
				virtual const char* what() const throw();
		};

		class	NoScriptAllowed : public std::exception
		{
			private:
				//int	ErrorCode_;
			public:
				virtual const char* what() const throw();
		};

		class	NotAcceptedUploadPath : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class	NoUploadPathConfigurated : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class	NonexistentScript : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class	WrongCGIPath : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};
};

#endif
