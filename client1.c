
/* A simple client program for server.c

   To compile: gcc client.c -o client -lsocket -lnsl
   				      (-l links required on csse Unix machines)	

   To run: start the server, then the client */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#ifdef __unix__
#include <unistd.h>
#elif defined _WIN32
#include <windows.h>
#define sleep(x) Sleep(1000 * x)
#endif

	/* number of columns in the game */
#define WIDTH		7

	/* number of slots in each column */
#define HEIGHT		6

	/* number in row required for victory */
#define STRAIGHT	4

	/* sign that a cell is still empty */
#define EMPTY		' '

	/* the two colours used in the game */
#define RED		'R'
#define YELLOW		'Y'

	/* horizontal size of each cell in the display grid */
#define WGRID	5

	/* vertical size of each cell in the display grid */
#define HGRID	3

#define RSEED	876545678

#define LEN 256

typedef char c4_t[HEIGHT][WIDTH];

void print_config(c4_t);
void init_empty(c4_t);
int do_move(c4_t, int, char);
void undo_move(c4_t, int);
int get_move(c4_t,char*,int);
int move_possible(c4_t);
char winner_found(c4_t);
int rowformed(c4_t,  int r, int c);
int explore(c4_t, int r_fix, int c_fix, int r_off, int c_off);
int suggest_move(c4_t board, char colour);
void qread(int newsockfd,char* buffer, int len);
void qwrite(int newsockfd,char* buffer);


int main(int argc, char**argv)
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[256];

	if (argc < 3) 
	{
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}

	portno = atoi(argv[2]);

	
	/* Translate host name into peer's IP address ;
	 * This is name translation service by the operating system 
	 */
	server = gethostbyname(argv[1]);
	
	if (server == NULL) 
	{
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	
	/* Building data structures for socket */

	bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;

	bcopy((char *)server->h_addr, 
			(char *)&serv_addr.sin_addr.s_addr,
			server->h_length);

	serv_addr.sin_port = htons(portno);

	/* Create TCP socket -- active open 
	* Preliminary steps: Setup: creation of active open socket
	*/
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sockfd < 0) 
	{
		perror("ERROR opening socket");
		exit(0);
	}
	
	if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
	{
		perror("ERROR connecting");
		exit(0);
	}

	/* Do processing
	*/
	c4_t board;
	int move;
	char *ptr;

	srand(RSEED);
	init_empty(board);
	print_config(board);

	while ((move = get_move(board,buffer,sockfd)) != EOF) {


		if (do_move(board, move, YELLOW)!=1) {
			printf("Panic\n");
			exit(EXIT_FAILURE);
		}
		print_config(board);

		if (winner_found(board) == YELLOW) {
			/* rats, the person beat us! */
			printf("Ok, you beat me, beginner's luck!\n");
			exit(EXIT_SUCCESS);
		}

		if (!move_possible(board)) {
			/* yes, looks like it was */
			printf("An honourable draw\n");
			exit(EXIT_SUCCESS);
		}
		/* otherwise, look for a move from the computer */
		qread(sockfd,buffer,LEN);

		move=strtol(buffer, &ptr, 10);

		printf("Ok, let's see now....");
		sleep(1);
		/* then play the move */
		printf(" I play in column %d\n", move);

		if (do_move(board, move, RED)!=1) {
			printf("Panic\n");
			exit(EXIT_FAILURE);
		}
		print_config(board);

		if (winner_found(board) == RED) {
			/* yes!!! */
			printf("I guess I have your measure!\n");
			exit(EXIT_SUCCESS);
		}

		// printf("Please enter the message: ");

		// bzero(buffer,256);

		// fgets(buffer,255,stdin);

		// n = write(sockfd,buffer,strlen(buffer));

		// if (n < 0) 
		// {
		// 	perror("ERROR writing to socket");
		// 	exit(0);
		// }
	
		// bzero(buffer,256);

		// n = read(sockfd,buffer,255);
	
		// if (n < 0)
		// {
		// 	perror("ERROR reading from socket");
		// 	exit(0);
		// }

		// printf("%s\n",buffer);

		// return 0;
	}
	printf("\n");
}









void qwrite(int newsockfd,char* buffer) {
	int n;

	n = write(newsockfd,buffer,strlen(buffer));
	
	if (n < 0) 
	{
		perror("ERROR writing to socket");
		exit(1);
	}
}


void qread(int newsockfd,char* buffer, int len) {

	int n;

	bzero(buffer,len);

	/* Read characters from the connection,
		then process */
	
	n = read(newsockfd,buffer,len-1);

	if (n < 0) 
	{
		perror("ERROR reading from socket");
		exit(1);
	}
}

/* Initialise the playing array to empty cells */
void
init_empty(c4_t board) {
	int r, c;
	for (r=HEIGHT-1; r>=0; r--) {
		for (c=0; c<WIDTH; c++) {
			board[r][c] = EMPTY;
		}
	}
	printf("Welcome to connect-4 \n\n");
}

/* Apply the specified move to the board
 */
int
do_move(c4_t board, int c, char colour) {
	int r=0;
	/* first, find the next empty slot in that column */
	while ((r<HEIGHT) && (board[r][c-1]!=EMPTY)) {
		r += 1;
	}
	if (r==HEIGHT) {
		/* no move is possible */
		return 0;
	}
	/* otherwise, do the assignment */
	board[r][c-1] = colour;
	return 1;
}

/* Remove the top token from the specified column c
 */
void
undo_move(c4_t board, int c) {
	int r=0;
	/* first, find the next empty slot in that column, but be
	 * careful not to run over the top of the array
	 */
	while ((r<HEIGHT) && board[r][c-1] != EMPTY) {
		r += 1;
	}
	/* then do the assignment, assuming that r>=1 */
	board[r-1][c-1] = EMPTY;
	return;
}

/* Check board to see if it is full or not */
int
move_possible(c4_t board) {
	int c;
	/* check that a move is possible */
	for (c=0; c<WIDTH; c++) {
		if (board[HEIGHT-1][c] == EMPTY) {
			/* this move is possible */
			return 1;
		}
	}
	/* if here and loop is finished, and no move possible */
	return 0;
}


/* Read the next column number, and check for legality 
 */
int
get_move(c4_t board,char* buffer, int newsockfd) {
	int c;
	/* check that a move is possible */
	if (!move_possible(board)) {
		return EOF;
	}
	/* one is, so ask for user input */
	printf("Enter column number: ");
	if (scanf("%d", &c) != 1) {
		return EOF;
	}
	/* and keep asking until a valid move is entered */
	while ((c<=0) || (c>WIDTH) || (board[HEIGHT-1][c-1]!=EMPTY)) {
		printf("That move is not possible. ");
		printf("Enter column number: ");
		if (scanf("%d", &c) != 1) {
			return EOF;
		}
	}
	/* now have a valid move */
	bzero(buffer,256);
	sprintf(buffer,"%d",c);
	// printf("%s",buffer);
	qwrite(newsockfd,buffer);
	return c;
}

/* Print out the current configuration of the board 
 */
void
print_config(c4_t board) {
	int r, c, i, j;
	/* lots of complicated detail in here, mostly this function
	 * is an exercise in attending to detail and working out the
	 * exact layout that is required.
	 */
	printf("\n");
	/* print cells starting from the top, each cell is spread over
	 * several rows
	 */
	for (r=HEIGHT-1; r>=0; r--) {
		for (i=0; i<HGRID; i++) {
			printf("\t|");
			/* next two loops step across one row */
			for (c=0; c<WIDTH; c++) {
				for (j=0; j<WGRID; j++) {
					printf("%c", board[r][c]);
				}
				printf("|");
			}
			printf("\n");
		}
	}
	/* now print the bottom line */
	printf("\t+");
	for (c=0; c<WIDTH; c++) {
		for (j=0; j<WGRID; j++) {
			printf("-");
		}
		printf("+");
	}
	printf("\n");
	/* and the bottom legend */
	printf("\t ");
	for (c=0; c<WIDTH; c++) {
		for (j=0; j<(WGRID-1)/2; j++) {
			printf(" ");
		}
		printf("%1d ", c+1);
		for (j=0; j<WGRID-1-(WGRID-1)/2; j++) {
			printf(" ");
		}
	}
	printf("\n\n");
}

/* Is there a winning position on the current board?
 */
char
winner_found(c4_t board) {
	int r, c;
	/* check exhaustively from every position on the board
	 * to see if there is a winner starting at that position.
	 * could probably short-circuit some of the computatioin,
	 * but hey, the computer has plenty of time to do the tesing
	 */
	for (r=0; r<HEIGHT; r++) {
		for (c=0; c<WIDTH; c++) {
			if ((board[r][c]!=EMPTY) && rowformed(board,r,c)) {
				return board[r][c];
			}
		}
	}
	/* ok, went right through all positions and if we are still
	 * here then there isn't a solution to be found
	 */
	return EMPTY;
}


/* Is there a row in any direction starting at [r][c]?
 */
int
rowformed(c4_t board, int r, int c) {
	return
		explore(board, r, c, +1,  0) ||
		explore(board, r, c, -1,  0) ||
		explore(board, r, c,  0, +1) ||
		explore(board, r, c,  0, -1) ||
		explore(board, r, c, -1, -1) ||
		explore(board, r, c, -1, +1) ||
		explore(board, r, c, +1, -1) ||
		explore(board, r, c, +1, +1);
}

/* Nitty-gritty detail of looking for a set of straight-line
 * items all the same colour. Need to be very careful not to step
 * over the edge of the array
 */
int
explore(c4_t board, int r_fix, int c_fix, int r_off, int c_off) {
	int r_lim, c_lim;
	int r, c, i;
	r_lim = r_fix + (STRAIGHT-1)*r_off;
	c_lim = c_fix + (STRAIGHT-1)*c_off;
	/* can we go in the specified direction?
	 */
	if (r_lim<0 || r_lim>=HEIGHT || c_lim<0 || c_lim>=WIDTH) {
		/* no, not enough space */
		return 0;
	}
	/* can, so check the colours for all the same */
	for (i=1; i<STRAIGHT; i++) {
		r = r_fix + i*r_off;
		c = c_fix + i*c_off;
		if (board[r][c] != board[r_fix][c_fix]) {
			/* found one different, so cannotbe a row */
			return 0;
		}
	}
	/* by now, a straight row all the same colour has been found */
	return 1;
}

/* Try to find a good move for the specified colour
 */
int
suggest_move(c4_t board, char colour) {
	int c;
	/* look for a winning move for colour */
	for (c=0; c<WIDTH; c++) {
		/* temporarily move in column c... */
		if (do_move(board, c+1, colour)) {
			/* ... and check to see the outcome */
			if (winner_found(board) == colour) {
				/* it is good, so unassign and return c */
				undo_move(board, c+1);
				return c+1;
			} else {
				undo_move(board, c+1);
			}
		}
	}
	/* ok, no winning move, look for a blocking move */
	if (colour == RED) {
		colour = YELLOW;
	} else {
		colour = RED;
	}
	for (c=0; c<WIDTH; c++) {
		/* temporarily move in column c... */
		if (do_move(board, c+1, colour)) {
			/* ... and check to see the outcome */
			if (winner_found(board) == colour) {
				/* it is good, so unassign and return c */
				undo_move(board, c+1);
				return c+1;
			} else {
				undo_move(board, c+1);
			}
		}
	}
	/* no moves found? then pick at random... */
	c = rand()%WIDTH;
	while (board[HEIGHT-1][c]!=EMPTY) {
		c = rand()%WIDTH;
	}
	return c+1;
}