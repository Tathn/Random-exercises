#ifndef SOCKETLIB_H
#define SOCKETLIB_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

struct addrinfo getConnectionInfo()
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    
    // don't care IPv4 or IPv6, vals: AF_INET, AF_INET6, AF_UNSPEC
    hints.ai_family = AF_UNSPEC;

    // TCP Transmission Control Protocol
    // UDP User Datagram Protocol
    // TCP stream sockets, available values: SOCK_STREAM, SOCK_DGRAM
    hints.ai_socktype = SOCK_STREAM;

    // hints.ai_flags = AI_PASSIVE;
    return hints;
}

int createSocketFileDescriptor(struct addrinfo* serverInfo)
{
    int sockfd = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
    int true = 1;
    //allows socket to be reused faster after close()
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int));
    if(sockfd == -1)
    {
        fprintf(stderr, "Could not estabilish socket connection\n");
    }
    return sockfd;
}

int bindSocket(int socketFileDescriptor, struct addrinfo* serverInfo)
{
    int bindResult = bind(socketFileDescriptor, serverInfo->ai_addr, serverInfo->ai_addrlen);
    int result = bindResult != -1;
    if(!result)
    {
        fprintf(stderr, "Could not bind socket\n");
    }
    return result;
}

void connectToSocket(int socketFileDescriptor, struct addrinfo* serverInfo)
{
    int result = connect(socketFileDescriptor, serverInfo->ai_addr, serverInfo->ai_addrlen);
    if(result == -1)
    {
        fprintf(stderr, "Could not connect to socket\n");
        exit(1);
    }
}

void startListening(int socketFileDescriptor, int allowedConnectionsInQueue)
{
    int result = listen(socketFileDescriptor, allowedConnectionsInQueue);
    if(result == -1)
    {
        fprintf(stderr, "Could not start listening on socket\n");
        exit(1);
    }
}

int acceptIncomingConnection(int listeningSocketFD,
                             struct sockaddr* socketStorage,
                             socklen_t* addressLength)
{
    int clientFD = accept(listeningSocketFD, socketStorage, addressLength);
    if(clientFD == -1)
    {
        fprintf(stderr, "Could not accept socket connection from client\n");
        exit(1);
    }
    return clientFD;
}

void sigchldHandler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

void setupChildProcessSignalHandler()
{
    struct sigaction sa;
    sa.sa_handler = sigchldHandler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        fprintf(stderr, "Could not setup signal child process handler\n");
        exit(1);
    }
}

void* getInternetAddresBasedOnIPversion(struct sockaddr *socketAddress)
{
    if (socketAddress->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)socketAddress)->sin_addr);
    }

    return &(((struct sockaddr_in6*)socketAddress)->sin6_addr);
}

void setToNonBlocking(int socketFD)
{
    fcntl(socketFD, F_SETFL, O_NONBLOCK);
}

int bindTo(char* ipAddress, char* port)
{
    struct addrinfo hints = getConnectionInfo();
    if(ipAddress == NULL)
    {
        // vals: AI_CANONNAME, AI_PASSIVE...
        // fill in my IP for me
        hints.ai_flags = AI_PASSIVE;
    }

    struct addrinfo *destinationInfo;
    int result = getaddrinfo(ipAddress, port, &hints, &destinationInfo);
    if(result != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(result));
        exit(1);
    }

    // destinationInfo now points to a linked list of 1 or more struct addrinfos
    // as some services can have both IPv4 and IPv6 addresses
    for(struct addrinfo* entry = destinationInfo; entry != NULL; entry = entry->ai_next)
    {
        int sockfd = createSocketFileDescriptor(entry);
        if(sockfd != -1)
        {
            if(bindSocket(sockfd, entry))
            {
                freeaddrinfo(destinationInfo);
                return sockfd;
            }
            close(sockfd);
        }
    }
    freeaddrinfo(destinationInfo);
    return -1;
}

int connectTo(char* ipAddress, char* port)
{
    struct addrinfo hints = getConnectionInfo();
    if(ipAddress == NULL)
    {
        // vals: AI_CANONNAME, AI_PASSIVE...
        // fill in my IP for me
        hints.ai_flags = AI_PASSIVE;
    }

    struct addrinfo *destinationInfo;
    int result = getaddrinfo(ipAddress, port, &hints, &destinationInfo);
    if(result != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(result));
        exit(1);
    }

    // destinationInfo now points to a linked list of 1 or more struct addrinfos
    // as some services can have both IPv4 and IPv6 addresses
    for(struct addrinfo* entry = destinationInfo; entry != NULL; entry = entry->ai_next)
    {
        int sockfd = createSocketFileDescriptor(entry);
        if(sockfd != -1)
        {
            if(connect(sockfd, entry->ai_addr, entry->ai_addrlen) != -1)
            {
                freeaddrinfo(destinationInfo);
                return sockfd;
            }
            fprintf(stderr, "Socket connection failed\n");
            close(sockfd);
        }
    }
    freeaddrinfo(destinationInfo);
    return -1;
}
#endif
