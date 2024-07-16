/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/06 14:01:17 by brolivei          #+#    #+#             */
/*   Updated: 2024/07/16 12:28:08 by brolivei         ###   ########.fr       */
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

bool	CGI::DirExists(const std::string& path)
{
	struct stat buffer;
	if (stat(path.c_str(), &buffer) != 0)
		return false;
	else if (buffer.st_mode & S_IFDIR)
		return true;

	return false;
}

void	CGI::CreateScriptURI()
{
	size_t	pyPosition = this->Request_.path.find(".py");

	if (pyPosition != std::string::npos)
		for (size_t i = 0; i != pyPosition + 3; i++)
			this->ScriptURI_ += this->Request_.path[i];

	else
		throw CGI_ExceptionClass(415); // erro 415: Unsupported Media Type

	//std::cout << "SCRIPT_URI CREATED:" << this->ScriptURI_ << std::endl;
	//std::cout << "PATH_CGI:" << this->Info_.Path_CGI << std::endl;
	if (this->Info_.Path_CGI.empty())
		throw CGI_ExceptionClass(404);

	if (this->ScriptURI_.substr(0, this->Info_.Path_CGI.size()) != this->Info_.Path_CGI)
		throw CGI_ExceptionClass(404); // Not found the CGI resource
	this->ScriptURI_.insert(0, ".");

	//std::cout << "SCRIPT_URI AFTER INSERTION:" << this->ScriptURI_ << std::endl;

	if (!FileExists(this->ScriptURI_))
		throw CGI_ExceptionClass(404); // The Script does not exist
}

void	CGI::ExtractPathInfo(std::string& buffer, conf_File_Info& info)
{
	if (buffer.find("/UploadScript.py") == std::string::npos)
		throw CGI_ExceptionClass(415);

	size_t	path_position = buffer.find("/UploadScript.py") + 16;

	while (buffer[path_position] != ' ')
		this->Path_Info_ += buffer[path_position++];

	// 	Necessario verificar aqui se PATH_INFO está de acordo com o diretorio de uploads autorizado
	// no ficheiro de configuração.

	//std::cout << "PATH_INFO FOUND: " << this->Path_Info_ << std::endl;

	if (info.fileUploadDirectory.find(this->Path_Info_) == std::string::npos)
		throw NotAcceptedUploadPath();

	// Uma das primeiras verificações será ver se estão a tentar aceder a um diretorio transversal:

	if (this->Path_Info_.find("..") != std::string::npos)
		throw NotAcceptedUploadPath();
}

void	CGI::FindFinalBoundary(std::string& buffer)
{
	size_t	boundPosition = buffer.find("boundary=");

	if (boundPosition == std::string::npos)
		return ;

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

	if (this->Body_.empty())
		throw CGI_ExceptionClass(400);
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

void	CGI::SendAllRequestToScript()
{
	size_t	contentLength = this->TotalRequest_.size();
	size_t	bytesWritten = 0;

	while (bytesWritten < contentLength)
	{
		size_t	chunkSize = std::min(contentLength - bytesWritten, static_cast<size_t>(PIPE_BUF));
		ssize_t	bytes = write(this->P_FD[1], this->TotalRequest_.data() + bytesWritten, chunkSize);

		if (bytes < 0)
		{
			std::cerr << "Error writtting to pipe\n";
			break;
		}
		bytesWritten += bytes;
	}
	close(this->P_FD[1]);
}

std::string	CGI::GetUploadDir(const std::string& path)
{
	//std::cout << "UPLOAD_PATH_RECIEVED:" << path << std::endl;
	if (path[0] == '/')
	{
		if (DirExists(path))
			return path; // É absoluto
		std::string	tmp;
		tmp = path;
		tmp.insert(0, this->Info_.Path_CGI);
		tmp.insert(0, ".");
		//std::cout << "PATH_TOTAL:" << tmp << std::endl;
		if (DirExists(tmp))
			return "." + path;
		throw CGI_ExceptionClass(500); // The directory to upload is not created
	}
	if (path[0] == '.')
	{
		std::string	tmp;

		tmp = path;
		tmp.erase(0, 1);
		tmp.insert(0, this->Info_.Path_CGI);
		tmp.insert(0, ".");
		//std::cout << "PATH_TOTAL:" << tmp << std::endl;
		if (DirExists(tmp))
			return path; // É relativo e está verificado se correto.
		else
			throw CGI_ExceptionClass(500); // The directory to upload is not created
	}
	return "./" + path; // É relativo e vamos verificar se correto.
}

void	CGI::CreateEnv()
{
	std::string	key;
	std::string	value;

	key = "PATH_INFO=";
	value = this->ScriptURI_;

	this->EnvStrings_.push_back(key + value);

	key = "UPLOAD_DIR=";
	//value = this->Info_.fileUploadDirectory;
	value = GetUploadDir(this->Info_.fileUploadDirectory);

	this->EnvStrings_.push_back(key + value);

	std::cout << "UPLOAD_DIR: " << this->Info_.fileUploadDirectory << std::endl;

	key = "CONTENT_LENGTH=";
	value = this->Request_.content_length;

	this->EnvStrings_.push_back(key + value);

	key = "FILE_NAME=";

	CR	Chunk;
	if (Chunk.ItIsChunked(this->TotalRequest_))
		this->EnvStrings_.push_back(key + "chunk.txt");
	else
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

void	CGI::ExtractChunkBody()
{
	size_t	StartBody = this->TotalRequest_.find("\r\n\r\n") + 4;

	while (this->TotalRequest_[StartBody])
		this->Body_ += this->TotalRequest_[StartBody++];

	size_t	EndBody = this->Body_.find("\r\n\r\n");

	this->Body_.erase(EndBody);
	//this->Body_.append('\0');
	this->FileContent_ = this->Body_;

	//std::cout << "BODY_FOUND_IN_CHUNKED:" << this->Body_ << "[FINISH]\n";
}

void	CGI::ExtractFormFromBody()
{
	size_t	BodyStart = this->TotalRequest_.find("\r\n\r\n") + 4;
	int		Length = 0;

	while (Length != this->Request_.content_length)
	{
		this->FileContent_ += this->TotalRequest_[BodyStart++];
		Length++;
	}
}

std::string&	CGI::PerformCGI(const int ClientSocket, std::string& buffer)
{
	CR	Chunk;

	this->TotalRequest_ = buffer;

	if (Chunk.ItIsChunked(buffer) == true)
	{
		//std::cout << "CGI:Dealing with ChunkedRequest\n";

		if (this->Info_.fileUploadDirectory.empty())
			throw CGI_ExceptionClass(500);

		this->ClientSocket_ = ClientSocket;

		CreateScriptURI();
		ExtractChunkBody();
		CreateEnv();
		if (this->FileContent_.empty())
			throw CGI_ExceptionClass(400);
	}

	else if (this->Request_.cgi_env["REQUEST_METHOD"] == "GET")
	{
		//std::cout << "Dealing with get request\n";
		this->ClientSocket_ = ClientSocket;

		CreateScriptURI(); //BAJ
		CreateEnv(); //BAJ
	}
	else
	{
		if (this->Info_.fileUploadDirectory.empty())
			throw CGI_ExceptionClass(500); // The upload path is not configurated in .conf

		this->ClientSocket_ = ClientSocket;

		CreateScriptURI();

		//ExtractPathInfo(buffer, this->Info_);

		/*
			FindFinalBoundary will find the boundary Header and had two '-' characters
		in the beginning and two of them in the end. That will give us the Final boundary
		since this is the rule of that boundary.

		*/
		FindFinalBoundary(buffer);
		if (this->FinalBoundary_.empty() == false)
		{

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
		}
		CreateEnv();
		if (this->TotalRequest_.find("application/x-www-form-urlencoded"))
			ExtractFormFromBody();
		if ((this->FileContent_.empty() || this->FileContent_ == "\r") && this->Request_.query.empty())
			throw CGI_ExceptionClass(400);
	}

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
		this->FinalResponse = Parent_process();
	return (this->FinalResponse);
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
		std::cerr << "Could not change to CGI directory\n";
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

	execve(python_args[0], const_cast<char**>(python_args), this->Env_.data());

	std::cerr << "Error in execve\n";
}

void	CGI::WaitFiveSeconds()
{
	clock_t	start;
	clock_t end;

	start = clock();
	end = clock();
	while ((double(end - start) / CLOCKS_PER_SEC) < 5)
	{
		if (waitpid(this->pid, NULL, WNOHANG) != 0)
			break;
		end = clock();
	}
}

std::string&	CGI::Parent_process()
{
	close(this->C_FD[1]);
	close(this->P_FD[0]);
	dup2(this->C_FD[0], STDIN_FILENO);

	if (this->Chunks.ItIsChunked(this->TotalRequest_) == true)
		SendContentToScript();
	else if (this->Request_.cgi_env["REQUEST_METHOD"] == "POST"  && this->FileContent_.empty() == false)
		SendContentToScript();
	else
	{
		SendAllRequestToScript();
	}

	char		line[1024];

	WaitFiveSeconds();

	if (waitpid(this->pid, NULL, WNOHANG) == 0)
	{
		kill(this->pid, SIGKILL);
		throw CGI_ExceptionClass(504); // Gateway Timeout
	}

	while (1)
	{
		//memset(line, 0, 1024);
		ssize_t	bytesRead = read(this->C_FD[0], line, 1023);

		if (bytesRead < 0)
		{
			std::cerr << "Error in read\n";
			exit(-1);
		}

		this->FinalResponse.append(line, bytesRead);

		if (bytesRead < 1023)
			break;
	}
	close(this->C_FD[0]);

	if ((this->FinalResponse.empty() == true) || (this->FinalResponse.find("HTTP/1.1") == std::string::npos))
	{
		throw CGI_ExceptionClass(500); // Internal error.
	}

	return (this->FinalResponse);
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
	return ("ALERT: CGI Nonexistent Script\n");
}

const char* CGI::WrongCGIPath::what() const throw()
{
	return ("ALERT: CGI Wrong path to cgi script\n");
}

const char* CGI::CGI_ExceptionClass::what() const throw()
{
	return ("ALERT: CGI DETECTED ERROR\n");
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
