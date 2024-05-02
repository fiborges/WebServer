#ifndef CONF_INFO_HPP
#define CONF_INFO_HPP

#include "librarie.hpp"

struct conf_File_Info;
typedef std::map<std::string, conf_File_Info> Locations;

struct Redirect
{
    int code;
    std::string url;
};

struct conf_File_Info
{
    int listen;
    std::string server_name;
    std::string index;
    std::string root;
    std::string cgi;
    bool autoindex;
    std::map<int, std::string> error_page;
    Redirect redirect;
    std::set<std::string> limit_except;
    int client_max_body_size;
    std::string upload_dir;
    Locations locations;// dentro daqui temos locatoon path e root e o cgi
};

#endif