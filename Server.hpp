#ifndef Server_hpp
#define Server_hpp

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <fcntl.h>
#include <poll.h>

#include <vector>
#include <string>
#include <iostream>
#include <sys/socket.h>
#include "Client.hpp"

class Server
{
public:

    void listenConnections();
    std::vector<Client> m_clients;
    void configureServer(int port);
    void setServerSocket();
    void setSocketToNonBlocking(int socketToSet);
    void exitOnError(int operationStatus, const std::string& errorMessage);
    void setServerPollFD();
    void handleConnections();
    void acceptNewConnection();
private:
    int m_serverSocket;

	sockaddr_in m_serverAddr;
    std::vector<pollfd> m_pollfds;



};

#endif