#ifndef Client_hpp
#define Client_hpp

#include <netinet/in.h>
#include <string>
#include <vector>
#include <poll.h>

class Client
{
public:

    int headers_from_server_size;
    int already_sent_to_server = 0;
    int already_received_from_server = 0;
    int already_sent_to_browser;
    int response_from_server_content_length = 0;
    sockaddr_in clientAddr;
    std::string httpRequest = "";
    std::string targetServer = std::string("");
    std::string httpResponse = "";
    int socketBrowserProxy;
    int socketProxyServer;
    bool isAllReceivedFromBrowser = false;
    bool isAllReceivedFromServer = false;
    bool isAllSentToServer = false;
    bool isAllSentToBrowser = false;
    int  browserProxy;
    int proxyServer;

    unsigned offset = 0;
};


#endif // Client_hpp