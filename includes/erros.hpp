#ifndef ERROS_HPP
#define ERROS_HPP

#include "librarie.hpp"
#include "RequestParser.hpp"
#include "parser.hpp"
#include "parserConfig.hpp"
#include "get.hpp"

class ServerErrorHandler
{
public:
	ServerErrorHandler();
	std::string generateErrorPage(int errorCode);
	std::string getErrorMessage(int errorCode);

	std::string BgenerateErrorPage_2(std::string errorCodeStr);
	std::string getErrorMessage_2(std::string errorCode);

	std::string generateIndex(const std::string& name);
	std::string generateDelete();
	std::string generateGet();
	std::string generatePost();
	std::string generateUpload();

private:
	std::map<int, std::string> errorMessages;
	std::string AgenerateErrorPage_2(std::string errorCodeStr);
	std::string AgenerateErrorPage_3(std::string errorCodeStr);
	std::string generateErrorPage(int errorCode, const std::string& errorMessage);

	void initializeErrorMessages();
};

#endif
