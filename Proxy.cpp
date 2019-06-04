#include <sstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include "HttpParser.hpp"
#include "Server.hpp"

int main()
{
    // std::string request("CONNECT example.host.com:22 HTTP/1.1
    //     Proxy-Authorization: Basic encoded-credentials");

    std::string request("CONNECT example.host.com:22 HTTP/1.1\nProxy-Authorization: Basic encoded-credentials");
    std::string method = HttpParser::getHttpMethod(request);
    std::string targetServer = HttpParser::getTargetServer(request);

    Server server;
    server.configureServer(62333);
    server.listenConnections();
    server.handleConnections();

    std::cout << method << std::endl;
    std::cout << targetServer << std::endl;

}