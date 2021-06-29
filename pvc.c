/*
 * GAME STATE ARCHITECTURE
 *
 * The game state (both X and O bitboards) are stored in a single 
 * unsigned 32-bit integer (8 Bytes).
 *
 * BYTE 1:  0000 0000
 * BYTE 2:  000X XXXX
 * BYTE 3:  XXXX 000O
 * BYTE 4:  OOOO OOOO
 *
 * The positions marked X and O are the respective bitboards of the
 * X and Y Playables.
 *
 * ------- MAKING A PLAY 
 *
 * To set a Bit (i.e. to make a play), the entire game state is 
 * ORed with the specific bitmask from the state_bitmasks array 
 *
 * Assume I want to play X at position 1. The empty game state is
 * 0x00000000 and the state mask for X at position 1 is 0x00100000. Hence,
 * 0x00000000 OR 0x00100000 gives me 0x00100000.
 *
 * Likewise, if I want to play O at position 1, I need to do assign the state to itself
 * ORed with the specific state bitmask
 * 0x00100000 OR 0x00001000, which is 0x00100100
 *
 * ------- CHECKING A POSITION
 *
 * To Check if a bit is set or not, we just AND the game state with the specific bitmask.
 * Assume I want to query if O has been played at 1 in the previous bitboard.
 *
 * 0x00100100 AND 0x00000100 gives us 0x00000100, which is a non-zero value, indicating
 * the bit is set. If the bit has not been set, the value of the AND operation will be 0.
 *
 * To Evaluate if a board is in a win state, we just AND the game state with a win position.
 * if the value is greater than zero, then the player has won.
 *
 * ------- CHECKING BOARD VALIDITY
 * To ensure the board is valid, we need to make sure both X and O haven't been played at that state.
 * (state & (state >> 12)) should be zero. We ensure there are no positions in common between the X 
 * and Y bitboards by 
 * 1. Converting to the same range
 * 2. ANDing the states.
 *
 * ------ CHECKING FOR WIN
 * It's essentially the same as checking if a position's full, but just AND with a win state bitmask.
 * 
 */


#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "pvc.h"

// constant values

const uint32_t state_bitmasks[2][9] = {
    {0x00100000, 0x00080000, 0x00040000, 0x00020000, 0x00010000, 0x00008000, 0x00004000, 0x00002000, 0x00001000},
    {0x00000100, 0x00000080, 0x00000040, 0x00000020, 0x00000010, 0x00000008, 0x00000004, 0x00000002, 0x00000001}
};

const uint32_t win_bitmasks[2][8] = {
    {0x00111000, 0x00054000, 0x001C0000, 0x00038000, 0x00007000, 0x00124000, 0x00092000, 0x00049000},
    {0x00000111, 0x00000054, 0x000001C0, 0x00000038, 0x00000007, 0x00000124, 0x00000092, 0x00000049}
};

const int all_fill_bitmask = 0x000001FF;

typedef enum {
    X, O
} playables;

typedef struct node_t
{
    // game state
    uint32_t game_state;
    int state_score;

    // move to get to this game state
    int position;
    playables p;

    // previous and next states
    struct node_t* previous;
    // this is an array of future states. Will be NULL 
    // if the state is a win/loss/draw state
    struct node_t** future_states; 

    // boolean status indicator
    bool is_maximizer;
} node;


// utility functions

int get_index_from_playable(playables p)
{
    // function to check whether the playable is valid
    // and return its index in the list
    switch (p)
    {
        case X:
            return 0;
        case O:
            return 1;
        default:
            printf("This is an Invalid Playable!\n");
            return 9;
    }
}

int verify_position(int position)
{
    // function to range-check a position and
    // return the index to check for the position
    if (position >= 1 && position <= 9)
    {
        return position - 1;
    }
    else {
        return -1;
    }
}

char* get_string_for_playable(playables p)
{
    // get the string for each playable
    switch (p)
    {
        case X:
            return "X";
        case O:
            return "O";
        default:
            printf("This is an invalid playable.\n");
            break;
    }
}

/*
 * BITBOARD METHODS
 */

int check_board_validity(uint32_t state)
{
    /*
     * Check the Validity of the Board
     * Return 0 if the board is invalid, 
     * else Return 1
     */
    return !(state & (state >> 12));
}

int check_win(uint32_t state, playables p)
{
    // check if playable P has won the game or not
    int index = get_index_from_playable(p);
    for (int i = 0; i < 8; i++)
    {
        // printf("\n");
        // printf("Evaluating \n");
        // print_board(state);
        // printf("Against \n");
        // print_board(win_bitmasks[index][i]);
        uint32_t result = win_bitmasks[index][i] & state;
        // print_board(result);

        // check for equality so that 
        if (!(result ^ win_bitmasks[index][i]))
        {
            printf("Win!\n");
            return 1;
        }

    }
    return 0;
}



// heuristic function
int heuristic(uint32_t state, playables p)
{
    /*
     * This is the Heuristic Function for the Game Board
     * Returns 1 for a win for the specific player Returns 0 for a draw
     * Returns -1 for a loss for the specific player
     *
     * Unwieldy hack: create temporary playable array to handle index toggles
     *
     */
    playables p_array[2] = {X, O};
    int player_index = get_index_from_playable(p);

    playables anti_player = p_array[!player_index];

    if (check_win(state, p))
    {
        return 1;
    }
    // else if (check_draw(state))
    // {
    //     return 0;
    // }
    else if (check_win(state, anti_player))
    {
        return -1;
    }
    else {
        return -10;
    }

}


int get_state(uint32_t state, playables player, int position)
{
    /*
     * Function to check if the either X or O are played at
     * position <position>
     *
     * returns 1 if the position is filled for the playable,
     * and 0 if it's not
     *
     */
    int selector = get_index_from_playable(player);
    int index = verify_position(position);
    if (selector < 0 || index < 0)
    {
        return -1;
    }
    return !!(state_bitmasks[selector][index] & state);
}

// print the board
void print_board(uint32_t state)
{
    // check to make sure the board is valid
    int valid = check_board_validity(state);
    printf("Validity %d\n", valid);

    if (!valid)
    {
        printf("Board is Invalid. Exiting\n");
        return;
    }

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            int index = 3*i + j + 1;
            if (get_state(state, X, index))
            {
                printf(" X ");
            }
            else if (get_state(state, O, index))
            {
                printf(" O ");
            }
            else {
                printf(" * ");
            }
        }
        printf("\n");
    }
}




uint32_t set_state(uint32_t state, playables player, int position)
{
    /*
     * Function to set the state of the either X or O at
     * a position <position>
     *
     * returns 1 for succesful assignments, and -1 for errors
     */
    int selector = get_index_from_playable(player);
    int index = verify_position(position);
    if (selector < 0 || index < 0)
    {
        return -1;
    }
    uint32_t new_state = state_bitmasks[selector][index];
    /* 
    printf("Making A play at \n");
    print_board(new_state);
    printf("on the following board \n");
    print_board(state);
    */
    uint32_t modified = state | new_state;
    return modified;
}

int check_draw(uint32_t state)
{
    // if it's a draw, it's
    // 1. NOT A WIN
    // 2. All spaces are full
    // print_board(state);

    // OR the bits to get superimposed positions of bitboards X and O
    uint32_t temp_result = state | (state >> 12);

    // retain only the last 12 bits
    temp_result &= 0x000001FF;

    // toggle the last 12 bits using XOR
    uint32_t result = all_fill_bitmask ^ temp_result;

    if (result == 0)
    {
        printf("Draw!\n");
        return 1;
    }
    else {
        return 0;
    }
}


// print status
void print_play_status(playables p, int index)
{
    printf(
            "Playing %s at %d.\n",
            (p == X) ? "X" : "O",
            index
            );
}

int make_play(uint32_t* state, playables playable, int position)
{

    /*
     * mutate game state to play <playable> at position
     * <position>.
     *
     * return -1 for an usuccessful play, and 1 for a succesful play
     *
     */
    // printf("validity %d\n", check_board_validity(*state));
    uint32_t temp_state = set_state(*state, playable, position);

    if (check_board_validity(temp_state) != 1)
    {
        printf("Illegal Move!\n");
        return -1;
    }
    else {

        *state = temp_state;
        return 1;
    }
}

/*
 * BOARD/NODE methods
 */


node** generate_moves(node* origin);





void play_pvc()
{

    // implement the player vs computer game here
    printf("Playing the PVC game.\n");
    uint32_t state = 0;
    int flag = 0;
    int lead_choice = 0;
    int turn = 0;

    // testcase for draw
    // printf("%d\n", check_draw(0x0017208D));

    playables seq_array[2];

    printf("WELCOME TO PVC TICTACTOE!\n");
    printf("1 - X first, 2 - O first\n");
    scanf("%d", &lead_choice);

    switch (lead_choice)
    {
        case 1:
            seq_array[0] = X;
            seq_array[1] = O;
            break;

        case 2:
            seq_array[0] = O;
            seq_array[1] = X;
            break;

        default:
            printf("That isn't a valid answer! exiting..\n");
            flag = 1;
            break;
    }

    printf("Beginning Game...\n");

    // game loop
    while (!flag)
    {
        playables current = seq_array[turn % 2];
        char* playable_string = get_string_for_playable(current);
        printf("Player Turn : %s\n", playable_string);

        printf("The board is currently >> \n");
        print_board(state);

        int play_pos;
        printf("Enter the position to play at (1-9) >> ");
        scanf("%d", &play_pos);

        int result = make_play(&state,  current, play_pos);
        if (result == -1)
        {
            flag = 1;
        }
        else if (check_win(state, current))
        {
            printf("Player %s wins!\n", playable_string);
            flag = 1;
        }

        turn++;

    }

}
