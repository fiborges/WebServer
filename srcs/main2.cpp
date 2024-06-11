#include "../includes/librarie.hpp"
#include "../includes/parser.hpp"

void printHeader(const std::string& header) {
    std::cout << BOLD << BLUE << "\n=== " << header << " ===" << RESET << std::endl;
}

void printSubHeader(const std::string& subHeader) {
    std::cout << BOLD << CYAN << "\n--- " << subHeader << " ---" << RESET << std::endl;
}

void printConfig(const conf_File_Info& config, const std::string& location = "") {
    if (!location.empty()) {
        std::cout << BOLD << CYAN << "\n Location: " << location << RESET << std::endl;
    }

    std::cout << BOLD << "int portListen: " << RESET << config.portListen << std::endl;
    std::cout << BOLD << "std::string ServerName: " << RESET << config.ServerName << std::endl;
    std::cout << BOLD << "std::string defaultFile: " << RESET << config.defaultFile << std::endl;
    std::cout << BOLD << "std::string RootDirectory: " << RESET << config.RootDirectory << std::endl;
    std::cout << BOLD << "std::string Path_CGI: " << RESET << config.Path_CGI << std::endl;
    std::cout << BOLD << "std::string cgiExtension: " << RESET << config.cgiExtension << std::endl; // Adicionado
    std::cout << BOLD << "bool directoryListingEnabled: " << RESET << (config.directoryListingEnabled ? "true" : "false") << std::endl;

    std::cout << BOLD << YELLOW << "std::map<int, std::string> errorMap: " << RESET << std::endl;
    for (std::map<int, std::string>::const_iterator it = config.errorMap.begin(); it != config.errorMap.end(); ++it) {
        std::cout << "  " << it->first << " -> " << it->second << std::endl;
    }

    std::cout << BOLD << MAGENTA << "ForwardingURL redirectURL: " << RESET << std::endl;
    std::cout << "  " << BOLD << "httpStatusCode: " << RESET << config.redirectURL.httpStatusCode << std::endl;
    std::cout << "  " << BOLD << "destinationURL: " << RESET << config.redirectURL.destinationURL << std::endl;

    std::cout << BOLD << "std::set<std::string> allowedMethods: " << RESET << std::endl;
    for (std::set<std::string>::const_iterator it = config.allowedMethods.begin(); it != config.allowedMethods.end(); ++it) {
        std::cout << "  " << *it << std::endl;
    }

    std::cout << BOLD << "int maxRequestSize: " << RESET << config.maxRequestSize << std::endl;
    std::cout << BOLD << "std::string fileUploadDirectory: " << RESET << config.fileUploadDirectory << std::endl;
    std::cout << BOLD << "std::string uploadToDirectory: " << RESET << config.uploadToDirectory << std::endl; // Adicionado

    std::cout << BOLD << "\n Locations LocationsMap: " << RESET << std::endl;
    for (std::map<std::string, conf_File_Info>::const_iterator it = config.LocationsMap.begin(); it != config.LocationsMap.end(); ++it) {
        printConfig(it->second, it->first);
    }
}

void testMatching(const ParserConfig& parserConfig, const std::string& path) {
    try {
        printHeader("Testing Path: " + path);
        std::string matchedPath = parserConfig.matchPath(path);
        ParserConfig context = parserConfig.extractContext(matchedPath);
        std::cout << GREEN << "Matched Path: " << matchedPath << RESET << std::endl;
        printConfig(*context.getServerConfigurations(), matchedPath);
        std::cout << BOLD << "cgiExtension: " << RESET << context.fetchCGIExtension() << std::endl; // Adicionado
        std::cout << BOLD << "uploadToDirectory: " << RESET << context.fetchUploadToDirectory() << std::endl; // Adicionado
    } catch (const std::exception& e) {
        std::cout << RED << "Error: " << e.what() << RESET << std::endl;
    }
}

int main() {
    try {
        printHeader("Initializing Parser");
        ParserClass* parser = new ParserClass("4server2.conf");
        const ConfiguredServers& servers = parser->fetchSpecifications();
        
        for (size_t i = 0; i < servers.size(); ++i) {
            std::stringstream ss;
            ss << (i + 1);
            std::string numberAsString = ss.str();

            printHeader("Server " + numberAsString + " Configuration");
            printConfig(*servers[i].getServerConfigurations());
        }
        
        // Testing path matching
        if (!servers.empty()) {
            testMatching(servers[0], "/test/path");
            testMatching(servers[0], "/cgi-bin/script");
            testMatching(servers[0], "/upload/file");
            testMatching(servers[0], "/redirect");
            testMatching(servers[0], "/non-existent");
        }

        delete parser; // Liberar a memória alocada
    } catch (const std::exception& e) {
        std::cerr << RED << "Exception: " << e.what() << RESET << std::endl;
    }
    return 0;
}