#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <pthread.h> 
#include <string.h>
#include <ncurses.h>
#include "utils.h"

/* ---------------------------------------------------------------------
   This is a sample client program for the number server. The client and
   the server need to run on the same machine.
   --------------------------------------------------------------------- */

int MAX_PLAYERS;



char * create_board(int board_size){
    //create a list repr NxN cells of a board 
    char *board = malloc(board_size*board_size);
    memset(board,' ',board_size*board_size);
    return board;
}

void draw_board(char * board,int board_size,int x,int y){
    //draw the board
    clear();
    for (int i= 0; i<board_size+2;i++) printw(" -");
    printw(" \n");
    for (int i = 0;i<board_size;i++){
        printw(" |");
        for (int j=0;j<board_size;j++){
            if (j==x && i==y) attron(A_BOLD);
            printw(" %c",*(board+i*board_size+j));
            if (j==x && i==y) attroff(A_BOLD); 
        }
        printw(" |\n");
    }
    for (int i= 0; i<board_size+2;i++) printw(" -");
    printw("\n");
    refresh();
    
}

void change_board(int x_pos,int y_pos,int facing,int board_size, char * board){
    char c;
    if (facing==0)
        c = '^';
    else if (facing==1)
        c = 'v';
    else if (facing==2)
        c = '<';
    else if (facing==3)
        c = '>';
    else if (facing==4)//fire
        c = 'o';
    //c indicate what will be changed
    
    *(board+y_pos*board_size+x_pos) = c; 
}




void * action_thread(void * arg){
    // this thread listens for action and send them to server
    int * s = arg;
    char action;
    char msg[] = "exit";
    
    while(1){
        action = getchar();
        
        if (action == 'q') break;
        if (action=='j'||action=='i'||action=='k'||action=='l'||action==' ') send(*s,&action,1,0);  
    }

    send(*s,msg,4,0);
    endwin();
    exit(0);
}
int main(int argc,char * argv[])
{
    //check input
    if (argc < 3){
        printf("input error\n");
        return -1;
    }
	int	s, number;
    
	struct	sockaddr_in	server;
	struct	hostent		*host;
    long my_port = strtol(argv[1],NULL,10);
    char ip[15];
    int gameMsg[2]= {0,0};//[playerNum,board_size]
    sprintf(ip,"%s",argv[2]);
    
	host = gethostbyname ("localhost");

	if (host == NULL) {
		perror ("Client: cannot get host description");
		exit (1);
	}


	s = socket (AF_INET, SOCK_STREAM, 0);

	if (s < 0) {
		perror ("Client: cannot open socket");
		exit (1);
	}

	bzero (&server, sizeof (server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ip);
	server.sin_port = htons (my_port);

	if (connect (s, (struct sockaddr*) & server, sizeof (server))) {
		perror ("Client: cannot connect to server");
		exit (1);
	}
    
    
    //recv game msg if the game is full
    recv(s,gameMsg,2*sizeof(int),0);
    if (gameMsg[0] == -1){
        perror("game full, try again later");
        return 0;
    } 
    //else init
    int game = 1; //game is on
    int playerNum = gameMsg[0];
    int board_size = gameMsg[1];
    int current_x,current_y;
    char * board;
    struct player_info players;
    

    MAX_PLAYERS = board_size * board_size;

    printf ("playerNum: %d\nboard_size: %d\n",gameMsg[0],gameMsg[1]);

    //start GUI(ncurses)
    initscr();
    
    

    //open thread for listening actions
    pthread_t thread;
    pthread_create(&thread, NULL, action_thread,(void *) &s);    
    
   

    //main thread keep checking update
    while (1){
        board = create_board(board_size);
        for (int i = 0; i<MAX_PLAYERS; i++){
            recv(s,&game,sizeof(int),0);//check if there is any pkt to recieve

            if (game == 0) break;//if not break out of the loop and display
            else if (game == 2){
                char msg[] = "exit";
                send(s,msg,4,0);
                int b;
                for (b = 5; b > 0; b--){
                    clear();
                    printw("GAME OVER!(%d)\n", b);
                    refresh();
                    sleep(1);
                }
                endwin();
                exit(0);
            }

            recv(s,&players,sizeof(struct player_info),0);//else recv info pkt

            if (players.client_num != 0){//and change the board
                change_board(players.x,players.y,players.facing,board_size,board);
                
     	    }

            if (players.client_num == playerNum){
                current_x = players.x;
                current_y = players.y;
            }

            if (players.action == 0){//fire 
                if (players.facing==0){//check which side the player is facing
                    change_board(players.x,players.y-1,4,board_size,board);
                    change_board(players.x,players.y-2,4,board_size,board);
                }
                else if (players.facing==1){
                    change_board(players.x,players.y+1,4,board_size,board);
                    change_board(players.x,players.y+2,4,board_size,board);
                }
                else if (players.facing==2){
                    if (players.x > 0)change_board(players.x-1,players.y,4,board_size,board);
                    if (players.x > 1)change_board(players.x-2,players.y,4,board_size,board);
                }
                else if (players.facing==3){
                     if (players.x<(board_size-1))change_board(players.x+1,players.y,4,board_size,board);
                     if (players.x<(board_size-2))change_board(players.x+2,players.y,4,board_size,board);
                } 
            }

        draw_board(board,board_size,current_x,current_y);
        }
    
    }
                   
         
    //join thread
    pthread_join(thread,NULL);
    
}
