/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fde-carv <fde-carv@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/26 15:14:05 by fde-carv          #+#    #+#             */
/*   Updated: 2024/06/22 15:09:32 by fde-carv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// para post depois de correr  ./webserver config.conf  abrimos novo terminal e corremos
// curl -X POST -d "param1=value1&param2=value2" http://localhost:8080
// curl -X POST -d "param1=value1&param2=value2" http://localhost:8085/post_endpoint


// abrir outro terminal e verificar as portas que estam a ouvir e se estam a ler
// netstat -tuln ou netstat -tuln | grep 8081 ou netstat -tuln | grep '808[0-6]'

// para testar o get
// curl -v http://localhost:8080/index.html

#ifndef GET_HPP
# define GET_HPP

# define GREY "\033[1;30m"
//# define YELLOW "\033[1;33m"
# define YELLOW1 "\033[0;33m"
//# define GREEN "\033[1;32m"
# define GREEN1 "\033[0;32m"
//# define RED "\033[1;31m"
# define RED1 "\033[0;31m"
//# define BLUE "\033[1;34m"
# define BLUE1 "\033[1;34m"
//# define CYAN "\033[1;36m"
# define CYAN1 "\033[0;36m"
# define WHITE "\033[1;37m"
//# define RESET "\033[0m"
# define SBLINK	"\033[5m"
# define BG_GREEN "\033[42m"
//# define MAGENTA "\033[1;35m"
# define REVERSE "\033[7m"
# define BG_MAGENTA "\033[45m"
# define BG_RED "\033[41m"
# define BG_GREEN_YELLOW "\033[42;33m"
# define BG_RED_YELLOW "\033[41;33m"
# define BG_GREEN_BLACK "\033[42;30m"
# define BG_RED_BLACK "\033[41;30m"
# define BG_DARK_GRAY "\033[100m"
# define BG_CYAN "\033[46m"
# define BG_CYAN_BLACK "\033[46;30m"
# define BG_BLUE "\033[44m"
# define BG_BLUE_BLACK "\033[44;30m"
# define RESET_BG "\033[49m"

# include <sys/socket.h> // OK
# include <netinet/in.h> // OK
# include <string.h>
# include <unistd.h>
# include <iostream>
# include <fstream>
# include <sstream>
# include <cstdlib> // for exit() // OK
# include <dirent.h> // for opendir() and closedir() // OK
# include <sys/stat.h> // for mkdir()

# include <ftw.h>
# include <signal.h>
# include <ctime>
# include <list>

# include "conf_info.hpp"
# include "RequestParser.hpp"
# include "parser.hpp"
# include "CGI.hpp"
# include "erros.hpp"

extern volatile sig_atomic_t flag;
extern std::vector<std::string> createdFiles;

class ServerInfo
{
	private:
		int sockfd;
		sockaddr_in serv_addr;
		std::vector<sockaddr_in> cli_addrs;
		std::string response;
		
		std::string rootUrl;
		std::vector<int> clientSockets;
		std::vector<int> portListen;

		size_t contentLength;
		std::string rootOriginalDirectory;
		std::map<int, conf_File_Info> configs;
		std::string complete_path;

	public:
		ServerInfo();
		~ServerInfo();
	
		int	clientSocket;
		int sair;

		void		setSocketFD(int socket);
		int			getSocketFD() const;
		void		setAddress(const sockaddr_in& address);

		void		setResponse(const std::string& response);
		std::string	getResponse() const;

		std::string	getRootUrl() const;
		void		addSocketToList(int sockfd);

		void		handleUnknownRequest(HTTrequestMSG& requestMsg, ServerInfo &server, conf_File_Info &serverConfig);
		void		handleGetRequest(HTTrequestMSG& request, ServerInfo& server, conf_File_Info &serverConfig);
		void		handlePostRequest(HTTrequestMSG& request, ServerInfo &server, conf_File_Info &serverConfig);
		void		handleDeleteRequest(HTTrequestMSG& requestMsg, ServerInfo& server, conf_File_Info &serverConfig);
		void		addPortToList(int port);
		std::vector<int> getPortList() const;
		std::vector<int>& getSockets();
		void		handleRedirectRequest(HTTrequestMSG& request, ServerInfo& server);
		void		setRedirectResponse(const std::string &location, const conf_File_Info &config);
		void		setContentLength(size_t length);
		size_t		getContentLength() const;
		void		addConfig(int port, const conf_File_Info& config);
		conf_File_Info& getConfig(int port);
		std::string	getCompletePath(const std::string& path);
		void		setCompletePath(const std::string& path);
		std::string	getCompletePath2() const;
		std::string	getRootOriginalDirectory() const;
		void		setRootOriginalDirectory(const std::string& dir);	
		void		handleError(const std::string& errorMessage);

		void cleanup1();
		void cleanup2();
};

std::string extractFileNameFromURL(const std::string& url);

std::string	methodToString(HTTrequestMSG::Method method);
void		printLog(const std::string& method, const std::string& path, const std::string& version, const std::string& httpResponse, ServerInfo& server);
bool		is_directory(const std::string &path);
void		setupDirectory(ServerInfo& server, const conf_File_Info& config);
int			remove_file(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);
int			remove_directory(const char *path); //to use in main() to remove the temp directory
void		setupServer(ServerInfo& server, const conf_File_Info& config);
std::string	readRequest(int sockfd);
std::string	readRequest(int sockfd, ServerInfo& server);
void		processRequest(const std::string& request, ServerInfo& server);
bool		processRulesRequest(HTTrequestMSG& requestMsg, ServerInfo& server);
void		handleRequest(HTTrequestMSG& request, ServerInfo& server, conf_File_Info &serverConfig);
std::string readFileContent(const std::string& filePath);
std::vector<std::string> readDirectoryContent(const std::string& directoryPath);
bool		ends_with(const std::string& value, const std::string& ending);
std::string getContentType(const std::string& filePath);
void		setupUploadDirectory(const std::string& serverRoot, const std::string& uploadDirectory);
std::string	removeFirstDirectory(const std::string& fullPath);
std::string	removeTrailingSlash(const std::string& path);
bool		fileExistsInDirectory(const std::string& directory, const std::string& filename);
void		processErrorPage(std::string second, int errorCode, const std::string& rootDirectory);
void		handleError2(int errorCode, ServerInfo& server, conf_File_Info& serverConfig, const HTTrequestMSG& requestMsg);

std::string	getNewPath(const std::string& root, const std::string& path);
std::vector<std::string> tokenize(const std::string& str, char delimiter);
bool		isMethodAllowed(const std::set<std::string>& allowedMethods, const std::string& requestMethod);
std::string	getDirectoryPath(const std::string& fullPath);
std::string	ifFileRmoveFile(std::string path);
std::string	removeLastSlash(const std::string& fullPath);
bool		handleDirectoryListing(conf_File_Info& serverConfig, HTTrequestMSG& requestMsg);
bool		fileExists(const std::string& filePath);
bool		isDirectory(const std::string& path);

void createIndexFile(conf_File_Info &serverConfig, const std::string& rootDirectory);
void createHtmlFiles(const std::string& rootDirectory);

void		setupRunServer(std::vector<ServerInfo*>& servers, fd_set& read_fds, fd_set& write_fds, int& max_fd);
void		runServer(std::vector<ServerInfo*>& servers, fd_set read_fds, fd_set write_fds, int max_fd);

#endif