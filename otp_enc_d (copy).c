//Cihan Adrian Towery
//P4-OTP
//encoding and socket daemon. a lot is taken from server.c
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h> 
#include <unistd.h>
//for text key and cipher buffer arrays to use, taken and modified from TLPI ch 60.3
#define BUF_SIZE 99999
//for range of ascii and space
#define MIN 65
#define MAX 92
#define SPACE 32
//global process counter and space to transfer cipher text
int authenticate(char text[BUF_SIZE]);
void error( const char *msg ) {perror(msg); exit(1);}
int processCount = 0;
pid_t processes[512]; //to catch and limit processes during debug and to kill at end
char cipher[BUF_SIZE];

void encrypt(char[], char[]);


int main(int argc, char* argv[]){

	//from server.c
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, charsSent;
	int running = 1;
    int one = 1;
	int *yes = &one;
	*yes = 1; //authenticate continuing 
	int i;
	char *authKey = "e";
    char text[BUF_SIZE];
	char key[BUF_SIZE];
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;
	pid_t spawnPid;

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args
	setsockopt(listenSocketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	//// Set up the socket
	//if (listenSocketFD < 0) {
	//	error("ERROR opening socket");	
	//	exit(1);
	//}
	// Set up the address struct for this process (the server)
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process


	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {// Connect socket to port
		error("ERROR on binding");
		exit(1);
	}

	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	while(running) {
		//clear buff
		memset(text, '\0', BUF_SIZE);
		memset(key, '\0', BUF_SIZE);
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
        case 0: //where we do the child stuff
        	////DO MORE STUFF HERE KEEP LOOPING TIL WE READ IT ALL
        	//start with a blank chararray, copy, encrypt, then send
		    memset(text, '\0', BUF_SIZE);
        	//close(listenSocketFD);
			charsRead = recv(establishedConnectionFD, text, BUF_SIZE, 0); // Read the client's message from the socket
			if (charsRead < 0) {error("ERROR on reading"); exit(1);}
			//the transfer of a character for an auth handshake was e

			if(text[0] == authKey[0]){  
            	//printf("received: %s\n", text);
            	send(establishedConnectionFD, authKey, strlen(authKey), 0);
			    text[charsRead-1] = 0;
	            //if(!authenticate(text)) break;;          
	        }
	        else{
	        	//print error like on example
	            fprintf(stderr, "Error: could not contact %s on port %d\n", argv[0], portNumber);
	            *yes=0;
	        }
		    char completeMessage[80000], readBuffer[1000];
			memset(completeMessage, '\0', sizeof(completeMessage)); // Clear the buffer
			while (strstr(completeMessage, "@@") == NULL) // As long as we haven't found the terminal...
			{
				memset(readBuffer, '\0', sizeof(readBuffer)); // Clear the buffer
				int r = recv(establishedConnectionFD, readBuffer, sizeof(readBuffer) - 1, 0); 
				strcat(completeMessage, readBuffer); 
				//printf("PARENT: Message received from child: \"%s\", total: \"%s\"\n", readBuffer, completeMessage);
				if (r == -1) { //printf("r == -1\n"); 
					break; 
				} // Check for errors
				if (r == 0) { //printf("r == 0\n"); 
					break; 
				}
			}
			send(establishedConnectionFD, cipher, strlen(cipher), 0);

			int terminalLocation = strstr(completeMessage, "@@") - completeMessage; 
			completeMessage[terminalLocation] = '\0';




			// printf("PARENT: Complete string: \"%s\"\n", completeMessage);
		    char completeMessage2[80000];
			memset(completeMessage2, '\0', sizeof(completeMessage2)); // Clear the buffer
			while (strstr(completeMessage2, "@@") == NULL) // As long as we haven't found the terminal...
			{
				memset(readBuffer, '\0', sizeof(readBuffer)); // Clear the buffer
				int s = recv(establishedConnectionFD, readBuffer, sizeof(readBuffer) - 1, 0); 
				strcat(completeMessage2, readBuffer); 
				//printf("PARENT: Message received from child: \"%s\", total: \"%s\"\n", readBuffer, completeMessage);
				if (s == -1) { //printf("r == -1\n"); 
					break; 
				} // Check for errors
				if (s == 0) { //printf("r == 0\n"); 
					break; 
				}
			}
			int terminalLocation2 = strstr(completeMessage, "@@") - completeMessage; 
			completeMessage2[terminalLocation2] = '\0';
			printf("KEY SHOULD BE %s\n", completeMessage2);
			encrypt(completeMessage	, completeMessage2);
			send(establishedConnectionFD, cipher, strlen(cipher), 0);

			close(establishedConnectionFD);
            exit(0);



        default:
        	close(establishedConnectionFD);

        }
	}	
	close(listenSocketFD); // Close the listening socket

	return 0; 
}


void encrypt(char text[], char key[]) {    //Function prototypes to avoid errors
    memset(cipher, '\0', sizeof(cipher));
	int i, plain, encoded, decoderRing;
	for(i=0;i<strlen(text);i++){
		plain = text[i];
		//be sure to drink your ovaltine
		decoderRing = key[i];
		//change them to our ascii range
		if(plain == SPACE) plain = 91;
		if(decoderRing == SPACE) decoderRing = 91;
		encoded = plain + decoderRing;
		//printf("encoded is %d\n", encoded);
		if (encoded > 91) { 
			//subtract by range to get it back down to where it needs to be
			encoded = encoded - (MAX-MIN);
			//printf("encoded is changed to %d\n", encoded);
		}
		//THEN change to a space.. 
		if (encoded == 91) encoded = SPACE;
	cipher[i] = encoded;
	}
	cipher[i+1] = '\n'; //end the file with  a newline
	printf("%s\n", cipher);

}

int authenticate(char text[BUF_SIZE]) {
	int i;
	int success = 0;
	for(i=0; i<strlen(text); i++){
        if(text[i] == SPACE || !(text[i] >= MIN && text[i] <= (MAX-2))) {      //bad input
            error("input contains bad characters");
            return success;
        }
    }     
    success = 1;
	return success;
}

// So in OTP_ENC_D I am calling accept(), and when one comes in I fork() and do process stuff.
// My OTP_ENC_D is handling multiple encryptions via multiple processes.
