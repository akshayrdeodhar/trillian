"Chess"

# DSA Project

111703013
Akshay Deodhar
SY BTech
Computer Engineering

Usage: ./project <file.sv OR file.fen>

Chess program.

# Board State
Supports Forsyth Edwards Format for storing board position
FEN is a format which stores the state of the chessboard as a string.
The string is of form:

                  end of row
                        |
  r3k2r/pppppppp/8/8/8/8/PPPPPPPP/1KR5
    |            |                  |
rank 8        8 blank square       rook 

The program takes a filename as commandline arguement. 
The file is in format
Line 1: FEN line
Line 2: Player info (human or computer, names) [OPTIONAL]
The program checks whether the string is a valid FEN, converts it to board state, and starts game from that state
If invalid, program exits

Alternately, if no filename is specified, game starts from default starting position of board

# Game Loop
Once the game starts, the game waits for user input. The input can be
[a-h][1-8]-[a-h][1-8]: a move which specifies the initial square (from) and the final square (to)
board: prints board again
save: saves state of board in ../save/ folder (FEN + player-info)
help: prints help
quit: exits as it is

The game checks whether the move is valid- if yes, it makes the move and updates the state accordingly
If invalid, the game simply goes to the next iteration of the game loop (and thus, tries again)

The state of the board and pieces is updated after each move. 
At every point, the *number of squares the piece  can move in a direction, along with the piece occupying the LAST square in that direction is stored*
When a move is made, for every piece, the program checks whether the *from* and *to* squares are in the range of the piece. It recalculates the moves of the piece in the direction from position of piece to the square. Only if the direction is a valid direction for the piece, dies it recalculate the pieces moves in that direction.
The program uses a set of number codes which represent a direction 
There are static arrays which store the delta-x and delta-y for that direction (which are used for move generation)

# Computer Player - Trillian
There is a way to update the state of the board and possible moves of the piece for each move made
The Computer player uses the minmax algorithm, with alpha-beta pruning. 
Minmax uses a static evaluation function. The function has 3 main parameters-

Values of pieces
Number of squares controlled
King safety

The function uses these parameters to generate a number which represents the *goodness* of the position for white and black. A positive score is good for white, and negative for black. Minmax tries to *minimise* the score for black, and *maximise* the score for white. 
For example, for a depth-2 minmax, (assume white starts)

White tries ALL moves, and then for each move, calculates all possible moves for black. Now the score for the best move for black, is the score of that *branch*. Now, this is done recursively, with white doing MAX(minimise) ad black doing MIN(maximise)
Alpha beta pruning eliminates those moves in search using information about best move for black or white till that point.

The algorithm works fairly fast upto depth 4, but takes more than 1.5 minutes for depth 6. Odd depths are unreliable, as do not end with opponent's move.
