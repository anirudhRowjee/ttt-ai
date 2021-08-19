#include <stdio.h>

// include the files for the player vs computer game
#include "pvc.h"
// include the files for the player vs player game
#include "pvp.h"

int main()
{

    int choice;

    // the menu goes here
    printf("Welcome to Tic Tac Toe!\n");
    printf("Press 1 to play against another person.\n");
    printf("Press 2 to play against the computer.\n");

    scanf("%d", &choice);

    switch (choice) {
        case 1:
            play_pvp();
            break;
        case 2:
            play_pvc();
            break;
        default:
            printf("This option is incorrect! Try Again.\n");
            break;
    }

}
