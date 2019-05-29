#include <sstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include "HttpParser.hpp"

std::vector <std::string> split(std::string str, std::string token) {
    std::vector <std::string> result;
    while (str.size()) {
        int index = str.find(token);
        if (index != std::string::npos) {
            result.push_back(str.substr(0, index));
            str = str.substr(index + token.size());
            if (str.size() == 0)result.push_back(str);
        } else {
            result.push_back(str);
            str = "";
        }
    }
    return result;
}

int main()
{
    // std::string request("CONNECT example.host.com:22 HTTP/1.1
    //     Proxy-Authorization: Basic encoded-credentials");

    std::string request("CONNECT example.host.com:22 HTTP/1.1\nProxy-Authorization: Basic encoded-credentials");
    std::string method = HttpParser::getHttpMethod(request);
    std::string targetServer = HttpParser::getTargetServer(request);

    std::cout << method << std::endl;
    std::cout << targetServer << std::endl;

}