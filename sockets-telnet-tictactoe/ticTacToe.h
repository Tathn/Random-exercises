#ifndef TICTACTOE_H
#define TICTACTOE_H

#include "socketUtils.h"
#include <time.h>

struct TicTacToe
{
    char fields[3][3];
};

void sendGameField(int clientFD, struct TicTacToe* game)
{
    char gameFieldToPrint[6][9];
    strcpy(gameFieldToPrint[0], "  0 1 2\n\0");
    snprintf(gameFieldToPrint[1], sizeof(gameFieldToPrint[0]), "0 %C|%C|%C\n\0", game->fields[0][0], game->fields[0][1], game->fields[0][2]);
    
    strcpy(gameFieldToPrint[2], "  -----\n\0");
    snprintf(gameFieldToPrint[3], sizeof(gameFieldToPrint[2]), "1 %C|%C|%C\n\0", game->fields[1][0], game->fields[1][1], game->fields[1][2]);
    
    strcpy(gameFieldToPrint[4], "  -----\n\0");
    snprintf(gameFieldToPrint[5], sizeof(gameFieldToPrint[4]), "2 %C|%C|%C\n\0", game->fields[2][0], game->fields[2][1], game->fields[2][2]);
    
    for(int i = 0; i < 6; ++i)
    {
        sendMessage(clientFD, gameFieldToPrint[i], 9);
    }
}
int validateChoice(int x, int y, struct TicTacToe* game)
{
    int fieldEmpty = game->fields[y][x] == ' ';
    return x >= 0 && x < 3 && y >= 0 && y < 3 && fieldEmpty;
}

void place(int x, int y, struct TicTacToe* game, char symbol)
{
    game->fields[y][x] = symbol;
}

void computerPlace(struct TicTacToe* game)
{
    int xs[9];
    int ys[9];
    int index = -1;
    for(int y = 0; y < 3; ++y)
    {
        for(int x = 0; x < 3; ++x)
        {
            if(game->fields[y][x] == ' ')
            {
                index += 1;
                xs[index] = x;
                ys[index] = y;
            }
        }
    }
    int choice = rand() % index + 1;
    int y = ys[choice];
    int x = xs[choice];
    place(x, y, game, 'o');
}

int checkFieldsEquality(char field1, char field2, char field3)
{
    return field1 != ' ' && field2 != ' ' && field3 != ' ' &&
           (field1 == field2 && field2 == field3);
}

int checkIfSomeoneWon(struct TicTacToe* game)
{
    return checkFieldsEquality(game->fields[0][0], game->fields[0][1], game->fields[0][2]) ||
           checkFieldsEquality(game->fields[1][0], game->fields[1][1], game->fields[1][2]) ||
           checkFieldsEquality(game->fields[2][0], game->fields[2][1], game->fields[2][2]) ||
           checkFieldsEquality(game->fields[0][0], game->fields[1][0], game->fields[2][0]) ||
           checkFieldsEquality(game->fields[0][1], game->fields[1][1], game->fields[2][1]) ||
           checkFieldsEquality(game->fields[0][2], game->fields[1][2], game->fields[2][2]) ||
           checkFieldsEquality(game->fields[0][0], game->fields[1][1], game->fields[2][2]) ||
           checkFieldsEquality(game->fields[0][2], game->fields[1][1], game->fields[2][0]);
}

int checkIfFreeSlotsAvailable(struct TicTacToe* game)
{
    for(int y = 0; y < 3; ++y)
    {
        for(int x = 0; x < 3; ++x)
        {
            if(game->fields[y][x] == ' ')
            {
                return 1;
            }
        }
    }
    return 0;
}

int playerMove(int x, int y, struct TicTacToe* gameField, int clientFD)
{
    place(x, y, gameField, 'x');
    if(checkIfSomeoneWon(gameField))
    {
        sendGameField(clientFD, gameField);
        sendMessage(clientFD, "You won! Yay! Thanks for playing...\n", 36);
        return 0;
    }
    if(!checkIfFreeSlotsAvailable(gameField))
    {
        sendGameField(clientFD, gameField);
        sendMessage(clientFD, "Draw! Yay! Thanks for playing...\n", 34);
        return 0;
    }
    return 1;
}

int computerMove(int x, int y, struct TicTacToe* gameField, int clientFD)
{
    computerPlace(gameField);
    if(checkIfSomeoneWon(gameField))
    {
        sendGameField(clientFD, gameField);
        sendMessage(clientFD, "AI won! Yay! Thanks for playing...\n", 35);
        return 0;
    }
    if(!checkIfFreeSlotsAvailable(gameField))
    {
        sendGameField(clientFD, gameField);
        sendMessage(clientFD, "Draw! Yay! Thanks for playing...\n", 34);
        return 0;
    }
    return 1;
}

void startTicTacToeWithClient(int clientFD)
{
    sendMessage(clientFD, "Hello, let's play a game!\n", 26);
    srand(time(NULL));
    struct TicTacToe gameField = { .fields = {
        {' ',' ',' '},
        {' ',' ',' '},
        {' ',' ',' '}
    } };
    
    int maxBufferSize = 100;
    while(1)
    {
        sendGameField(clientFD, &gameField);
        sendMessage(clientFD, 
                    "Choose where to place an X.\nProvide in xy form eg. 01\n Send 'e' to exit.\n",
                    73);

        char choice[maxBufferSize];
        receiveMessage(clientFD, choice, maxBufferSize);
        if(choice[0] == 'e')
        {
            sendMessage(clientFD, "Bye bye...\n", 11);
            exit(0);
        }

        int x = choice[0] - '0';
        char y = choice[1] - '0';

        int result = validateChoice(x, y, &gameField);
        if(!result)
        {
            sendMessage(clientFD, "Please provide correct move command...\n", 39);
            continue;
        }
        if(!playerMove(x, y, &gameField, clientFD) || !computerMove(x, y, &gameField, clientFD))
        {
            break;
        }
    }
}
#endif
