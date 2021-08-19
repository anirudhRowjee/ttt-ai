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
#include <stdlib.h>
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

// one specific state of the game.
typedef struct node_t
{

    // current state of the game
    uint32_t state;

    // current playable
    playables current_playable;

    // ancestral state
    struct node_t* previous;

    // state score (min_max values come here)
    int score;

    // boolean to determine if the node is a maximizer node or a minimizer node
    // this determines the function to be used to compare all the values after a
    // recursive call.
    bool is_maximizer;

    // array of pointers to previous states
    // at max nine
    struct node_t** future_states;

    // the move required to reach this state
    playables move_playable;
    int move_index;

    // keep track of the number of children (index iteration)
    int children_count;

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

playables get_next_playable(playables p)
{
    // returns X for O, and O for X
    switch (p)
    {
        case X:
            return O;
        case O:
            return X;
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
    return !(0x000001FF & (state & (state >> 12)));
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
            // printf("Win!\n");
            return 1;
        }

    }
    return 0;
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
        // printf("Draw!\n");
        return 1;
    }
    else {
        return 0;
    }
}



int heuristic(uint32_t state, playables p)
{
    /*
     * Heuristic Function for the Game Board
     * Returns 1 for a win for the specific player 
     * Returns 0 for a draw
     * Returns -1 for a loss for the specific player
     */
    playables anti_player = get_next_playable(p);

    if (check_win(state, p))
    {
        return 1;
    }
    else if (check_draw(state))
    {
        return 0;
    }
    else if (check_win(state, anti_player))
    {
        return -1;
    }
    else {
        return 2;
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


// check if an index is set or not
int check_index(uint32_t state, int position)
{
    return (get_state(state, X, position) || get_state(state, O, position));
}

// print the board
void print_board(uint32_t state)
{
    // check to make sure the board is valid
    int valid = check_board_validity(state);
    // printf("Validity %d\n", valid);
    if (!valid)
    {
        // printf("Board is Invalid. Exiting\n");
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

// print status
void print_play_status(playables p, int index)
{
    printf(
            "Playing %s at %d.\n",
            (p == X) ? "X" : "O",
            index
            );
}

uint32_t make_play(uint32_t state, playables playable, int position)
{

    /*
     * mutate game state to play <playable> at position
     * <position>.
     *
     * return -1 for an usuccessful play, and 1 for a succesful play
     *
     */
    // printf("validity %d\n", check_board_validity(*state));
    uint32_t temp_state = set_state(state, playable, position);

    if (check_board_validity(temp_state) != 1)
    {
        // printf("Illegal Move!\n");
        return -1;
    }
    else {
        return temp_state;
    }
}

/*
 * BOARD/NODE methods
 */

void print_node(node* origin)
{
    // function to print a node
    printf(" ======= PRINTING NODE AT %p\n", origin);
    printf("\tPLAYABLE: %s\n", get_string_for_playable(origin->current_playable));
    printf("\tPARENT: %p\n", origin->previous);
    printf("\tSCORE: %d\n", origin->score);
    printf("\tIS_MAXIMIZER: %d\n", origin->is_maximizer);
    printf("\tFUTURE STATES: %p\n", origin->future_states);
    printf("\tMOVE PLAYABLE: %s\n", get_string_for_playable(origin->current_playable));
    printf("\tMOVE INDEX: %d\n", origin->move_index);
    printf("\tCHILD COUNT: %d\n", origin->children_count);
}


node** generate_moves(node* origin, int depth)
{
    /*
     * Move generator
     * Works by playing a move at the first empty positon using get_state for
     * the given playable, and rejecting invalid moves
     *
     * Returns an array of all the possible valid moves, and NULL if the state is a win state
     *
     */

    playables next_player = get_next_playable(origin->current_playable);

    // check to see if the current game state is valid or not
    // if the current node is either a win or a loss, we return 
    // and do nothing
    int status = heuristic(origin->state, origin->current_playable);

    int current_children_count = 0;

    // declare the next pointer array
    node** next_moves = malloc(sizeof(node*)*9);

    if (status >= -1 && status <= 1)
    {
        // terminal state, no future states possible
        // assign score to origin node and move on
        origin->score = status;
        // printf("Terminal state >> \n");
        // print_board(origin->state);
        // origin->children_count = 0;
        free(next_moves);
        return NULL;
    }
    else
    {
        // non-terminal state with possible future states
        // recursively call generate_moves on each node
        // spawned, maximize/minimize based on the is_maximizer
        // function and then output the score

        // This forces it into DFS by default
        for (int i = 0; i < 9; i++)
        {
            // check for possible open positions

            // make the play, and check if it's valid or not
            uint32_t played = make_play(origin->state, origin->current_playable, i+1);

            if (check_board_validity(played))
            {
                // printf("Valid move for state at %d\n", i+1);
                // we have found a valid move! allocate the memory for it.
                node* new_node = malloc(sizeof(node));
                if (new_node == NULL)
                {
                    // bad news - malloc failed
                    printf("\n Allocation Failed! Exiting.. \n");
                    exit(1);
                }
                else
                {
                    // allocation successfull! we now allocate everything else.

                    // set the next "origin"
                    new_node->state = played;
                    new_node->current_playable = next_player;

                    new_node->previous = origin;

                    new_node->score = 10;

                    // flip the bit
                    new_node->is_maximizer = !origin->is_maximizer;

                    new_node->future_states = NULL;

                    new_node->move_playable = origin->current_playable;
                    new_node->move_index = i+1;

                    new_node->children_count = 0;

                    // TODO implement recursive call here, and maximize

                    if (depth > 0)
                    {
                        new_node->future_states = generate_moves(new_node, depth-1);
                    }
                    else
                    {
                        new_node->future_states = NULL;
                    }
                }
                // allocate to array index
                // print_node(new_node);
                // print_board(new_node->state);
                next_moves[current_children_count] = new_node;
                current_children_count++;
            }
        }
        origin->children_count = current_children_count;
    }

    // TODO modify score here
    return next_moves;
}

void free_game_tree(node* origin)
{
    // printf("Starting recursive free at %p\n", origin);
    // recursively free the game tree
    if (origin->future_states == NULL)
    {
        // base case! do nothing
        // printf("No Future States!\n");
    }
    else
    {
        for (int i = 0; i < origin->children_count; i++)
        {
            if (origin->future_states[i] != NULL)
            {
                free_game_tree(origin->future_states[i]);
            }
        }
        // free the pointer array itself
        free(origin->future_states);
        // printf("Free Complete!\n");
    }
    free(origin);
}


// function to run the minimax algorithm from a node
// and return the index of the next best possible move.
int run_minimax(node* origin)
{
    // the score of a node is either the maximum
    // or the minimum of its child nodes
    // this is decided based on the :is_maximizer flag

    // index of the move to be made
    // also the return value
    int move_tbm_index = 0;

    // safe iteration limits so that we don't segfault 
    int node_children_count = origin->children_count;

    // if it's a terminal state, return the score
    // this won't trigger on non-terminal nodes as the
    // default score for non-terminal nodes is 10
    if (origin->score >= -1 && origin->score <= 1)
    {
        return origin->score;
    }

    // decision state
    if (origin->is_maximizer)
    {
        // safe lower bound
        int max_score = -10;
        // recursively call this function on each child node to find the maximum
        for (int i = 0; i < node_children_count; i++)
        {
            node* operational_node = origin->future_states[i];
            int node_score = run_minimax(operational_node);
            // printf("Score for Node %p is %d\n", operational_node, node_score);
            if (node_score > max_score)
            {
                max_score = operational_node->move_index;
            }
        }
        move_tbm_index = max_score;
    }
    else // defualts to minimizer
    {
        // safe upper bound
        int min_score = 100;
        // recursively call this function on each child node to find the minimum
        for (int i = 0; i < node_children_count; i++)
        {
            node* operational_node = origin->future_states[i];
            int node_score = run_minimax(operational_node);
            // printf("Score for Node %p is %d\n", operational_node, node_score);
            if (node_score < min_score)
            {
                min_score = operational_node->move_index;
            }
        }
        move_tbm_index = min_score;
    }
    return move_tbm_index;
}


int generate_move_for_state(uint32_t state)
{
    // function to allocate and generate game tree for a specific game
    // state
    node* origin = malloc(sizeof(node));

    // use this constant pointer to free the game state
    const node* origin_ref = origin;

    // populate genesis node
    origin->state = state;
    origin->current_playable = X;
    origin->previous = NULL;
    origin->score = 10;
    origin->is_maximizer = true;
    origin->future_states = NULL;
    origin->move_playable = X;
    origin->move_index = 0;
    origin->children_count = 0;

    printf("Generating Game Tree For >> \n");
    print_node(origin);
    node** testbed = generate_moves(origin, 8);
    origin->future_states = testbed;

    // run while we still have access to game tree
    int wm_index = run_minimax(origin);
    printf("Winning Move at %d\n", wm_index);

    // free the game tree
    free_game_tree(origin);
    return wm_index;
}


void play_pvc()
{

    // printf("%ld\n", sizeof(node));

    // implement the player vs computer game here
    printf("Playing the PVC game.\n");
    uint32_t state = 0;
    int flag = 0;
    int lead_choice = 0;
    int turn = 0;

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

        // set X to be the first player, computer
        if (current == X)
        {
            // generate the move
            int play_pos = generate_move_for_state(state);
            state = make_play(state,  current, play_pos);
            if (state == -1)
            {
                printf("Invalid Move!\n");
                flag = 1;
                break;
            }
        }
        else
        {
            int play_pos;
            printf("Enter the position to play at (1-9) >> ");
            scanf("%d", &play_pos);

            state = make_play(state,  current, play_pos);
            if (state == -1)
            {
                printf("Invalid Move!\n");
                flag = 1;
                break;
            }
        }

        int winstatus = heuristic(state, current);

        switch (winstatus)
        {
            case 1:
                print_board(state);
                printf("Player Wins!\n");
                flag = 1;
                break;

            case -1:
                print_board(state);
                printf("Player Loses!\n");
                flag = 1;
                break;

            case 0:
                print_board(state);
                printf("Draw!\n");
                flag = 1;
                break;
        }
        turn++;

    }

    // free_game_tree(origin);
    printf("Free Complete!\n");

}
