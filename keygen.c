//keygen: This program creates a key file of specified length
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MIN 65
#define MAX 92
#define SPACE 32

int main(int argc, char *argv[]){
    int i, r;
    int keylength = atoi(argv[1]);    
    //seed random
    srand(time(NULL)); 
    //handling an error, this is the only error i could think of, was improper # of args
    if (argc != 2){
        //print err and exit with a code
        fprintf(stderr, "Wrong Number of Arguments");
        exit(1);
    }
    for(i = 0; i < keylength; i++){
        //picks a number between A-Z in ascii table, except 91, which we will pretend is a space
        //derived from https://stackoverflow.com/questions/18117582/picking-rand-number-from-1-6
        r = (rand()%(MAX-MIN))+MIN;
        if(r == 91){
            //print spacespace
            putchar(SPACE);
        }
        //you can just print ints as characters
        else {
            putchar(r);
        }
    }
    //add the endline
    fprintf(stdout, "\n");
    return 0;
}