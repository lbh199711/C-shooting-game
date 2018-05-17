#ifndef _utils_h
#define _utils_h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct player_info{
	// type = 0 is fire type = 1 is player
	int type;
	int client_num;
	//facing: 0, up, 1, down, 2, left, 3, right
	int facing;
	int x;
	int y;
	// 0 is fire, 1 is up, 2 is down, 3 is left, 4 is right
	int action;
};

#endif
