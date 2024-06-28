/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/06 14:01:17 by brolivei          #+#    #+#             */
/*   Updated: 2024/06/28 14:57:42 by brolivei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/CGI.hpp"

CGI::CGI()
{}

CGI::~CGI()
{
	std::cout << "CGI Destructor\n";
}

CGI::CGI(conf_File_Info info, HTTrequestMSG request)
{
	this->Info_ = info;
	this->Request_ = request;
}

bool	CGI::FileExists(const std::string& path)
{
	struct stat	buffer;
	return (stat(path.c_str(), &buffer) == 0);
}

void	CGI::CreateScriptURI()
{
	size_t	pyPosition = this->Request_.path.find(".py");

	if (pyPosition != std::string::npos)
	{
		for (size_t i = 0; i != pyPosition + 3; i++)
			this->ScriptURI_ += this->Request_.path[i];
	}
	else
		throw NoScriptAllowed();

	std::cout << "SCRIPT_URI CREATED:\n\n";
	std::cout << this->ScriptURI_ << "\n\n";

	if (this->ScriptURI_.substr(0, this->Info_.Path_CGI.size()) != this->Info_.Path_CGI)
		throw NotAcceptedUploadPath();
	this->ScriptURI_.insert(0, ".");

	std::cout << "SCRIPT_URI AFTER INSERTION:\n\n";
	std::cout << this->ScriptURI_ << "\n\n";

	if (!FileExists(this->ScriptURI_))
		throw NonexistentScript();
}

void	CGI::ExtractPathInfo(std::string& buffer, conf_File_Info& info)
{
	if (buffer.find("/UploadScript.py") == std::string::npos)
		throw NoScriptAllowed();

	size_t	path_position = buffer.find("/UploadScript.py") + 16;

	while (buffer[path_position] != ' ')
		this->Path_Info_ += buffer[path_position++];

	// 	Necessario verificar aqui se PATH_INFO está de acordo com o diretorio de uploads autorizado
	// no ficheiro de configuração.

	std::cout << "PATH_INFO FOUND: " << this->Path_Info_ << std::endl;

	if (info.fileUploadDirectory.find(this->Path_Info_) == std::string::npos)
		throw NotAcceptedUploadPath();

	// Uma das primeiras verificações será ver se estão a tentar aceder a um diretorio transversal:

	if (this->Path_Info_.find("..") != std::string::npos)
		throw NotAcceptedUploadPath();
}

void	CGI::FindFinalBoundary(std::string& buffer)
{
	ssize_t	boundPosition = buffer.find("boundary=");

	while (std::isalnum(buffer[boundPosition + 9]) || buffer[boundPosition + 9] == '-')
	{
		this->FinalBoundary_ += buffer[boundPosition + 9];
		boundPosition++;
	}
	this->FinalBoundary_.insert(0, "--");
	this->FinalBoundary_.append("--");
}

void	CGI::ExtractBody(std::string& buffer)
{
	ssize_t	boundStart = buffer.find("\r\n\r\n");

	this->Body_.append(buffer, boundStart + 4);
}

void	CGI::ExtractFileName()
{
	ssize_t	fileNamePos = this->Body_.find("filename=");

	while (this->Body_[fileNamePos + 10] != '"')
	{
		this->FileName_ += this->Body_[fileNamePos + 10];
		fileNamePos++;
	}
}

void	CGI::ExtractFileContent()
{
	ssize_t	ContentStart = this->Body_.find("\r\n\r\n") + 4;
	ssize_t	ContentEnd = this->Body_.find(this->FinalBoundary_) - 1;

	while (ContentStart < ContentEnd)
	{
		this->FileContent_ += this->Body_[ContentStart];
		ContentStart++;
	}
}

void	CGI::SendContentToScript()
{
	size_t	contentLength = this->FileContent_.size();
	size_t	bytesWritten = 0;

	while (bytesWritten < contentLength)
	{
		size_t	chunkSize = std::min(contentLength - bytesWritten, static_cast<size_t>(PIPE_BUF));
		ssize_t	bytes = write(this->P_FD[1], this->FileContent_.data() + bytesWritten, chunkSize);

		if (bytes < 0)
		{
			std::cerr << "Error writtting to pipe\n";
			break;
		}
		bytesWritten += bytes;
	}
	close(this->P_FD[1]);
}

void	CGI::CreateEnv()
{
	std::string	key;
	std::string	value;

	key = "PATH_INFO=";
	value = this->ScriptURI_;

	this->EnvStrings_.push_back(key + value);

	key = "UPLOAD_DIR=";
	value = this->Info_.fileUploadDirectory;

	this->EnvStrings_.push_back(key + value);

	std::cout << "UPLOAD_DIR: " << this->Info_.fileUploadDirectory << std::endl;

	key = "CONTENT_LENGTH=";
	value = this->Request_.content_length;

	this->EnvStrings_.push_back(key + value);

	key = "FILE_NAME=";

	this->EnvStrings_.push_back(key + this->FileName_);

	key = "GATEWAY_INTERFACE=";
	value = "CGI/1.1";

	this->EnvStrings_.push_back(key + value);

	key = "QUERY_STRING=";
	value = this->Request_.query;

	this->EnvStrings_.push_back(key + value);

	key = "REQUEST_METHOD=";
	value = this->Request_.cgi_env["REQUEST_METHOD"];

	this->EnvStrings_.push_back(key + value);

	for (size_t	i = 0; i < this->EnvStrings_.size(); i++)
		this->Env_.push_back(const_cast<char*>(this->EnvStrings_[i].c_str()));
	this->Env_.push_back(NULL);
}

void	CGI::PerformCGI(const int ClientSocket, std::string& buffer)
{
	if (this->Info_.fileUploadDirectory.empty())
		throw NoUploadPathConfigurated();

	if (this->Request_.cgi_env["REQUEST_METHOD"] == "GET")
	{
		std::cout << "Dealing with get request\n";
		this->ClientSocket_ = ClientSocket;

		CreateScriptURI();
		CreateEnv();
		return ;
	}

	else
	{
		// std::cout << "SERVER_INFO:\n\n";

		// std::cout << this->Info_.defaultFile << std::endl;
		// std::cout << this->Info_.RootDirectory << std::endl;
		// std::cout << "PATH_CGI: " << this->Info_.Path_CGI << std::endl;
		// std::cout << this->Info_.fileUploadDirectory << std::endl << std::endl;

		// std::cout << "REQUEST_INFO:\n\n";

		// std::cout << this->Request_.method << std::endl;
		// std::cout << this->Request_.path << std::endl;
		// std::cout << this->Request_.query << std::endl;
		// std::cout << this->Request_.content_length << std::endl;

		// for (std::map<std::string, std::string>::iterator it = this->Request_.cgi_env.begin(); it != this->Request_.cgi_env.end(); it++)
		// 	std::cout << it->first << " | " << it->second << std::endl;

		this->ClientSocket_ = ClientSocket;

		CreateScriptURI();

		//ExtractPathInfo(buffer, this->Info_);

		/*
			FindFinalBoundary will find the boundary Header and had two '-' characters
		in the beginning and two of them in the end. That will give us the Final boundary
		since this is the rule of that boundary.

		*/

		FindFinalBoundary(buffer);

		/*
			ExtractBody, making use of the rule, after the headers, the sequence "\r\n\r\n"
		is allways there, it extract all the body of the request, from the starting boundary
		until the end boundary.

		*/

		ExtractBody(buffer);

		/*
			ExtractFileName just gets the filename header in the body of the request to be
		given to the script.

		*/

		ExtractFileName();

		ExtractFileContent();

		CreateEnv();
	}

	// LOG_CLASS::CreateLog("FinalBoundary", this->FinalBoundary_);

	// LOG_CLASS::CreateLog("Body", this->Body_);

	// LOG_CLASS::CreateLog("FileName", this->FileName_);

	// LOG_CLASS::CreateLog("FileContent", this->FileContent_);

	// Creating Pipe
	if (pipe(this->P_FD) == -1 || pipe(this->C_FD) == -1)
	{
		std::cerr << "Error in pipe\n";
		exit (EXIT_FAILURE);
	}

	// Forking the program
	this->pid = fork();
	if (this->pid == -1)
	{
		std::cerr << "Error in Fork\n";
		exit (EXIT_FAILURE);
	}

	if (this->pid == 0)
		Child_process();
	else
		Parent_process();
}

void	CGI::Child_process()
{
	// this->P_FD[0] -> ReadEnd
	// this->P_FD[1] -> WriteEnd
	close(this->C_FD[0]);
	close(this->P_FD[1]);
	dup2(this->P_FD[0], STDIN_FILENO);
	dup2(this->C_FD[1], STDOUT_FILENO);
	close(this->P_FD[0]);
	close(this->C_FD[1]);

	const char*	python_args[3];

	std::string tmp = this->Info_.Path_CGI;

	tmp.insert(0, ".");

	if (chdir(tmp.c_str()) != 0)
	{
		std::cout << "Could not change to CGI directory\n";
		return ;
	}

	std::string	tmp2; // Becouse we change the directory we need to re-new the Script_URI

	size_t	script_place = this->ScriptURI_.find(tmp);
	for (size_t i = script_place + tmp.size(); i != this->ScriptURI_.size(); i++)
		tmp2 += this->ScriptURI_[i];
	tmp2.insert(0, ".");

	python_args[0] = "/usr/bin/python3";
	//python_args[1] = "./cgi-bin/UploadScript.py";
	//python_args[1] = this->ScriptURI_.c_str();
	python_args[1] = tmp2.c_str();
	python_args[2] = NULL;

	std::cout << "PATH where the script is running: " << tmp << std::endl;
	std::cout << "SCRIPT_URI FROM CGI DIRECTORY: " << tmp2 << std::endl;

	execve(python_args[0], const_cast<char**>(python_args), this->Env_.data());

	std::cerr << "Error in execve\n";
	exit(EXIT_FAILURE);
}

void	CGI::Parent_process()
{
	close(this->C_FD[1]);
	close(this->P_FD[0]);
	dup2(this->C_FD[0], STDIN_FILENO);

	if (this->Request_.cgi_env["REQUEST_METHOD"] == "POST")
		SendContentToScript();

	wait(NULL);

	char		line[1024];
	std::string	response;

	response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";

	while (1)
	{
		memset(line, 0, 1024);
		ssize_t	bytesRead = read(this->C_FD[0], line, 1023);

		if (bytesRead < 0)
		{
			std::cerr << "Error in read\n";
			exit(-1);
		}

		response.append(line, bytesRead);

		if (bytesRead < 1023)
			break;
	}
	close(this->C_FD[0]);
	std::cout << "Response: " << response << std::endl;
	send(this->ClientSocket_, response.c_str(), response.size(), 0);
}



// ===========================Exceptions

const char*	CGI::NoScriptAllowed::what() const throw()
{
	return ("ALERT: CGI request with not allowed script\n");
}

const char*	CGI::NotAcceptedUploadPath::what() const throw()
{
	return ("ALERT: CGI upload path was not accepted\n");
}

const char* CGI::NoUploadPathConfigurated::what() const throw()
{
	return ("ALERT: CGI there's no upload directory configurated\n");
}

const char* CGI::NonexistentScript::what() const throw()
{
	return ("ALERT: CGI Nonexistent Script");
}



























// void	CGI::PerformCGI(const int ClientSocket, std::string buffer_in)
// {
// 	// Read the HTTP request headers
// 	std::string	header(buffer_in);

//     // Open a file for writing
//     std::ofstream outputFile("output_file.txt", std::ios::out | std::ios::binary);
//     if (!outputFile.is_open())
//     {
//         std::cerr << "Error: Unable to open file for writing." << std::endl;
//         return;
//     }

//     // Write the contents of buffer_in to the file
//     outputFile.write(buffer_in.c_str(), buffer_in.size());

//     // Close the file
//     outputFile.close();
// 	//std::cout << "Buffer_in:\n\n";
// 	//std::cout << buffer_in;

// // ===============================================================

// 	// Extract the boundary string from the content-Type header
// 	std::string	boundary;
// 	size_t	boundary_pos = header.find("boundary=");
// 	// if (boundary_pos != std::string::npos)
// 	// 	boundary = header.substr(boundary_pos + 9); //Length of "boundary="
// 	if (boundary_pos != std::string::npos)
// 	{
// 		while (header[boundary_pos + 9] != '\n' && header[boundary_pos + 9] != '\r')
// 		{
// 			boundary = boundary + header[boundary_pos + 9];
// 			boundary_pos++;
// 		}
// 	}
// 	//std::cout << "Here is the Boundary:\n\n";
// 	//std::cout << boundary;

// 	//std::cout << "\nFinish here!!!!\n";
// 	// Read the request body and extract file data

// 	std::string	file_data;
// 	//size_t	body_start = header.find("\r\n\r\n");
// 	size_t	bound_start = header.find(boundary);
// 	size_t	body_start = header.find(boundary, bound_start + boundary.length());

// 	if (body_start != std::string::npos)
// 	{
// 		//body_start += 4;
// 		//body_start += boundary.length();
// 		file_data = header.substr(body_start);
// 	}

// 	//std::cout << "Here is the File_Data:\n\n";
// 	//std::cout << file_data;
// 	//std::cout << "\nFinish here!!\n";
// 	// Construct the command-line argument containing the request body
//     //std::string python_arg = "--request-body=" + file_data;
// 	std::string python_arg = file_data;
// // ===============================================================

// 	// Pipe creation
// 	if (pipe(this->P_FD) == -1)
// 	{
// 		std::cout << "Error in pipe\n";
// 		exit(EXIT_FAILURE);
// 	}

// 	// Fork
// 	this->pid = fork();
// 	if (this->pid == -1)
// 	{
// 		std::cout << "Error in fork\n";
// 		exit(EXIT_FAILURE);
// 	}

// 	if (this->pid == 0)
// 	{
// 		close(this->P_FD[0]);

// 		//dup2(this->P_FD[1], STDOUT_FILENO);

// 		const char*	python_args[4];

// 		python_args[0] = "/usr/bin/python3";
// 		python_args[1] = "./testExecutable/U_File_test2.py";
// 		python_args[2] = python_arg.c_str(); // Pass request body as argument
// 		python_args[3] = NULL;

// 		//std::cout << "Request body argument sending to the script:\n";
// 		//std::cout << python_args[2];
// 		dup2(this->P_FD[1], STDOUT_FILENO);

// 		execve(python_args[0], const_cast<char**>(python_args), NULL);

// 		std::cout << "Error in execve\n";
// 		exit(EXIT_FAILURE);
// 	}
// 	else
// 	{
// 		int	read_bytes;
// 		char	buffer[1024];

// 		close(this->P_FD[1]);

// 		std::string	pyOutPut;

// 		while ((read_bytes = read(this->P_FD[0], buffer, 1024)) > 0)
// 		{
// 			pyOutPut.append(buffer);
// 		}
// 		wait(NULL);
// 		close(this->P_FD[0]);

// 		// Send the Python script output back to the client

// 		std::stringstream	s;

// 		s << "HTTP/1.1 200 OK\r\nContent-Length: " << pyOutPut.length() << "\r\n\r\n" << pyOutPut;

// 		std::string	response = s.str();

// 		write(ClientSocket, response.c_str(), response.length());
// 	}
// }
