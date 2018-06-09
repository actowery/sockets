#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#define BUF_SIZE 99999

#define LOCALHOST "localhost"

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[BUF_SIZE];
	char authKey = 'e';
    
	if (argc < 4) { fprintf(stderr,"Improper number of arguments"); exit(0); } // Check usage & args



	// Set up the server address struct
	memset(buffer, '\0', BUF_SIZE);
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname(LOCALHOST); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	// Get input message from user
	printf("CLIENT: authenticating.... ");
	send(socketFD, authKey, sizeof(authKey), 0);
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
	recv(socketFD, buffer, sizeof(buffer), 0); 
	//open the files and compare sizes
    int text = open(argv[1], O_RDONLY);
    int textLength = lseek(text, 0 , SEEK_END);
    int key = open(argv[2], O_RDONLY);
    int keyLength = lseek(key, 0, SEEK_END);

    if (keyLength < textLength) {
    	fprintf(stderr, "Error: key %s is too short", argv[2]);
    	exit(1);
    }
	// Send message to server
	charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");

	// Get return message from server
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
	printf("CLIENT: I received this from the server: \"%s\"\n", buffer);

	close(socketFD); // Close the socket
	return 0;
}