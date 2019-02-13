#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include "potato.h"




int main(int argc, char * argv []){

	if(argc != 3){
		printf("Invalid command arguments. Kindly check your input.\n");
		exit(FAILURE);
	}

   /* The gethostbyname() function returns a structure of type hostent for
      the given host name.  Here name is either a hostname or an IPv4
   	  address in standard dot notation (as for inet_addr(3)). */

	struct hostent * ringmaster_details = gethostbyname(argv[1]);
	if(ringmaster_details == NULL){
		printf("Host not found.");
		exit(FAILURE);
	}

	int flag = 0;
	int j = 0;
	while( j < strlen(argv[2])){
  		flag = isdigit(argv[2][j]);
  		if (flag == 0){
  			printf("Port should be a number. Check input.");
  			exit(FAILURE);
  		}
  		j++;
  	}
	
	int ringmaster_port = atoi(argv[2]);

	//Connect to Ring-Master


	struct addrinfo host_details;
	struct addrinfo* host_details_list;
	memset(&host_details, 0, sizeof(host_details));
	host_details.ai_socktype = SOCK_STREAM;
	host_details.ai_family   = AF_UNSPEC;

	/* getaddrinfo() returns 0 if it succeeds, or one of the following
    nonzero error codes */

    int res = getaddrinfo(argv[1], argv[2], &host_details, &host_details_list);


	
	

}