
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <arpa/inet.h>  // used to get client info
/* "The <arpa/inet.h> header makes available the type in_port_t and the type in_addr_t as defined in the description " -https://pubs.opengroup.org/ */



#define BUFFER_SIZE	256

/* Default port to listen on */
#define DEFAULT_PORT	31337

#define GRID_SIZE 10

void placeShips(char grid[][GRID_SIZE], int player);
void initGrid(char grid[][GRID_SIZE]);
void printGrid(char grid[][GRID_SIZE]);
int checkHit(char grid[][GRID_SIZE], int row, int col);
int gameOver(char grid[][GRID_SIZE]);

int main(int argc, char **argv) {

	int socket_fd,new_socket_fd;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;	
	int port=DEFAULT_PORT;
	int n,on=1;
	socklen_t client_len;
	char buffer[BUFFER_SIZE];
	int intBuffer[2];

	int result;

	printf("Starting server on port %d\n",port);

	/* Open a socket to listen on */
	/* AF_INET means an IPv4 connection */
	/* SOCK_STREAM means reliable two-way connection (TCP) */
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd<0) {
		fprintf(stderr,"Error opening socket! %s\n",
			strerror(errno));
	}


	/* Set SO_REUSEADDR on socket so that we don't get stuck */
	/* in TIME_WAIT state on early exit */
	setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,(char *)&on,sizeof(on));

	/* Set up the server address to listen on */
	/* The memset to 0 sets the address to 0.0.0.0 which means */
	/* listen on any interface. */
	memset(&server_addr,0,sizeof(struct sockaddr_in));
	server_addr.sin_family=AF_INET;
	/* Convert the port we want to network byte order */
	server_addr.sin_port=htons(port);

	/* Bind to the port */
	result=bind(socket_fd,
		(struct sockaddr *) &server_addr,
		sizeof(server_addr));
	if (result <0 ) {
		fprintf(stderr,"Error binding! %s\n", strerror(errno));
	}

	/* Tell the server we want to listen on the port */
	/* Second argument is backlog, how many pending connections can */
	/* build up.  We arbitrarily pick 5 */
	listen(socket_fd,5);

	/* Call accept to create a new file descriptor for an incoming */
	/* connection.  It takes the oldest one off the queue */
	/* We're blocking so it waits here until a connection happens */
	client_len=sizeof(client_addr);
	new_socket_fd = accept(socket_fd,
		(struct sockaddr *)&client_addr,&client_len);
	if (new_socket_fd<0) {
		fprintf(stderr,"Error accepting! %s\n",strerror(errno));
	}
							//  "network to host short" converts port number from network order to host- little endian

	printf("Client connected from %s:%d\n",
       inet_ntoa(client_addr.sin_addr),	//  use inet_ntop to convert from binary to string for printing
       ntohs(client_addr.sin_port));	//  "network to host short" converts port number from network order to host- little endian

	// Set up Battlehip before while loop
	char player1Grid[GRID_SIZE][GRID_SIZE];
    char player2Grid[GRID_SIZE][GRID_SIZE];

	initGrid(player1Grid);	// Init empty grids
	initGrid(player2Grid);
	 
	placeShips(player1Grid, 1);
	// We want to change player 2 to get placeships info from buffer- should be ok
	// to just copy the first memset/message receive code from loop
	printf("Your board: \n");
	printGrid(player1Grid);
	

	int currentPlayer = 1;
    int gameFinished = 0;
	int row = 1;
	int col = 1;
	int hit =0;

	// Get the player 2 grid from client
	memset(buffer,0,BUFFER_SIZE);
	n = read(new_socket_fd,buffer,(BUFFER_SIZE-1));
	if (n==0) {
			fprintf(stderr,"Connection to client lost (Grid getting)\n\n");
	}
	else if (n<0) {
		fprintf(stderr,"Error reading from socket (Grid getting) %s\n",
			strerror(errno));
	}
	memcpy(player2Grid, buffer, sizeof(char) * GRID_SIZE * GRID_SIZE);
	// End grid getting



	
	while(gameFinished!=1){

		// Player 1 goes first: get their input, use the regular battleship functions, print state.
		// then send/read from player 2, receive their input, do calculations, update state. Check gameover, loop again.

		if(currentPlayer == 1){
			printf("---------------------------------------\n");
			printf("Your turn!\n");
			
        	printf("Enter row and column to fire: ");
        	scanf("%d %d", &row, &col);
			hit = checkHit(player2Grid, row, col);

			if (hit == 1) {
            printf("Hit!\n");
			strcpy(buffer, "Player1 scored a Hit!   ");
			} else if (hit == 2) {
				printf("Ship sunk!              \n");
				strcpy(buffer, "Player 1 sunk your ship!");
			} else {
				printf("Miss!\n");
				strcpy(buffer, "Player1 missed!         ");
			}
			//printGrid(player2Grid);
			// Game over check on player 2 board if player is p1, else on player1board
			gameFinished = gameOver(currentPlayer == 1 ? player2Grid : player1Grid);
			currentPlayer = currentPlayer == 1 ? 2 : 1; // Switch current player

			//memset(buffer, '\0', sizeof(buffer));
			n = write(new_socket_fd, buffer, strlen(buffer)); 
			if (n<0) {
				fprintf(stderr,"Error writing. %s\n",
					strerror(errno));
			}


		}
		else{	// Player 2 turn- need to do networking
			printf("---------------------------------------\n");
			printf("Player 2 turn- getting their move...\n");
			memset(buffer,0,BUFFER_SIZE);
			n = read(new_socket_fd,intBuffer,sizeof(intBuffer));
			if (n==0) {
				fprintf(stderr,"Connection to client lost\n\n");
			}
			else if (n<0) {
				fprintf(stderr,"Error reading from socket %s\n",
					strerror(errno));
			}
			printf("Guess received: %d %d\n", intBuffer[0], intBuffer[1]);
			hit = checkHit(player1Grid, intBuffer[0], intBuffer[1]);
			if (hit == 1) {
            printf("Player 2 scored a Hit!\n");
			strcpy(buffer, "You scored a Hit!");
			} else if (hit == 2) {
				printf("Your Ship sunk!\n");
				strcpy(buffer, "You sunk P1 ship!");
			} else {
				printf("Player 2 Missed!\n");
				strcpy(buffer, "You missed!");
			}

			n = write(new_socket_fd, buffer, strlen(buffer));  
			if (n<0) {
				fprintf(stderr,"Error writing. %s\n",
					strerror(errno));
			}
			gameFinished = gameOver(currentPlayer == 2 ? player1Grid : player2Grid);
			currentPlayer = 1; // Switch current player
		}
		if(gameFinished == 1){
			printf("Game Over: Player %d wins!\n", currentPlayer == 1 ? 2 : 1);
			break;
		}

		

	}

	printf("Exiting server\n\n");
	/* Close the sockets */
	close(new_socket_fd);
	close(socket_fd);
	return 0;
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

int checkHit(char grid[][GRID_SIZE], int row, int col) {
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

int gameOver(char grid[][GRID_SIZE]) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] == 'S') {
                return 0;
            }
        }
    }
    return 1;
}
