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
        printSubHeader("Location: " + location);
    }
    std::cout << BOLD << "int portListen: " << RESET << config.portListen << std::endl;
    std::cout << BOLD << "std::string ServerName: " << RESET << config.ServerName << std::endl;
    std::cout << BOLD << "std::string defaultFile: " << RESET << config.defaultFile << std::endl;
    std::cout << BOLD << "std::string RootDirectory: " << RESET << config.RootDirectory << std::endl;
    std::cout << BOLD << "std::string Path_CGI: " << RESET << config.Path_CGI << std::endl;
    std::cout << BOLD << "bool directoryListingEnabled: " << RESET << (config.directoryListingEnabled ? "true" : "false") << std::endl;
    std::cout << BOLD << "std::map<int, std::string> errorMap: " << RESET << std::endl;
    for (std::map<int, std::string>::const_iterator it = config.errorMap.begin(); it != config.errorMap.end(); ++it) {
        std::cout << "  " << it->first << " -> " << it->second << std::endl;
    }
    std::cout << BOLD << "ForwardingURL redirectURL: " << RESET << std::endl;
    std::cout << "  httpStatusCode: " << config.redirectURL.httpStatusCode << std::endl;
    std::cout << "  destinationURL: " << config.redirectURL.destinationURL << std::endl;
    std::cout << BOLD << "std::set<std::string> allowedMethods: " << RESET;
    for (std::set<std::string>::const_iterator it = config.allowedMethods.begin(); it != config.allowedMethods.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    std::cout << BOLD << "int maxRequestSize: " << RESET << config.maxRequestSize << std::endl;
    std::cout << BOLD << "std::string fileUploadDirectory: " << RESET << config.fileUploadDirectory << std::endl;
    std::cout << BOLD << "Locations LocationsMap: " << RESET << std::endl;
    for (Locations::const_iterator it = config.LocationsMap.begin(); it != config.LocationsMap.end(); ++it) {
        printConfig(it->second, it->first);
    }
    std::cout << BOLD << "Exact Locations LocationsMap: " << RESET << std::endl;
    for (Locations::const_iterator it = config.ExactLocationsMap.begin(); it != config.ExactLocationsMap.end(); ++it) {
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
    } catch (const std::exception& e) {
        std::cout << RED << "Error: " << e.what() << RESET << std::endl;
    }
}

int main() {
    try {
        printHeader("Initializing Parser");
        ParserClass* parser = new ParserClass("server2.conf");
        const ConfiguredServers& servers = parser->fetchSpecifications();
        
        for (size_t i = 0; i < servers.size(); ++i) {
            printHeader("Server " + std::to_string(i + 1) + " Configuration");
            printConfig(*servers[i].getServerConfigurations());
        }
        
        // Testing path matching
        if (!servers.empty()) {
            testMatching(servers[0], "/test/path");
            testMatching(servers[0], "/cgi-bin/script");
            testMatching(servers[0], "/upload/file");
            testMatching(servers[0], "/redirect");
            testMatching(servers[0], "/non-existent");
            testMatching(servers[0], "/exact-match");
        }

        delete parser; // Liberar a memória alocada
    } catch (const std::exception& e) {
        std::cerr << RED << "Exception: " << e.what() << RESET << std::endl;
    }
    return 0;
}