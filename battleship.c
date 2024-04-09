#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define GRID_SIZE 10

void initGrid(char grid[][GRID_SIZE]);
void printGrid(char grid[][GRID_SIZE]);
void placeShips(char grid[][GRID_SIZE], int player);
int checkHit(char grid[][GRID_SIZE], int row, int col, int player);
int gameOver(char grid[][GRID_SIZE], int player);

int main() {
    char player1Grid[GRID_SIZE][GRID_SIZE];
    char player2Grid[GRID_SIZE][GRID_SIZE];

    initGrid(player1Grid);
    initGrid(player2Grid);

    placeShips(player1Grid, 1);
    placeShips(player2Grid, 2);

    int currentPlayer = 1;
    int gameFinished = 0;

    while (!gameFinished) {
        printf("\nPlayer %d's turn:\n", currentPlayer);
        printGrid(currentPlayer == 1 ? player1Grid : player2Grid);

        int row, col;
        printf("Enter row and column to fire: ");
        scanf("%d %d", &row, &col);

        int hit = checkHit(currentPlayer == 1 ? player2Grid : player1Grid, row, col, currentPlayer);

        if (hit == 1) {
            printf("Hit!\n");
        } else if (hit == 2) {
            printf("Ship sunk!\n");
        } else {
            printf("Miss!\n");
        }

        gameFinished = gameOver(currentPlayer == 1 ? player2Grid : player1Grid, currentPlayer);

        currentPlayer = currentPlayer == 1 ? 2 : 1;
    }

    printf("\nGame over! Player %d wins!\n", currentPlayer == 1 ? 2 : 1);

    return 0;
}

void initGrid(char grid[][GRID_SIZE]) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = '-';
        }
    }
}

void printGrid(char grid[][GRID_SIZE]) {
    printf("   ");
    for (int i = 0; i < GRID_SIZE; i++) {
        printf("%d ", i);
    }
    printf("\n");

    for (int i = 0; i < GRID_SIZE; i++) {
        printf("%d  ", i);
        for (int j = 0; j < GRID_SIZE; j++) {
            printf("%c ", grid[i][j]);
        }
        printf("\n");
    }
}

void placeShips(char grid[][GRID_SIZE], int player) {
    int numShips, shipLength, row, col, direction;

    printf("\nPlayer %d, place your ships:\n", player);

    for (numShips = 0; numShips < 5; numShips++) {
        printf("Ship %d (length %d):\n", numShips + 1, numShips + 2);

        do {
            printf("Enter row and column for start: ");
            scanf("%d %d", &row, &col);
            printf("Enter direction (0 for horizontal, 1 for vertical): ");
            scanf("%d", &direction);

            if (row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) {
                printf("Invalid coordinates. Try again.\n");
                continue;
            }

            shipLength = numShips + 2;
            int valid = 1;

            for (int i = 0; i < shipLength; i++) {
                int r = direction ? row + i : row;
                int c = direction ? col : col + i;

                if (r < 0 || r >= GRID_SIZE || c < 0 || c >= GRID_SIZE || grid[r][c] != '-') {
                    valid = 0;
                    break;
                }
            }

            if (valid) {
                for (int i = 0; i < shipLength; i++) {
                    int r = direction ? row + i : row;
                    int c = direction ? col : col + i;
                    grid[r][c] = 'S';
                }
                break;
            } else {
                printf("Invalid placement. Try again.\n");
            }
        } while (1);
    }
}

int checkHit(char grid[][GRID_SIZE], int row, int col, int player) {
    if (row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) {
        return 0;
    }

    if (grid[row][col] == 'S') {
        grid[row][col] = 'X';
        int shipSunk = 1;

        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                if (grid[i][j] == 'S') {
                    shipSunk = 0;
                    break;
                }
            }
            if (!shipSunk) {
                break;
            }
        }

        return shipSunk ? 2 : 1;
    } else if (grid[row][col] == '-') {
        grid[row][col] = 'O';
        return 0;
    } else {
        return 0;
    }
}

int gameOver(char grid[][GRID_SIZE], int player) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] == 'S') {
                return 0;
            }
        }
    }
    return 1;
}