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

void encrypt(char[], char[], int);


int main(int argc, char* argv[]){

	//from server.c
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	int running = 1;
	int* yes;
	char* key;
    char text[BUF_SIZE];
	socklen_t sizeOfClientInfo;
	char buffer[256];
	struct sockaddr_in serverAddress, clientAddress;
	pid_t spawnPid;
	
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
        	char * readBuffer = buffer; //im not sure why this is needed but when i added this it worked
        	while (running) {
				// Get the message from the client and display it
				charsRead = recv(establishedConnectionFD, readBuffer, 255, 0); // Read the client's message from the socket
				if (charsRead < 0) {error("ERROR reading from socket"); exit(1);}
				else if (charsRead == 0) break;

				//search for enf of file
				int i;
				for (i=0; i< charsRead; i++) {

					//DO STUFF HERE?
				}

				// Send a Success message back to the client
				charsRead = send(establishedConnectionFD, "I am the server, and I got your message", 39, 0); // Send success back
				if (charsRead < 0) error("ERROR writing to socket");
				close(establishedConnectionFD); // Close the existing socket which is connected to the client

        	}
        	//start with a blank chararray, copy as much as the 
            memset(text, '/0', sizeof(text));
            strncpy(text, buffer, key-buffer);    
           	encrypt(text, key, strlen(text));
            write(establishedConnectionFD, text, sizeof(text));
        }
	}	
	close(listenSocketFD); // Close the listening socket
	return 0; 
}


void encrypt(char text[], char key[], int l) {    //Function prototypes to avoid errors
	int i, plain, encoded, decoderRing;
	for(i=0;i<l-1;i++){
		plain = text[i];
		decoderRing = key[i];
		//change them to our ascii range
		if(plain == SPACE) plain = 91;
		if(decoderRing == SPACE) decoderRing = 91;
		encoded = plain + decoderRing;
		printf("encoded is %d\n", encoded);
		if (encoded > 91) { 
			//subtract by range to get it back down to where it needs to be
			encoded = encoded - (MAX-MIN);
			printf("encoded is changed to %d\n", encoded);
		}
		//THEN change to a space.. this little bug got me for a while
		if (encoded == 91) encoded = SPACE;
	}

}



// So in OTP_ENC_D I am calling accept(), and when one comes in I fork() and do process stuff.
// My OTP_ENC_D is handling multiple encryptions via multiple processes.