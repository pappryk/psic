#ifndef Client_hpp
#define Client_hpp

#include <netinet/in.h>
#include <string>
#include <vector>

class Client
{
public:

	sockaddr_in clientAddr; 
	std::string httpRequest = "";
	std::string targetServer;
	std::vector<char> httpResponse;

	unsigned offset = 0;
};


#endif // Client_hpp