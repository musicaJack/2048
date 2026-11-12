/**
 * @file game_logic.cpp
 * @brief 2048游戏逻辑实现
 */

#include "game/game_logic.hpp"
#include "game/game_state.hpp"
#include <algorithm>

namespace game2048 {

GameLogic::GameLogic() = default;

GameLogic::~GameLogic() = default;

bool GameLogic::moveLeft(GameState& game_state) {
    BoardArray& board = game_state.getBoard();
    bool moved = false;
    int gained = 0;
    
    for (int i = 0; i < BOARD_SIZE; ++i) {
        moved |= moveRowLeft(board[i], gained);
    }
    
    if (moved) {
        game_state.addRandomTile();
        game_state.updateLastMoveTime();
        game_state.updateScore(gained);
    }
    
    return moved;
}

bool GameLogic::moveRight(GameState& game_state) {
    BoardArray& board = game_state.getBoard();
    bool moved = false;
    int gained = 0;

    for (int i = 0; i < BOARD_SIZE; ++i) {
        moved |= moveRowRight(board[i], gained);
    }
    
    if (moved) {
        game_state.addRandomTile();
        game_state.updateLastMoveTime();
        game_state.updateScore(gained);
    }
    
    return moved;
}

bool GameLogic::moveUp(GameState& game_state) {
    BoardArray& board = game_state.getBoard();
    bool moved = false;
    int gained = 0;

    for (int j = 0; j < BOARD_SIZE; ++j) {
        moved |= moveColumnUp(board, j, gained);
    }
    
    if (moved) {
        game_state.addRandomTile();
        game_state.updateLastMoveTime();
        game_state.updateScore(gained);
    }
    
    return moved;
}

bool GameLogic::moveDown(GameState& game_state) {
    BoardArray& board = game_state.getBoard();
    bool moved = false;
    int gained = 0;

    for (int j = 0; j < BOARD_SIZE; ++j) {
        moved |= moveColumnDown(board, j, gained);
    }
    
    if (moved) {
        game_state.addRandomTile();
        game_state.updateLastMoveTime();
        game_state.updateScore(gained);
    }
    
    return moved;
}

bool GameLogic::moveRowLeft(int row[BOARD_SIZE], int& gained) {
    bool moved = false;
    int write_pos = 0;
    
    // 第一步：移除所有0，将非0数字移到左边
    for (int i = 0; i < BOARD_SIZE; ++i) {
        if (row[i] != 0) {
            if (write_pos != i) {
                row[write_pos] = row[i];
                row[i] = 0;
                moved = true;
            }
            write_pos++;
        }
    }
    
    // 第二步：从左到右合并相同的数字
    for (int i = 0; i < BOARD_SIZE - 1; ++i) {
        if (row[i] != 0 && row[i] == row[i + 1]) {
            row[i] *= 2;
            gained += row[i];
            row[i + 1] = 0;
            moved = true;
        }
    }
    
    // 第三步：再次移除0
    write_pos = 0;
    for (int i = 0; i < BOARD_SIZE; ++i) {
        if (row[i] != 0) {
            if (write_pos != i) {
                row[write_pos] = row[i];
                row[i] = 0;
                moved = true;
            }
            write_pos++;
        }
    }
    
    return moved;
}

bool GameLogic::moveRowRight(int row[BOARD_SIZE], int& gained) {
    bool moved = false;
    int write_pos = BOARD_SIZE - 1;
    
    // 第一步：移除所有0，将非0数字移到右边
    for (int i = BOARD_SIZE - 1; i >= 0; --i) {
        if (row[i] != 0) {
            if (write_pos != i) {
                row[write_pos] = row[i];
                row[i] = 0;
                moved = true;
            }
            write_pos--;
        }
    }
    
    // 第二步：从右到左合并相同的数字
    for (int i = BOARD_SIZE - 1; i > 0; --i) {
        if (row[i] != 0 && row[i] == row[i - 1]) {
            row[i] *= 2;
            gained += row[i];
            row[i - 1] = 0;
            moved = true;
        }
    }
    
    // 第三步：再次移除0
    write_pos = BOARD_SIZE - 1;
    for (int i = BOARD_SIZE - 1; i >= 0; --i) {
        if (row[i] != 0) {
            if (write_pos != i) {
                row[write_pos] = row[i];
                row[i] = 0;
                moved = true;
            }
            write_pos--;
        }
    }
    
    return moved;
}

bool GameLogic::moveColumnUp(BoardArray& board, int col, int& gained) {
    bool moved = false;
    int write_pos = 0;
    
    // 第一步：移除所有0，将非0数字移到上边
    for (int i = 0; i < BOARD_SIZE; ++i) {
        if (board[i][col] != 0) {
            if (write_pos != i) {
                board[write_pos][col] = board[i][col];
                board[i][col] = 0;
                moved = true;
            }
            write_pos++;
        }
    }
    
    // 第二步：从上到下合并相同的数字
    for (int i = 0; i < BOARD_SIZE - 1; ++i) {
        if (board[i][col] != 0 && board[i][col] == board[i + 1][col]) {
            board[i][col] *= 2;
            gained += board[i][col];
            board[i + 1][col] = 0;
            moved = true;
        }
    }
    
    // 第三步：再次移除0
    write_pos = 0;
    for (int i = 0; i < BOARD_SIZE; ++i) {
        if (board[i][col] != 0) {
            if (write_pos != i) {
                board[write_pos][col] = board[i][col];
                board[i][col] = 0;
                moved = true;
            }
            write_pos++;
        }
    }
    
    return moved;
}

bool GameLogic::moveColumnDown(BoardArray& board, int col, int& gained) {
    bool moved = false;
    int write_pos = BOARD_SIZE - 1;
    
    // 第一步：移除所有0，将非0数字移到下边
    for (int i = BOARD_SIZE - 1; i >= 0; --i) {
        if (board[i][col] != 0) {
            if (write_pos != i) {
                board[write_pos][col] = board[i][col];
                board[i][col] = 0;
                moved = true;
            }
            write_pos--;
        }
    }
    
    // 第二步：从下到上合并相同的数字
    for (int i = BOARD_SIZE - 1; i > 0; --i) {
        if (board[i][col] != 0 && board[i][col] == board[i - 1][col]) {
            board[i][col] *= 2;
            gained += board[i][col];
            board[i - 1][col] = 0;
            moved = true;
        }
    }
    
    // 第三步：再次移除0
    write_pos = BOARD_SIZE - 1;
    for (int i = BOARD_SIZE - 1; i >= 0; --i) {
        if (board[i][col] != 0) {
            if (write_pos != i) {
                board[write_pos][col] = board[i][col];
                board[i][col] = 0;
                moved = true;
            }
            write_pos--;
        }
    }
    
    return moved;
}

bool GameLogic::canMove(const GameState& game_state) const {
    const BoardArray& board = game_state.getBoard();
    
    // 检查是否有空位置
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (board[i][j] == 0) {
                return true;
            }
        }
    }
    
    // 检查是否可以合并
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            int current = board[i][j];
            
            // 检查右边
            if (j < BOARD_SIZE - 1 && board[i][j + 1] == current) {
                return true;
            }
            
            // 检查下边
            if (i < BOARD_SIZE - 1 && board[i + 1][j] == current) {
                return true;
            }
        }
    }
    
    return false;
}

} // namespace game2048 
