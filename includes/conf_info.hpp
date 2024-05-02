#ifndef CONF_INFO_HPP
#define CONF_INFO_HPP

#include "librarie.hpp"

//allows to declare pointers or references to this struct before it's fully defined
struct conf_File_Info;
//Defines an alias Locations for a map where the keys are strings (representing paths) and the values are conf_File_Info structs. 
typedef std::map<std::string, conf_File_Info> Locations;

//struct Redirect that represents a redirect configuration
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
    bool directoryListingEnabled;
    std::map<int, std::string> errorMap;
    ForwardingURL redirectURL;
    std::set<std::string> allowedMethods;
    int maxRequestSize;
    std::string fileUploadDirectory;
    Locations LocationsMap;// dentro daqui temos locatoon path e root e o cgi
};

#endif