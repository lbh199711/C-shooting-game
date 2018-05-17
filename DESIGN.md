Server design: Our project used multi-thread structure to manage the system. When server start, the main thread controls the exit of the server. Then main thread start an acception client thread that waits and accepts the connection from client. When a client tries to establish the connection, the acception client thread starts a new thread that exclusivly listen to the new thread. The main thread also starts a manage thread that computs all data received from clients and send the data back to each client. 

Issues and solutions: When we tested socket, we found that accept and receive action stop the excution of the program, then we used multi-thread structure to solved it. 


Client:	The client end is rather simple compare to the server. Each client has 2 threads. One listen to user input and send all useful input to the server,this thread also helps the user to quit the program if wanted to. Another one constantly wait for server to send the updated board and print them to the user. 

Issues and solution: 

	- sending struct over socket is a mess especially when we tried to send an entire list of struct at once. We changed it to sending one struct at a time and have a header message before the actual info to indicate whether there are more packets to come.

	- There is a lot of things going on in this program and it is very difficult to debug but thankfully we decide to use version control very early on, and instead of doing everything at once we build the program bit by bit.
	
