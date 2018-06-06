//encoding and socket daemon. a lot is taken from server.c
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h> 
#include <unistd.h>
//for buffer use, taken from TLPI ch 60.3
#define BUF_SIZE 4096
//for range of ascii and space
#define MIN 65
#define MAX 92
#define SPACE 32

static void handleRequest(int);

int main(int argc, char* argv[]){

	//from server.c
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	int running = 1;
	int* yes =1;
	socklen_t sizeOfClientInfo;
	char buffer[256];
	struct sockaddr_in serverAddress, clientAddress;
	spawnPid_t spawnPid;
	
	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args
	setsockopt(listenSocketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) {
		error("ERROR opening socket");	
		exit(1);
	}

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {// Connect socket to port
		error("ERROR on binding");
		exit(1);
	}

	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	while(running) {
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) {
			error("ERROR on accept");
			exit(1);
		}
        spawnPid = fork();
		switch (spawnPid){
			
			case -1:
            	error("ERROR on fork");
            	exit(1);
            case 0:
				memset(buffer, '\0', 256);
            	read(listenSocketFD, buffer, sizeof(buffer));
            	////DO MORE STUFF HERE KEEP LOOPING TIL WE READ IT ALL
            	while ( data > 0) {
					// Get the message from the client and display it
					charsRead = recv(establishedConnectionFD, buffer, 255, 0); // Read the client's message from the socket
					if (charsRead < 0) error("ERROR reading from socket");
					else if (charsRead == 0) break;

					// Send a Success message back to the client
					charsRead = send(establishedConnectionFD, "I am the server, and I got your message", 39, 0); // Send success back
					if (charsRead < 0) error("ERROR writing to socket");
					close(establishedConnectionFD); // Close the existing socket which is connected to the client

            	}
            	encrypt(text, key, strlen(text)); //Encrypt it using earlier function

        }
	}	
	close(listenSocketFD); // Close the listening socket
	return 0; 
}


void handleRequest(int cfd)
{
	char buf[BUF_SIZE];
 	ssize_t numRead;
 	while ((numRead = read(cfd, buf, BUF_SIZE)) > 0) {
		if (write(cfd, buf, numRead) != numRead) {
			syslog(LOG_ERR, "write() failed: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	if (numRead == -1) {
		syslog(LOG_ERR, "Error from read(): %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
}



// So in OTP_ENC_D I am calling accept(), and when one comes in I fork() and do process stuff.
// My OTP_ENC_D is handling multiple encryptions via multiple processes.