#ifndef CONF_INFO_HPP
#define CONF_INFO_HPP

#include "librarie.hpp"

struct conf_File_Info;

typedef std::map<std::string, conf_File_Info> Locations;

struct ForwardingURL
{
    int httpStatusCode;
    std::string destinationURL;
};

struct conf_File_Info
{
    int portListen;
    std::string ServerName;
    std::string defaultFile;
    std::string RootDirectory;
    std::string Path_CGI;
    std::string host;
    bool directoryListingEnabled;
    std::map<int, std::string> errorMap;
    ForwardingURL redirectURL;
    std::set<std::string> allowedMethods;
    bool autoindexPresent;
    int maxRequestSize;
    std::string fileUploadDirectory;
    Locations LocationsMap;
    std::string cgiExtension;
    std::string uploadToDirectory;
    std::string tryFile;
};

#endif