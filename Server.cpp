#include <netdb.h>
#include "Server.hpp"
#include "HttpParser.hpp"

int nthOccurrence(const std::string& str, const std::string& findMe, int nth)
{
	size_t  pos = 0;
	int     cnt = 0;

	while( cnt != nth )
	{
		pos+=1;
		pos = str.find(findMe, pos);
		if ( pos == std::string::npos )
			return -1;
		cnt++;
	}
	return pos;
}

void Server::configureServer(int port) 
{
	memset(&m_serverAddr, 0, sizeof(m_serverAddr));
	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_port = htons(port);
	m_serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    setServerSocket();
    setSocketToNonBlocking(m_serverSocket);
    setServerPollFD();

}

void Server::setServerSocket() 
{
	m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	exitOnError(m_serverSocket, "Unable to create socket");
    int one = 1;
	int operationStatus = setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
	exitOnError(operationStatus, "Unable to set socket options");
    operationStatus = bind(m_serverSocket, (sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
	exitOnError(operationStatus, "Unable to bind");

}

void Server::setSocketToNonBlocking(int socketToSet)
{
	int flags = fcntl(socketToSet, F_GETFL);
	int operationStatus = fcntl(socketToSet, F_SETFL, O_NONBLOCK | flags);
	exitOnError(operationStatus, "fcntl() function failed");
}

void Server::exitOnError(int operationStatus, const std::string& errorMessage)
{
	if (operationStatus < 0) {
		perror(errorMessage.c_str());
		exit(EXIT_FAILURE);
	}
}

void Server::setServerPollFD()
{
	m_pollfds.push_back({m_serverSocket, POLLIN, 0});
	m_clients.push_back(Client());
}

void Server::listenConnections()
{
	int operationStatus = listen(m_serverSocket, 1);
	exitOnError(operationStatus, "Unable to listen");
}

void Server::handleConnections() 
{
	while(1) 
	{
		int operationStatus = poll(m_pollfds.data(), m_pollfds.size(), -1);
	    exitOnError(operationStatus, "poll() function failed");

        if (operationStatus == 0)
            continue;

		for (unsigned i = 0; i < m_pollfds.size(); i++) {

			if ((m_pollfds.at(i).fd == m_serverSocket) && (m_pollfds.at(i).revents & POLLIN)) 
			{
				acceptNewConnection();
				break;
			}

			if ((m_pollfds.at(i).fd != m_serverSocket) && (m_pollfds.at(i).revents & POLLIN)) 
			{

				std::cout << m_pollfds.size() << "SIZE POLL" << std::endl;
				std::cout << m_clients.size() << "SIZE CLIENT" << std::endl;

				char buffer[1000000];
                int receive_status = recv(m_pollfds[i].fd, (void*) buffer, sizeof(buffer), 0);
                std::string str(buffer);

				std::string method = HttpParser::getHttpMethod(str);
				std::string targetServer = HttpParser::getTargetServer(str);
				if(method == "CONNECT"){
					m_clients[i].targetServer = targetServer;
					std::string responseOK("HTTP/1.1 200 OK");
					send(m_pollfds[i].fd, responseOK.c_str(), responseOK.size(), 0);
				} else if(method == "GET") {
					if(targetServer[0] != '/'){
						int n = nthOccurrence(targetServer, "/", 3);
						std::string subst = targetServer.substr(n);
						std::cout<< "EEEEEEEEEEEEEEEEEEEEEEEE           " << subst << std::endl;
						std::string youtube("youtube.pl");

						addrinfo ahints;
						addrinfo *paRes, *rp;

						ahints.ai_family = AF_UNSPEC;
						ahints.ai_socktype = SOCK_STREAM;
						if (getaddrinfo(youtube.c_str(), "80", &ahints, &paRes) != 0)
							std::cout << "BLAD" << std::endl;


						for (rp = paRes; rp != NULL; rp = rp->ai_next) {
							std::cout<< "PARES" << rp->ai_addr->sa_data << std::endl;
						}


					}
				}

				m_clients[i].httpRequest.append(buffer);
                std::cout<< buffer <<std::endl;

				;
			}

			if ((m_pollfds.at(i).fd != m_serverSocket) && (m_pollfds.at(i).revents & POLLOUT))
			{
				;
			}
		}
	}
}

void Server::acceptNewConnection()
{
	sockaddr_in addr;
	uint len = sizeof(addr);

	int clientSocket = accept(m_serverSocket, (sockaddr*)&addr, &len);
	exitOnError(clientSocket, "accept() function failed");
    
	setSocketToNonBlocking(clientSocket);

	Client *newClient = new Client();
	newClient->clientAddr = addr;

	m_clients.push_back(*newClient);
	m_pollfds.push_back({clientSocket, POLLIN | POLLOUT, 0});

	std::cout << " + New connection accepted + " << std::endl;
}