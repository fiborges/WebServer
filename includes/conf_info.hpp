#ifndef CONF_INFO_HPP
#define CONF_INFO_HPP

#include "librarie.hpp"

struct Directives;
typedef std::map<std::string, Directives> Locations;

struct Redirect
{
    int code;
    std::string url;
};

struct Directives
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