#include <netdb.h>
#include "Server.hpp"
#include "HttpParser.hpp"
//
//int getContentLength(std::string response) {
//    std::size_t indexOfLineStartsWithContentLength = response.find("Content-Length:");
//
//    if (indexOfLineStartsWithContentLength != std::string::npos) {
//
//        std::string lineStartsWithContentLength = response.substr(indexOfLineStartsWithContentLength);
//        std::istringstream ss(lineStartsWithContentLength);
//        std::string foundOneLineWithContentLength;
//        std::getline(ss, foundOneLineWithContentLength);
//
//        std::size_t ColonAfterContentLength = foundOneLineWithContentLength.find(':');
//        if (ColonAfterContentLength != std::string::npos) {
//            std::string contentLengthString = foundOneLineWithContentLength.substr(ColonAfterContentLength + 2);
//            std::stringstream contentLengthStringStream(contentLengthString);
//            int contentLength = 0;
//            contentLengthStringStream >> contentLength;
//            return contentLength;
//        } else
//            return 0;
//    }
//    return 0;
//
//}

//int getChunkLenght(std::string response) {
//    std::size_t startOfChunk = response.find("\r\n\r\n");
//    std::string lineWithChunkLength;
//    if (startOfChunk != std::string::npos) {
//        std::string chunk = response.substr(startOfChunk + 4);
//        std::istringstream ss(chunk);
//        std::getline(ss, lineWithChunkLength);
//    } else {
//        std::istringstream ss(response);
//        std::getline(ss, lineWithChunkLength);
//        std::cout << lineWithChunkLength << std::endl;
//    }
//
//    unsigned int x;
//    std::stringstream ss2;
//    ss2 << std::hex << lineWithChunkLength;
//    ss2 >> x;
//    std::cout << x << std::endl;
//    return x;
//}

//std::vector<std::string> split(std::string str, std::string token) {
//    std::vector<std::string> result;
//    while (str.size()) {
//        int index = str.find(token);
//        if (index != std::string::npos) {
//            result.push_back(str.substr(0, index));
//            str = str.substr(index + token.size());
//            if (str.size() == 0)result.push_back(str);
//        } else {
//            result.push_back(str);
//            str = "";
//        }
//    }
//    return result;
//}

int nthOccurrence(const std::string &str, const std::string &findMe, int nth) {
    size_t pos = 0;
    int cnt = 0;

    while (cnt != nth) {
        pos += 1;
        pos = str.find(findMe, pos);
        if (pos == std::string::npos)
            return -1;
        cnt++;
    }
    return pos;
}

void Server::configureServer(int port) {
    memset(&m_serverAddr, 0, sizeof(m_serverAddr));
    m_serverAddr.sin_family = AF_INET;
    m_serverAddr.sin_port = htons(port);
    m_serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    setServerSocket();
    setSocketToNonBlocking(m_serverSocket);
    setServerPollFD();

}

void Server::setServerSocket() {
    m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    exitOnError(m_serverSocket, "Unable to create socket");
    int one = 1;
    int operationStatus = setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    exitOnError(operationStatus, "Unable to set socket options");
    operationStatus = bind(m_serverSocket, (sockaddr *) &m_serverAddr, sizeof(m_serverAddr));
    exitOnError(operationStatus, "Unable to bind");

}

void Server::setSocketToNonBlocking(int socketToSet) {
    int flags = fcntl(socketToSet, F_GETFL);
    int operationStatus = fcntl(socketToSet, F_SETFL, O_NONBLOCK | flags);
    exitOnError(operationStatus, "fcntl() function failed");
}

void Server::exitOnError(int operationStatus, const std::string &errorMessage) {
    if (operationStatus < 0) {
        perror(errorMessage.c_str());
        exit(EXIT_FAILURE);
    }
}

void Server::setServerPollFD() {
    m_clients.push_back(Client());
    m_clients[0].proxyServer = 0;
    m_clients[0].browserProxy= 1;

    m_pollfds.push_back({m_serverSocket, POLLIN, 0});
    m_pollfds.push_back({m_serverSocket, POLLIN, 0});
}

void Server::listenConnections() {
    int operationStatus = listen(m_serverSocket, 1);
    exitOnError(operationStatus, "Unable to listen");
}

void Server::handleConnections() {
    while (1) {

        int operationStatus = poll(m_pollfds.data(), m_pollfds.size(), -1);
        exitOnError(operationStatus, "poll() function failed");

        for (unsigned i = 0; i < m_clients.size(); i++) {


            if ((m_pollfds[m_clients[i].proxyServer].fd == m_serverSocket) && (m_pollfds[m_clients[i].proxyServer].revents & POLLIN)) {
                acceptNewConnection();
                continue;
            }

            if ((m_pollfds[m_clients[i].browserProxy].fd != m_serverSocket) && (m_pollfds[m_clients[i].browserProxy].revents & POLLIN)) {

                char buffer[10000];
                memset(buffer, 0, 10000);
                int receive_status = recv(m_pollfds[m_clients[i].browserProxy].fd, (void *) buffer, sizeof(buffer), 0);

                if(receive_status > 0) {
                    m_clients[i].httpRequest = std::string(buffer);
                    std::cout << " DOSTAJE REQUEST: " << i << "  " << std::endl << m_clients[i].httpRequest
                              << std::endl << "KONIEC" << std::endl;

                    std::string method = HttpParser::getHttpMethod(m_clients[i].httpRequest);
                    std::string targetServer = HttpParser::getTargetServer(m_clients[i].httpRequest);


                    if (method == "GET" && !m_clients[i].connected) {
                        if (targetServer[0] != '/') {
                            m_clients[i].connected = true;

                            int slashBeforeResourcePosition = nthOccurrence(targetServer, "/", 3);
                            int slashBeforeServerPosition = nthOccurrence(targetServer, "/", 2);

                            std::string resource = targetServer.substr(slashBeforeResourcePosition);
                            std::string serverName = targetServer.substr(slashBeforeServerPosition + 1,
                                                                         slashBeforeResourcePosition -
                                                                         slashBeforeServerPosition - 1);

                            addrinfo ahints;
                            addrinfo *paRes;

                            ahints.ai_family = AF_UNSPEC;
                            ahints.ai_socktype = SOCK_STREAM;
                            if (getaddrinfo(serverName.c_str(), "80", &ahints, &paRes) != 0)
                                std::cout << "BLAD" << std::endl;

                            m_clients[i].targetServer = serverName;

                            int iSockfd;
                            if ((iSockfd = socket(paRes->ai_family, paRes->ai_socktype, paRes->ai_protocol)) < 0) {
                                fprintf(stderr, " Error in creating socket to server ! \n");
                                exit(1);
                            } else { ;
                            }

                            if (connect(iSockfd, paRes->ai_addr, paRes->ai_addrlen) < 0) {
                                fprintf(stderr, " Error in connecting to server ! \n");
                                exit(1);
                            } else { ;
                            }

                            m_clients[i].httpRequest.replace(m_clients[i].httpRequest.find(targetServer),
                                                             slashBeforeResourcePosition, std::string(""));

                            pollfd new_poll = {iSockfd, POLLIN | POLLOUT, 0};
                            m_pollfds[m_clients[i].proxyServer] = new_poll;
                        }
                    }
                }
            }

            if ((m_pollfds[m_clients[i].proxyServer].fd != m_serverSocket) &&
                (m_pollfds[m_clients[i].proxyServer].revents & POLLOUT)) {
                int request_size = m_clients[i].httpRequest.length();
                int temp_response_size = request_size;

                char *temp_buffer_to_send = (char *) m_clients[i].httpRequest.c_str();

                if(request_size > 0) {
                    send(m_pollfds[m_clients[i].proxyServer].fd, temp_buffer_to_send,
                         temp_response_size, 0);
                    m_clients[i].httpRequest = std::string("");
                }
            }

            if ((m_pollfds[m_clients[i].proxyServer].fd != m_serverSocket) &&
                (m_pollfds[m_clients[i].proxyServer].revents & POLLIN)) {
                if(m_clients[i].closed)
                    continue;
                char response[10000];
                memset(response, 0, sizeof(response));

                    m_clients[i].already_received_from_server = recv(m_pollfds[m_clients[i].proxyServer].fd, response,
                                                                     sizeof(response), 0);
                    //m_clients[i].httpResponse.append(std::string(response));

                    if(m_clients[i].already_received_from_server == 0) {
                        close(m_pollfds[m_clients[i].browserProxy].fd);
                        std::cout << "CLOSED CONNECTION: " << m_pollfds[m_clients[i].browserProxy].fd << std::endl;
                        m_clients[i].closed = true;
                    }
                    else
                    send(m_pollfds[m_clients[i].browserProxy].fd,
                                                                (char *) response,
                     m_clients[i].already_received_from_server , 0);

                    //std::string response_from_server(response);

                   // std::cout << "ODBIERAM OD SERWERA: " << i <<" \n " << response_from_server << std::endl;
            }


            if ((m_pollfds[m_clients[i].browserProxy].fd != m_serverSocket) && (m_pollfds[m_clients[i].browserProxy].revents & POLLOUT)
                ) {

//                if(m_clients[i].httpResponse.length() > 0){
//                    ssize_t sent =  send(m_pollfds[m_clients[i].browserProxy].fd,
//                                                                (char *) m_clients[i].httpResponse.c_str(),
//                                                                m_clients[i].httpResponse.length() , 0);
//                    std::cout << "WYSYLAM DO BROWSERA: " << i <<" \n " << m_clients[i].httpResponse.substr(0, sent) << std::endl;
//                    if(sent > 0)
//                        m_clients[i].httpResponse = m_clients[i].httpResponse.substr(0, sent);
//                    std::cout << "ZOSTAJE DO WYSLANIA: " << i <<" \n " << m_clients[i].httpResponse << std::endl;
//                }
            }
        }
    }


}

void Server::acceptNewConnection() {
    sockaddr_in addr;
    uint len = sizeof(addr);

    int clientSocket = accept(m_serverSocket, (sockaddr *) &addr, &len);
    exitOnError(clientSocket, "accept() function failed");

    setSocketToNonBlocking(clientSocket);

    Client *newClient = new Client();
    newClient->clientAddr = addr;

    m_clients.push_back(*newClient);

    m_pollfds.push_back({clientSocket, POLLIN | POLLOUT, 0});
    m_pollfds.push_back({clientSocket, POLLIN | POLLOUT, 0});

    m_clients[m_clients.size() - 1].browserProxy = m_pollfds.size() - 2;
    m_clients[m_clients.size() - 1].proxyServer =  m_pollfds.size() - 1;
    std::cout << " + New connection accepted + " << clientSocket << std::endl;
}