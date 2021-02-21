#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "ticTacToe.h"
#include "socketUtils.h"

int main()
{
    int serverSocketFD = setupServer("2137");
    for(int i = 1;; ++i)
    {
        int clientFD = getNewClientConnection(serverSocketFD);
        if(!fork()) {
            printf("Starting connection %d\n", i);
            close(serverSocketFD);
            startTicTacToeWithClient(clientFD);
            close(clientFD);
            exit(0);
        }
        close(clientFD);
    }
    close(serverSocketFD);
    return 0;
}
