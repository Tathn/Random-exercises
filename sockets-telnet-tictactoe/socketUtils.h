#ifndef SOCKETUTILS_H
#define SOCKETUTILS_H

#include "socketLib.h"

int getNewClientConnection(int serverSocketFD)
{
    struct sockaddr_storage clientsAddresses;
    socklen_t addressSize = sizeof clientsAddresses;
    int clientFD = acceptIncomingConnection(serverSocketFD,
                                            (struct sockaddr*) &clientsAddresses,
                                            &addressSize);
    char clientAddress[INET6_ADDRSTRLEN];
    inet_ntop(clientsAddresses.ss_family,
              getInternetAddresBasedOnIPversion((struct sockaddr*) &clientsAddresses),
              clientAddress,
              sizeof clientAddress);
    printf("Server: got connection from %s\n", clientAddress);

    return clientFD;
}

int setupServer(char* listeningPort)
{
    int serverSocketFD = bindTo(NULL, listeningPort);
    if(serverSocketFD == -1)
    {
        fprintf(stderr, "Could not estabilish a connection, exiting...\n");
        exit(1);
    }

    int maxClientsQueueSize = 20;
    printf("Listening on port %s\n", listeningPort);
    startListening(serverSocketFD, maxClientsQueueSize);
    setupChildProcessSignalHandler();
    return serverSocketFD;
}

void receiveMessage(int clientFD, char* buffer, int maxBufferSize)
{
    int bytesRead = recv(clientFD, buffer, maxBufferSize-1, 0);
    if(bytesRead == -1)
    {
        printf("Unknown error occured. Check errno");
        exit(1);
        return;
    }
    if(bytesRead == 0)
    {
        printf("Connection closed by client.");
        exit(1);
        return;
    }
    buffer[bytesRead] = '\0';
}

void sendMessage(int clientFD, char* message, int messageSize)
{
    if (send(clientFD, message, messageSize, 0) == -1)
    {
        printf("Unknown error occured. Check errno");
    }
}
#endif
