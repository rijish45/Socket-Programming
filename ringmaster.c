
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <sys/types.h>
#include "potato.h"
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>


//Input Error Checking
int check_error_input(char * argv[]){


for(int i = 1; i <= 3; i++){
	int flag = 0;
	int j = 0;
	while( j < strlen(argv[i])){
  		flag = isdigit(argv[i][j]);
  		if (flag == 0){
  			printf("Inputs should be numbers. Check input.\n");
  			return EXIT_FAILURE;
  		}
  		j++;
  	}
  }

 if (atoi(argv[1]) < 1024) {
    printf("Please input valid port number above 1024.\n");
    return EXIT_FAILURE;
  }

 if(atoi(argv[3]) < 0 || atoi(argv[3]) > 512){
 	printf("Number of hops should be between 0 and 512.\n");
 	return EXIT_FAILURE;
 }

return 0;

}

int main(int argc, char * argv[]){

if(argc != 4){
		printf("Correct format is : ringmaster <port_num> <num_players> <num_hops>\n");
    	return EXIT_FAILURE;
}

check_error_input(argv);
int port_num = atoi(argv[1]);
int num_of_players = atoi(argv[2]);
int num_of_hops = atoi(argv[3]);





}
