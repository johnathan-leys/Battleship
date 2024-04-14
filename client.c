/* ECE435 Homework #1 Socket Client Code */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <sys/socket.h>

#include <netdb.h>

/* Default port.  Must be from 1024 to 65536 for normal user */
#define DEFAULT_PORT	31337

#define BUFFER_SIZE	256

/* Default hostname */
#define DEFAULT_HOSTNAME	"localhost"

// Battleship setup
#define GRID_SIZE 10

void placeShips(char grid[][GRID_SIZE], int player);
void initGrid(char grid[][GRID_SIZE]);
void printGrid(char grid[][GRID_SIZE]);

int socket_fd;

// Signal handler function, to handle unexpected closing
void sigint_handler(int signo) {
    if (signo == SIGINT) {
        // Send "bye" message to the server before exiting
        write(socket_fd, "bye", strlen("bye"));
        printf("\nInterrupt. Closing...\n");
        close(socket_fd);
        exit(0);
    }
}

int main(int argc, char **argv) {

	
	int port;
	struct hostent *server;
	struct sockaddr_in server_addr;
	char buffer[BUFFER_SIZE];
	int n;
	int result;

	// Signal handler for SIGINT to handle unexpected disconnect w/ ctr+c
    signal(SIGINT, sigint_handler);

	port=DEFAULT_PORT;

	/* Open a socket file descriptor */
	/* AF_INET means an IP network socket, not a local (AF_UNIX) one */
	/* There are other types you can open too */
	/* SOCK_STREAM means reliable two-way byte stream (TCP) */
	/* last argument is protocol subset.  We leave at zero */
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd<0) {
		fprintf(stderr,"Error socket: %s\n",
			strerror(errno));
	}

	printf("Connecting to server %s port %d\n",
		DEFAULT_HOSTNAME,port);

	/* Look up the server info based on its name */
	server=gethostbyname(DEFAULT_HOSTNAME);
	if (server==NULL) {
		fprintf(stderr,"ERROR!  No such host!\n");
		exit(0);
	}

	/* clear out the server_addr structure */
	memset(&server_addr,0,sizeof(server_addr));

	/* Copy in the destination address we got from the previous */
	/* gethostbyname() call */
	server_addr.sin_family=AF_INET;
	memcpy(server->h_addr,
		&server_addr.sin_addr.s_addr,
		server->h_length);

	/* port should be in "network byte order" (big-endian) so convert */
	/* htons = host to network [byte order] short (16-bit) */
	server_addr.sin_port=htons(port);

	/* Call the connect system call to actually connect to server */
	result = connect(socket_fd,
			(struct sockaddr *) &server_addr,
			sizeof(server_addr));
	if (result < 0) {
		fprintf(stderr,"Error connecting! %s\n",
			strerror(errno));
	}

	// Set up battleship stuff
	char player2Grid[GRID_SIZE][GRID_SIZE];
	char singleDimGrid[GRID_SIZE * GRID_SIZE];

	initGrid(player2Grid);
	placeShips(player2Grid, 2);
	printGrid(player2Grid);
	// From here, grid is set. Convert to 1d buffer, sent via network.
	memcpy(singleDimGrid, player2Grid, sizeof(char) * GRID_SIZE * GRID_SIZE);

	printf("Sending Grid to server: ");

	memset(buffer,0,BUFFER_SIZE);
	fgets(buffer,BUFFER_SIZE-1,stdin);

	n = write(socket_fd,singleDimGrid,strlen(singleDimGrid));
		if (n<0) {
			fprintf(stderr,"Error writing socket! %s\n",
				strerror(errno));
		}


	/*
	In the loop, we want to :
	Get whos turn it is:
	if player2 use fgets to ask player for coordinates as astring:
	Send that string to the server
	Server does logic for checking
	Server sends hit or miss message
	Print hit or miss message
	Server send grid state back to client
	Other players turn, indicate that, print their results
	
	*/
	/****************************************/
	/* Main client loop 			*/
	/****************************************/
	while(1){

		/* Prompt for a message */
		printf("Please enter a message to send: ");

		/* Read NUL terminated string from standard input */
		/* (your keyboard most likely) */
		memset(buffer,0,BUFFER_SIZE);
		fgets(buffer,BUFFER_SIZE-1,stdin);

		/* Write to socket using the "write" system call */
		n = write(socket_fd,buffer,strlen(buffer));
		if (n<0) {
			fprintf(stderr,"Error writing socket! %s\n",
				strerror(errno));
		}

		/* Clear buffer and read the response from the server */
		memset(buffer,0,BUFFER_SIZE);
		n = read(socket_fd,buffer,BUFFER_SIZE-1);
		if (n<0) {
			fprintf(stderr,"Error reading socket! %s\n",
				strerror(errno));
		}

		/* Print the response we got */
		printf("Received back from server: %s\n\n",buffer);
		if(strncmp(buffer, "BYE\n", 4) == 0){ //  check for exit command
			printf("Termination string detected, closing...\n");
			close(socket_fd);
			printf("Closed\n");
			return 0;
			//break;
		}
	}
	/* All finished, close the socket/file descriptor */
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