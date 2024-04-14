/* ECE435 Homework #1 socket server code */

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

int main(int argc, char **argv) {

	int socket_fd,new_socket_fd;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;			// changed from sockaddr for something cool
	int port=DEFAULT_PORT;
	int n,on=1;
	socklen_t client_len;
	char buffer[BUFFER_SIZE];

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

	/* 
	SOMETHING COOL: Print client address and port
	This was more difficult to do until i without changing the type of client_addr from sockaddr to sockaddr_in.
	Before that, I tried importing some stuff and typecasting.
	*/
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
	
	// Ideally we block here until receive grid setup from player2

	int currentPlayer = 1;
    int gameFinished = 0;

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

	/* Print the message we received */
	printf("Grid from client (DO NOT KEEP IN FINAL): \n");
	printGrid(player2Grid);				// This is cheating, get rid of when done testing...

	
	while(gameFinished!=1){


		/* Someone connected!  Let's try to read BUFFER_SIZE-1 bytes */
		memset(buffer,0,BUFFER_SIZE);
		n = read(new_socket_fd,buffer,(BUFFER_SIZE-1));
		if (n==0) {
			fprintf(stderr,"Connection to client lost\n\n");
		}
		else if (n<0) {
			fprintf(stderr,"Error reading from socket %s\n",
				strerror(errno));
		}

		/* Print the message we received */
		printf("Message from client: %s\n",buffer);

		for (int i = 0; i < n; i++) {
            buffer[i] = toupper(buffer[i]);
        }
		/* Send a response */
		n = write(new_socket_fd, buffer, strlen(buffer));  //  Updated to echo back the message: use strlen rather than constant to avoid sending entire buffer or truncating
		if (n<0) {
			fprintf(stderr,"Error writing. %s\n",
				strerror(errno));
		}	

		if(strncmp(buffer, "BYE\n", 4) == 0){ //  check for exit command
			printf("Termination string received: Exiting server\n\n");
			/* Close the sockets */
			close(new_socket_fd);
			close(socket_fd);
			return 0;
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