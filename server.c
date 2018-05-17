
#define _XOPEN_SOURCE 600

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "utils.h"
#define	MY_PORT	2224
int MAX_PLAYERS;
int fd;
struct player_info *players;
int size;
float interval;
int count;
pthread_mutex_t mutex;


void put_new_player(int client_num){
	// when a client connects to the server, put a new player in the board
	struct player_info player;
	int i = 0;
	int exist = 1;
	int x, y;
	// make sure new x, y not on the existing cell
	while (1){
		x = rand() % size;
		y = rand() % size;
		for (i = 0; i < MAX_PLAYERS; i++){
			if (players[i].client_num != -1 && players[i].x == x && players[i].y == y){
				break;
			}
			if (i == MAX_PLAYERS-1){
				exist = 0;
			}
		}
		if (!exist){
			break;
		}
	}
	player.type = 1;
	player.client_num = client_num;
	player.facing = rand() % 4;
	player.x = x;
	player.y = y;
	player.action = -1;
	for (i = 0; i < MAX_PLAYERS; i++){
		if (players[i].client_num == -1){
			players[i] = player;
			break;
		}
	}
}

void update_player(int client_num, char action[11]){
	// update player's action
	int i;
	for (i = 0; i < MAX_PLAYERS; i++){
		if (players[i].client_num == client_num){
			if (strcmp(action, "k") == 0)
				players[i].action = 2;
			else if (strcmp(action, "l") == 0)
				players[i].action = 4;
			else if (strcmp(action, "i") == 0)
				players[i].action = 1;
			else if (strcmp(action, "j") == 0)
				players[i].action = 3;
			else if (strcmp(action, " ") == 0)
				players[i].action = 0;

		}
	}
}

void clear_players(){
	int i = 0;
    for (i = 0; i < MAX_PLAYERS; i++){
    	struct player_info player;
		player.client_num = -1;
    	players[i] = player;
    }
}

void player_died(int j){
	// scend message to client
	// mark the current player as uninitialized player
	int msg = 2;
	send(players[j].client_num,&msg,sizeof(int),0);
	printf("player %d died\n", players[j].client_num);
	players[j].type = 0;
	players[j].client_num = -1;
	count--;
}

void player_undo(int i){
	// if two player try to enter a same block, undo both of them
	switch (players[i].action){
		case 2:
			players[i].y -= 1;
			break;
		case 1:
			players[i].y += 1;
			break;
		case 3: 
			players[i].x += 1;
			break;
		case 4:
			players[i].x -= 1;
			break;	
		}
}

void * manage_thread(void * arg){
	// this thread do all the computation and send the data to each client
	while (1){

		usleep(interval*1000000);
		//printf("in the manage_thread\n");
		pthread_mutex_lock(&mutex);
		int i;
		int j;

		// update move action
		for (i = 0; i < MAX_PLAYERS; i++){
			if (players[i].client_num != -1){
				switch (players[i].action){

					case 2:
						if (players[i].y != size-1){
							players[i].y += 1;
							
						}
						players[i].facing = 1;
						break;
					case 1:
						if (players[i].y != 0){
							players[i].y --;
							
						} 
						players[i].facing = 0;
						break;
					case 3: 
						if (players[i].x != 0){
							players[i].x -= 1;
							
						} 
						players[i].facing = 2;
						break;
					case 4:
						if (players[i].x != size-1){
							players[i].x += 1;
							
						} 
						players[i].facing = 3;
						break;	
				}
			}
		}

		//resolve conflict
		for (i = 0; i < MAX_PLAYERS; i++){
			for (j = 0; j < MAX_PLAYERS; j++){
				if ( i != j && players[i].client_num != -1 && players[j].client_num != -1 && players[i].x == players[j].x && players[i].y == players[j].y){
					//printf("1y is %d\n", players[i].y);
					//there's a conflict move each players back
					player_undo(i);
					player_undo(j);
					break;
				}
			}
		} 
		int msg = 2;
		//update fire action
		for (i = 0; i < MAX_PLAYERS; i++){
			if (players[i].client_num != -1 && players[i].action == 0){
				//printf("2y is %d\n", players[i].y);
				switch (players[i].facing){
					//up
					case 0:
						for (j = 0; j < MAX_PLAYERS; j++){
							if (players[j].client_num != -1 && players[j].x == players[i].x && (players[j].y == players[i].y - 1 || players[j].y == players[i].y - 2)){
								player_died(j);
								break;
							}
						}
						break;
					//down
					case 1:
						for (j = 0; j < MAX_PLAYERS; j++){
							if (players[j].client_num != -1 && players[j].x == players[i].x && (players[j].y == players[i].y + 1 || players[j].y == players[i].y + 2)){
								player_died(j);
								break;
							}
						}
						break;
					//left
					case 2:
						for (j = 0; j < MAX_PLAYERS; j++){
							if (players[j].client_num != -1 && (players[j].x == players[i].x - 1 || players[j].x == players[i].x - 2) && players[j].y == players[i].y){
								player_died(j);
								break;
							}
						}
						break;
					//right
					case 3:
						for (j = 0; j < MAX_PLAYERS; j++){
							if (players[j].client_num != -1 && (players[j].x == players[i].x + 1 || players[j].x == players[i].x + 2) && players[j].y == players[i].y){
								player_died(j);
								break;
							}
						}

						break;
				}
			}
		}

		// scend the board to client
        
        for (int j =0; j<MAX_PLAYERS;j++){
            if (players[j].client_num != -1){
                //printf("this is %d\n",j);
		        for (i = 0; i<MAX_PLAYERS; i++){
	            	if (players[i].client_num != -1){
                        
                        //printf("send to: %d\n", players[j].client_num);
                        msg = 1;//more packet to recv
	            		//printf("msg: %d\n", msg);
       
                        send(players[j].client_num,&msg,sizeof(int),0);//send system msg & player info
                        send(players[j].client_num, &players[i], sizeof(struct player_info),0);
                    }   
            	}
                msg = 0;//end transmission
                //printf("msg 0\n");
                send(players[j].client_num,&msg,sizeof(int),0);
            }
        }


		// clear the action
		for (i = 0; i < MAX_PLAYERS; i++){
			players[i].action = -1;
		}
		pthread_mutex_unlock(&mutex);

	}//end while
	return (void *)0;
}

void * listen_thread(void * arg){
	// this client only listen to the client that is assigned to 
    int * temp = (int *) arg;
    int threadNum = *temp;
    printf("client %d connected\n", threadNum);
    char c[11];
    char exit[11] = "exit";
    while (strcmp(c, exit) != 0){
	    recv(threadNum,c,11,0);
	    int i;
	    sscanf(c, "%d", &i);
		pthread_mutex_lock(&mutex);
		update_player(threadNum, c);
		pthread_mutex_unlock(&mutex);
		
	}
	
	printf("client %d disconnected\n", threadNum);
	int i;
	for (i = 0; i < MAX_PLAYERS; i++){
		if (players[i].client_num == threadNum){
			players[i].client_num = -1;
			players[i].type = 0;
			count--;
			break;
		}
	}
    return (void *)0;
}

void * accept_thread(void * arg){
	// this thread wait for client to connect to the server
	int	sock, snew, fromlength, number, outnum;

	struct	sockaddr_in	master, from;

	sock = socket (AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror ("Server: cannot open master socket");
		exit (1);
	}

	master.sin_family = AF_INET;
	master.sin_addr.s_addr = inet_addr("127.0.0.1");
	master.sin_port = htons (MY_PORT);

	if (bind (sock, (struct sockaddr*) &master, sizeof (master))) {
		perror ("Server: cannot bind master socket");
		exit (1);
	}

	listen (sock, 5);
	fromlength = sizeof (from);
	int info[2];
    int msg;//system msg to client
	while (1){
		snew = accept (sock, (struct sockaddr*) & from, & fromlength);
		// start a new thread for the new client
		if (snew < 0) {
			perror ("Server: accept failed");
			exit (1);
		}
		
		if (count < MAX_PLAYERS){
			pthread_t thread;
			put_new_player(snew);
		    pthread_create(&thread, NULL, listen_thread, (void *) &snew);
		    // info[0] is the client number, info[1] is board size
		    info[0] = snew;
		    info[1] = size;
		    send(snew, info,sizeof(info),0);

		    
		    count ++;
		}
		else{
			// info[0] = -1 if reached max players
			info[0] = -1;
		    info[1] = size;
		    send(snew, info,sizeof(info),0);
		}
		
	    
	}
}

int main(int argc, char * argv[])
{
	if (argc < 4){
        printf("input error\n");
        return -1;
    }
	size = atoi(argv[1]);
	MAX_PLAYERS = size*size;
	players = malloc(sizeof(struct player_info)*MAX_PLAYERS);
    clear_players();
	interval = atof(argv[2]);
	srand(atoi(argv[3]));
	pthread_t thread;
	pthread_mutex_init(&mutex,NULL);
    fd = open("thread_file",O_RDWR | O_CREAT, S_IRWXU);
    int i = 0;
    // start the thread wait for client to connect
    pthread_create(&thread, NULL, accept_thread, (void *) &i);
    // strat the thread that manage the data
    pthread_create(&thread, NULL, manage_thread, (void *) &i);
    printf("Server is working, hit enter to quit\n");
    char a;
    scanf("%c",&a);
    free(players);
    pthread_mutex_destroy(&mutex);
    close(fd);

}
