#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>
#include <algorithm>

void printBoard(const std::vector<char>& board) {
    for (int i = 0; i < 9; ++i) {
        std::cout << board[i] << ' ';
        if ((i + 1) % 3 == 0) std::cout << '\n';
    }
}

bool isWin(const std::vector<char>& board, char player) {
    // Check rows, columns, and diagonals
    return (board[0] == player && board[1] == player && board[2] == player) ||
           (board[3] == player && board[4] == player && board[5] == player) ||
           (board[6] == player && board[7] == player && board[8] == player) ||
           (board[0] == player && board[3] == player && board[6] == player) ||
           (board[1] == player && board[4] == player && board[7] == player) ||
           (board[2] == player && board[5] == player && board[8] == player) ||
           (board[0] == player && board[4] == player && board[8] == player) ||
           (board[2] == player && board[4] == player && board[6] == player);
}

bool isDraw(const std::vector<char>& board) {
    for (char cell : board) {
        if (cell == ' ') return false;
    }
    return true;
}

bool isValidMove(const std::vector<char>& board, int move) {
    return move >= 0 && move < 9 && board[move] == ' ';
}

int main(int argc, char* argv[]) {
    // Check if exactly one argument is provided
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <9-digit-number>\n";
        return 1;
    }

    std::string input = argv[1];

    // Check if input is valid
    if (input.length() != 9 || !std::all_of(input.begin(), input.end(), ::isdigit) || 
        std::unordered_set<char>(input.begin(), input.end()).size() != 9) {
        std::cerr << "Invalid input. Please provide a 9-digit number containing each digit from 1 to 9 exactly once.\n";
        return 1;
    }

    // Initialize the board
    std::vector<char> board(9, ' ');
    char currentPlayer = 'X';

    // Make the first move
    int firstMove = input[0] - '1';
    board[firstMove] = currentPlayer;
    printBoard(board);

    // Alternate players
    currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';

    // Play the game
    for (size_t i = 1; i < 9; ++i) {
        int move;
        if(currentPlayer == 'O'){
            std::cout << "Player " << currentPlayer << ", enter your move (1-9): ";
            std::cin >> move;
            move -= 1; // Convert to 0-based index
        } else {
            int j = 0;
           while(board[input[j]-'1']!=' ' && (size_t)j < input.size()){
                j++;
           }
            move = input[j]-'1';
            
        }
        if (currentPlayer=='O'&&!isValidMove(board, move)) {
            std::cerr << "Invalid move. Try again.\n";
            --i; // Repeat this turn
            continue;
            
        }

        board[move] = currentPlayer;
        printBoard(board);

        if (isWin(board, currentPlayer)) {
            std::cout << "Player " << currentPlayer << " wins!\n";
            return 0;
        }

        if (isDraw(board)) {
            std::cout << "The game is a draw.\n";
            return 0;
        }

        currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
    }

    return 0;
}
