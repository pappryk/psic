
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
    int operationStatus = bind(m_serverSocket, (sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
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
}

void Server::listen() 
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

		for (unsigned i = 0; i < m_pollfds.size(); ++i) {

			if ((m_pollfds.at(i).fd == m_serverSocket) && (m_pollfds.at(i).revents & POLLIN)) 
			{
				acceptNewConnection();
				break;
			}

			if ((m_pollfds.at(i).fd != m_serverSocket) && (m_pollfds.at(i).revents & POLLIN)) 
			{
				char buffer[1000000];
                int receive_status = recv(polls[i].fd, (void*) buffer, sizeof(buffer), 0);
                m_clients[i].httpRequest.append(buffer);

				if(shouldBreak) {
					break;
				}
			}

			if ((m_pollfds.at(i).fd != m_serverSocket) && (m_pollfds.at(i).revents & POLLOUT))
			{
				writeData(i);
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

	Client newClient;
	newClient.clientAddr = addr;

	m_clients.push_back(newClient);
	m_pollfds.push_back({clientSocket, POLLIN | POLLOUT, 0});

	std::cout << " + New connection accepted + " << std::endl;
}