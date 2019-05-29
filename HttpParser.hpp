#include <sstream>
#include <iostream>


class HttpParser
{
public:
    static std::string getHttpMethod(std::string request)
    {
        std::istringstream ss(request);
        std::string line;
        std::getline(ss, line);
        std::string method(split(line, " ")[0]);

        return method;
    }
    static std::string getTargetServer(std::string request)
    {
        std::istringstream ss(request);
        std::string line;
        std::getline(ss, line);
        std::string url(split(line, " ")[1]);

        return url;
    }
private:
    static std::vector <std::string> split(std::string str, std::string token) {
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
};