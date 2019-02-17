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


  int flag = 0;
  int j = 0;
	while( j < strlen(argv[2])){
  		flag = isdigit(argv[2][j]);
  		if (flag == 0){
  			printf("Port should be a number\n");
  			return EXIT_FAILURE;
  		}
  		j++;
  	}
 

 if (atoi(argv[2]) < 1024 || atoi(argv[2]) > 65535) {
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
  int id;
	int player_portnum;
  int num_of_hops;
  int num_of_players;
  int socket_fd;
  int player_socketfd;
  int neighbour_port;
  char neighbour_hostname[64];
  memset(neighbour_hostname, "\0", 64);
  int right_neighbour_sfd;
  int left_neighbour_sfd;


 


	if(!error){

         
      struct hostent * ring_master;
      struct hostent * neighbour_detail;
      struct addrinfo host_info;
      struct addrinfo * host_info_list;
      struct sockaddr_in player_socket_detail;
      struct hostent * player_detail;
      struct sockaddr_in buff;
      socklen_t len = sizeof(buff);

		  //Get ring-master information
		  ring_master = gethostbyname(argv[1]);
		  if(!ring_master){
		  	printf("Ring Master: host not found\n");
		  	EXIT_FAILURE;
		  }
		  else{
		  	ring_master_port = atoi(argv[2]);
		  }

		  

		  memset(&host_info, 0, sizeof(host_info));
		  host_info.ai_family   = AF_UNSPEC;
      host_info.ai_socktype = SOCK_STREAM;
          
           int status = getaddrinfo(argv[1], argv[2], &host_info, &host_info_list);
           if (status != 0) {
   				 perror("getaddrinfo()");
                 return EXIT_FAILURE;
  			}
           
          //Open the socket to the ringmaster
           socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
          if(socket_fd == -1){
          	printf("Error: cannot create socket.\n");
    		return EXIT_FAILURE;
          }

          //start connection
          status = connect(socket_fd, host_info_list->ai_addr,host_info_list->ai_addrlen);
          if(status < 0){
          	printf("Cannot connect\n");
          	return EXIT_FAILURE;
          }


       //Create a socket for the player 
       char playername[64];
       gethostname(playername, sizeof(playername));
       player_detail = gethostbyname(playername);

       player_socketfd = socket(AF_INET, SOCK_STREAM, 0);
       if(player_socketfd < 0){
       	printf("Error: Cannot create socket.\n");
       	return EXIT_FAILURE;
       }

  
       player_socket_detail.sin_family = AF_INET;
      
      for(int i = PORTBEGIN; i <= PORTEND; i++ ){
       		player_socket_detail.sin_port = htons(i);
       		memcpy(&player_socket_detail.sin_addr, player_detail->h_addr_list[0], player_detail->h_length);
       		int status = bind(player_socketfd, (struct sockaddr *)&player_socket_detail, sizeof(player_socket_detail));
       		if(!status)
       			break;
          else if(status < 0){
            printf("Error.\n");
            return EXIT_FAILURE;
          }
       	}

  
  if (!getsockname(player_socketfd, (struct sockaddr*)&buff, &len)) {
    	player_portnum = ntohs(buff.sin_port);
  }
  else{
    printf("Error: getsockname\n");
    return EXIT_FAILURE;
  }


  send(socket_fd, (char*)&player_portnum, sizeof(int), 0);
  recv(socket_fd, (char *)&num_of_players, sizeof(int), 0);
  recv(socket_fd, (char*)&num_of_hops, sizeof(int), 0);
  recv(socket_fd, (char*)&id, sizeof(int), 0);

  //Done receiving and sending basic information
  printf("Connected as player %d\n", id);
  recv(socket_fd, (char*)&neighbour_port, sizeof(int), 0);
  recv(socket_fd, (char*)&neighbour_hostname, 64, 0);
  //Done receiving neighbour port and hostname;


  neighbour_detail = gethostbyname(neighbour_hostname);
  if(!neighbour_detail){
    printf("Error getting neighbour info\n");
    return EXIT_FAILURE;
  }

  int status;
  status = listen(player_socketfd, 2);
  if(status != 0){
    printf("Player socket failed to listen to incoming connection.\n");
    return EXIT_FAILURE;
}


  int connection_signal;
  int signal_status = recv(socket_fd, &connection_signal, sizeof(int), 0);

  int ack = 0;
  send(socket_fd, &ack, sizeof(int), 0);

  right_neighbour_sfd = socket(AF_INET, SOCK_STREAM, 0);
  if(right_neighbour_sfd == -1){
    printf("Cannot create a socket to right neighbour\n");
  }

  
  


 

