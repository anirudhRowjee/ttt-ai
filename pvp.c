#include <stdio.h>
#include "pvp.h"
#include <conio.h>

char array[10] = { 'o', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

int check_if_won();
void board();


void play_pvp()
{
    // implement the player vs player game here
    printf("Playing the PVP game..\n");
    int player = 1, i, select;
    

    char letter;
    do
    {
        board();
        player = (player % 2) ? 1 : 2;
    
        printf("Player %d, enter a number:  ", player);
        scanf("%d", &select);

        letter = (player == 1) ? 'X' : 'O';

        if (select == 1 && array[1] == '1')
            array[1] = letter;
            
        else if (select == 2 && array[2] == '2')
            array[2] = letter;
            
        else if (select == 3 && array[3] == '3')
            array[3] = letter;
            
        else if (select == 4 && array[4] == '4')
            array[4] = letter;
            
        else if (select == 5 && array[5] == '5')
            array[5] = letter;
            
        else if (select == 6 && array[6] == '6')
            array[6] = letter;
            
        else if (select == 7 && array[7] == '7')
            array[7] = letter;
            
        else if (select == 8 && array[8] == '8')
            array[8] = letter;
            
        else if (select == 9 && array[9] == '9')
            array[9] = letter;
            
        else
        {
            printf("------*Invalid move* ");

            player--;
            getch();
        }
        i = check_if_won();

        player++;
    }while (i ==  - 1);
    
    board();
    
    if (i == 1)
        printf("--------------------\aPlayer %d wins--------------------- ", --player);
    else
        printf("----------------------\aGame draw------------------------");

    getch();

    return 0;

}

int check_if_won()
{
    if (array[1] == array[2] && array[2] == array[3])
        return 1;
        
    else if (array[4] == array[5] && array[5] == array[6])
        return 1;
        
    else if (array[7] == array[8] && array[8] == array[9])
        return 1;
        
    else if (array[1] == array[4] && array[4] == array[7])
        return 1;
        
    else if (array[2] == array[5] && array[5] == array[8])
        return 1;
        
    else if (array[3] == array[6] && array[6] == array[9])
        return 1;
        
    else if (array[1] == array[5] && array[5] == array[9])
        return 1;
        
    else if (array[3] == array[5] && array[5] == array[7])
        return 1;
        
    else if (array[1] != '1' && array[2] != '2' && array[3] != '3' &&
        array[4] != '4' && array[5] != '5' && array[6] != '6' && array[7] 
        != '7' && array[8] != '8' && array[9] != '9')

        return 0;
    else
        return  - 1;
}

void board()
{
    
    printf("\n\n\tWelcome to Tic Tac Toe!\n\n");

    printf("Player 1 (X)  VS  Player 2 (O)\n\n\n");


    printf("     |     |     \n");
    printf("  %c  |  %c  |  %c \n", array[1], array[2], array[3]);

    printf("_____|_____|_____\n");
    printf("     |     |     \n");

    printf("  %c  |  %c  |  %c \n", array[4], array[5], array[6]);

    printf("_____|_____|_____\n");
    printf("     |     |     \n");

    printf("  %c  |  %c  |  %c \n", array[7], array[8], array[9]);

    printf("     |     |     \n\n");
}

