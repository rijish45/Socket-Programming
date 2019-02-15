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

#define PORTBEGIN 51097
#define PORTEND 51015

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
 

 if (atoi(argv[1]) < 1024 || atoi(argv[1]) > 65535) {
    printf("Please input valid port number above 1024 and below 65535.\n");
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
	int player_portnum;


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
           if (status != 0) {
   				 perror("getaddrinfo()");
                 return EXIT_FAILURE;
  			}
           
          //Open the socket to the ringmaster
          int socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
          if(socket_fd == -1){
          	printf("Error: cannot create socket.\n");
    		return EXIT_FAILURE;
          }

          //start connection
          status = connect(socket_fd, host_info_list->ai_addr,host_info_list->ai_addrlen);
          if(status < 0){
          	printf("Cannot connect");
          	return EXIT_FAILURE;
          }


       //Create a socket for the player 
       char playername[64];
       gethostname(playername, sizeof(playername));
       struct hostent * player_detail = gethostbyname(playername);

       int player_socketfd = socket(AF_INET, SOCK_STREAM, 0);
       if(player_socketfd < 0){
       	printf("Error: Cannot create socket.\n");
       	return EXIT_FAILURE;
       }

       struct sockaddr_in player_socket;
       player_socket.sin_family = AF_INET;

       for(int i = PORTBEGIN; i <= PORTEND; i++ ){
       		player_socket_detail.sin_port = htons(i);
       		memcpy(&player_socket_detail.sin_addr, player_detail->h_addr_list[0], player_detail->h_length);
       		int status = bind(player_socketfd, (struct sockaddr *)&player_socket_detail, sizeof(player_socket_detail));
       		if(!status)
       			break;
       	}

  struct sockaddr_in buff;
  socklen_t len = sizeof(sockaddr_in);
  if (!getsockname(player_socketfd, (struct sockaddr*)&buff, &len)) {
    	player_portnum = ntohs(buff.sin_port);
  }













































	}




}