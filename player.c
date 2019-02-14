#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
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



	while( j < strlen(argv[2])){
  		flag = isdigit(argv[2][j]);
  		if (flag == 0){
  			printf("Port should be a number\n");
  			return EXIT_FAILURE;
  		}
  		j++;
  	}
 

 if (atoi(argv[2]) < 1024) {
    printf("Please input valid port number above 1024.\n");
    return EXIT_FAILURE;
  }


return 0;

}

//Based on TCP code provided in class

int main(int argc, char * argv[]){

	if(argc != 3){
		printf("Correct format is : player <machine_name> <port_num>\n");
        return EXIT_FAILURE;
	}

	int error = check_error_input(argv);
	int ring_master_port;


	if(!error){

			//Get ring-master information

		  struct hostent * ring_master = gethostbyname(argv[1]);
		  if(!ring_master){
		  	printf("Ring Master: host not found");
		  	EXIT_FAILURE;
		  }
		  else{
		  	ring_master_port = argv[2];
		  }

		  struct addrinfo host_info;
		  struct addrinfo * host_info_list;

		  memset(&host_info, 0, sizeof(host_info));
		  host_info.ai_family   = AF_UNSPEC;
          host_info.ai_socktype = SOCK_STREAM;
          int status = getaddrinfo(argv[1], argv[2], &host_info, &host_info_list);
           
          //Open the socket to the ringmaster
          int ring_master_sfd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
          if(ring_master_sfd == -1){
          	printf("Error: cannot create socket.\n");
    		return EXIT_FAILURE;
          }






































	}




}