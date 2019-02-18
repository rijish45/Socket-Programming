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

#define PORTBEGIN 51015
#define PORTEND 51097

//Input Error Checking
int check_error_input(char * argv[]){


  int flag = 0;
  int j = 0;
	while( j < strlen(argv[2])){
  		flag = isdigit(argv[2][j]);
  		if (flag == 0){
  			printf("Port should be a number.\n");
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

   //port numbers 
	int ring_master_port;
	int player_portnum;
    int neighbour_port;
  	
  	//necessary variables 
  	int id;
  	int ack = 0;
  	int receive_signal = 0;
  	int status;
	
	//receive information
	int num_of_hops;
	int neighbour_id;
  	int num_of_players;

    //socket file descriptors
  	int socket_fd; //ring master socket-descriptor
  	int player_socketfd;
  	int destination_fd;
  	int right_neighbour_sfd;
  	int left_neighbour_sfd;
  
  	
    char playername[64]; //Store playername
  	char neighbour_hostname[64]; //Store neighbour hostname
  	memset(neighbour_hostname, '\0', 64);
  	
  	POTATO hot_potato;

  	

 


	if(!error){

      //Required struct
      struct sockaddr_in buff;
      struct sockaddr_in neighbour_socket_in; 
      struct sockaddr_in player_socket_detail;
      struct sockaddr_in neighbour_socket_detail;
      socklen_t len = sizeof(buff);

      
      struct hostent * ring_master;
      struct hostent * neighbour_detail;
      struct hostent * player_detail;

      struct addrinfo host_info;
      struct addrinfo * host_info_list;
      
      
     
      

		//Get ring-master information

		  ring_master = gethostbyname(argv[1]); //stores the host information for ringmaster
		  if(!ring_master){
		  	printf("Ring Master: host not found\n");
		  	return EXIT_FAILURE;
		  }
		  else{
		  	ring_master_port = atoi(argv[2]);
		  }

		  

		  memset(&host_info, 0, sizeof(host_info));
		  host_info.ai_socktype = SOCK_STREAM;
		  host_info.ai_family   = AF_UNSPEC;
          
          
           status = getaddrinfo(argv[1], argv[2], &host_info, &host_info_list);
           if (status != 0) {
   				 perror("getaddrinfo()");
                 return EXIT_FAILURE;
  			}
           
          //Open the socket to the ringmaster
          socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
          if(socket_fd == -1){
          	printf("Error: cannot create socket to ringmaster.\n");
    		return EXIT_FAILURE;
          }

          //start connection
          status = connect(socket_fd, host_info_list->ai_addr,host_info_list->ai_addrlen);
          if(status < 0){
          	printf("Cannot connect to ringmaster.\n");
          	return EXIT_FAILURE;
          }


       //Create a socket for the player 
      
       gethostname(playername, sizeof(playername));
       player_detail = gethostbyname(playername);

       player_socketfd = socket(AF_INET, SOCK_STREAM, 0);
       if(player_socketfd < 0){
       	printf("Error: Cannot create socket.\n");
       	return EXIT_FAILURE;
       }

  
       player_socket_detail.sin_family = AF_INET;
       for(int i = PORTBEGIN; i <= PORTEND; i++ ){

       	   if(i == neighbour_port || i == ring_master_port){
       	   	continue;
       	   }
       		player_socket_detail.sin_port = htons(i);
       		memcpy(&player_socket_detail.sin_addr, player_detail->h_addr_list[0], player_detail->h_length);
       		status = bind(player_socketfd, (struct sockaddr *)&player_socket_detail, sizeof(player_socket_detail));
       		if(!status)
       			break;
          else if(status < 0 && i == PORTEND){
            printf("No remaining port.\n");
            return EXIT_FAILURE;
          }
       	}

  
  if (!(getsockname(player_socketfd, (struct sockaddr*)&buff, &len))) {
    	player_portnum = ntohs(buff.sin_port);
  }
  else{
    printf("Error: getsockname failure.\n");
    return EXIT_FAILURE;
  }


  send(socket_fd, (char*)&player_portnum, sizeof(int), 0);
  recv(socket_fd, (char *)&num_of_players, sizeof(int), 0);
  printf("%d\n", num_of_players);
  recv(socket_fd, (char*)&num_of_hops, sizeof(int), 0);
  printf("%d\n", num_of_hops);
  recv(socket_fd, (char*)&id, sizeof(int), 0);
  srand((unsigned int)time(NULL) + id);

  //Done receiving and sending basic information
  printf("Connected as player %d\n", id);
  recv(socket_fd, (char*)&neighbour_port, sizeof(int), 0);
  //printf("%d\n", neighbour_port);

  recv(socket_fd, (char*)&neighbour_hostname, 64, MSG_WAITALL);
  //Done receiving neighbour port and hostname;
  //printf("%s\n", neighbour_hostname);

  neighbour_detail = gethostbyname(neighbour_hostname);
  if(neighbour_detail == NULL){
    printf("Error getting neighbour info.\n");
    return EXIT_FAILURE;
  }

  
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
    printf("Cannot create a socket to right neighbour.\n");
    return EXIT_FAILURE;
  }

  neighbour_socket_detail.sin_family = AF_INET;
  neighbour_socket_detail.sin_port = htons(neighbour_port);
  memcpy(&neighbour_socket_detail.sin_addr, neighbour_detail->h_addr_list[0], neighbour_detail->h_length);

  status = connect(right_neighbour_sfd, (struct sockaddr*)&neighbour_socket_detail, sizeof(neighbour_socket_detail));
  if(status == -1){
  	printf("Couldn't connect with right neighbour\n");
    return EXIT_FAILURE;
  }

  left_neighbour_sfd = accept(player_socketfd, (struct sockaddr*)&neighbour_socket_in, &len);
  if(left_neighbour_sfd == -1){
  	printf("Problem arised in accepting connection.\n");
  	return EXIT_FAILURE;
  }

  fd_set game_fds;
  FD_ZERO(&game_fds);
  FD_SET(socket_fd, &game_fds);
  FD_SET(right_neighbour_sfd, &game_fds);
  FD_SET(left_neighbour_sfd, &game_fds);

  struct potato buffer[1];
  memset(&buffer[0], 0, sizeof(buffer[0]));

 int max_sfd = socket_fd;
 if(left_neighbour_sfd > max_sfd){
 	max_sfd = left_neighbour_sfd; 
 }
 else if (right_neighbour_sfd > max_sfd){
 	max_sfd = right_neighbour_sfd;
 }

 while(1){

 	fd_set temporary_fds = game_fds;
 	status = select(max_sfd + 1, & temporary_fds, NULL, NULL, NULL);
 	if(status == -1){
 		perror("select()");
 	}

   int reading_fd;

   if(FD_ISSET(socket_fd, &temporary_fds))
		reading_fd = socket_fd;
   else if(FD_ISSET(right_neighbour_sfd, &temporary_fds))
   		reading_fd = right_neighbour_sfd;
   else if(FD_ISSET(left_neighbour_sfd, &temporary_fds))
   		reading_fd = left_neighbour_sfd;

  
 	
 	recv(reading_fd, &receive_signal, sizeof(int), 0);
    if(receive_signal == 4500){
 		break;
 	}
 	else{
   
   		send(reading_fd, &ack, sizeof(int), 0);
   		int receive = recv(reading_fd, buffer, sizeof(buffer), MSG_WAITALL);
   		if(receive == -1){
   			printf("Error in receiving the potato\n");
   			return EXIT_FAILURE;
   		}
   		else{
   			send(reading_fd, &ack, sizeof(int), 0);
   		}

   		hot_potato = buffer[0];
   		printf("%d\n", hot_potato.hop_num);
   		printf("%d\n", hot_potato.current_hop);
   		hot_potato.hop_trace[hot_potato.current_hop] = id;
   		hot_potato.current_hop++;
   		hot_potato.hop_num--;
		buffer[0] = hot_potato;

		if(hot_potato.hop_num == 0){
			printf("I'm it\n");
			status = send(socket_fd, buffer, sizeof(buffer), 0);
			if(status == -1){
				printf("Problem in sending the potato back to ringmaster.\n");
				return EXIT_FAILURE;
			}
		}



 	else{

 			int random = rand() % 2;
 			if(random == 0)
 				destination_fd = left_neighbour_sfd;
 			else if (random == 1)
 				destination_fd = right_neighbour_sfd;
 	


 			if(id == 0){
 				if(random == 1)
 					neighbour_id = id + 1;
 				else if (random == 0) 
          	  		neighbour_id = num_of_players - 1;
          	}	


          else if(id == num_of_players - 1) {
          	if (random == 0)
            	neighbour_id = id - 1;
          	else if (random == 1)  
            	neighbour_id = 0;
         }

          else {
          	if (random == 1) 
            neighbour_id = id + 1;
          	else if(random == 0) 
    	        neighbour_id = id - 1;
    		}
          
        	printf("Sending potato to %d \n", neighbour_id);
        	receive_signal = 5500;
        	send(destination_fd, (char*)&receive_signal, sizeof(signal), 0);
        	recv(destination_fd, &ack, sizeof(ack), 0);
        	send(destination_fd, buffer, sizeof(buffer), 0);
        	recv(destination_fd, &ack, sizeof(int), 0);

        	continue;

		}

	}

 
  }



}


  close(right_neighbour_sfd);
  close(player_socketfd);
  close(socket_fd);
  return EXIT_SUCCESS;



}

