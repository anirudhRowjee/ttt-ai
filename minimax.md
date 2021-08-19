# Minimax - Naive Implementation

This is a brief explanation of the Minimax Algorithm.

## Components


0. Move

A move is a combination of a game state, an index and a playable.

1. Search Tree

This is the procedural representation of the gameplay from state to state. It starts at an origin node, and has links to all possible nodes, which each have their own set of links to future states, and to the previous state.
The search tree is a complete representation of all the ways the game could pan out, given a set of moves.

2. Heuristic Function

This is a function that accepts the game state as input and gives you a number descibing the "favorability" of the state as output. We'll use the Heuristic function to decide which moves are (and later, which moves lead to ) win and loss states.
For a given player, the Heuristic Funtion should output as follows -
    * +1 for a win
    * 0 for a draw
    * -1 for a loss

3. Node

A Node contains the following details -
* The Game State at that point
* The Move required to reach that state
* The Previous State
* The Successive States
* The "Score" of the node
* Minimizer/Maximizer Indicator (single bit to indicate if the node is a maximizer or a minimizer node, and whose turn it is)

A node can be traversed into (we can reach child nodes from the node) or traversed out of (we can reach the parent node from the child node).

4. Move Generator

This is a function that takes the game state and provides you an array of future states with the moves required to reach said states. This retunrs a list of Nodes with preset information for Game State, move and previous state.


5. Tree Creator

NOTE: Tree Generation, Traversal and Evaluation is happening at the same time to ensure compatibility with an implementation of Alpha-Beta Pruning.

The Tree is first generated and then evaluated. Generation can happen in any arbitrary fashion, however, evaluation may only happen in a DFS-style traversal such that upon finding a situaition where Alpha-Beta Pruning may be applied, it's possible to remove the node which doesn't need to be evaluated. 

The first implementation shall implement naive minimax with no pruning. Alpha-Beta pruning will be considered an optimization to be implemented later.




The Game starts off by generating the entire game tree from the start node (usually a Blank Node)


