# C-shooting-game
An online game with simple GUI that has both server and client-end made using socket, pthread API. 

## TO RUN
Run makefile firse to compile</br>
Server: ./server [boardSize] [updateInterval] [rngSeed]</br>
client: ./client 2224 [serverIp]

## TO GO ONLINE(local network)
Change server.c, ln 301, master.sin_addr.s_addr = inet_addr("127.0.0.1"); to  master.sin_addr.s_addr = inet_addr("[serverIp]");

## General
Part worked together: communication between sever and client</br>
</br>
Leo Liu(Client): Implemented the client side multi-thread structure. </br>
Client side board calculation and output.</br>
</br>
</br> 
Yi Zhang(Server): implemented multi-thread structure to listen to client and to manage the manipulation of data</br>

!!!CLIENT CONTROL!!!:</br>
</br>
i -move up</br>
j -move left</br>
k -move down</br>
l -move right</br>
space -fire</br>
q -quit game</br>
</br>
!!!SERVER CONTROL!!!:</br>
</br>
ENTER -quit(so be careful don't hit enter if you don't want to stop the game)</br>
</br>
</br>
known bug:</br>
- if client quit the game not by pressing 'q' and game is not over, the server crushes and have to be restarted.</br>
- if server stops, the client will not be notified and the screen will freeze for 1 sec and go back to command line without user pressing 'q'.</br>

