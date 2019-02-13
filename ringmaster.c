
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

 if (atoi(argv[2]) < 1) {
    printf("Please input valid number for players.\n");
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

int error = check_error_input(argv);
if(!error){
int port_num = atoi(argv[1]);
int num_of_players = atoi(argv[2]);
int num_of_hops = atoi(argv[3]);
int player_port_fd[num_of_players][2]; //For storing player file-descriptor and port-number
char player_hostname[num_of_players][128]; //For storing player host-name
struct sockaddr_storage socket_addr;
socklen_t socket_addr_len;



srand((unsigned int)time(NULL) + num_of_players);

//Initialization
printf("Potato Ringmaster\n");
printf("Players = %d\n", num_of_players);
printf("Hops = %d\n", num_of_hops);


//Get host info
int status;
int socket_fd;
struct addrinfo hostsocket_info;
struct addrinfo* hostsocket_info_list;
struct hostent* host_info;

char hostname[64];
gethostname(hostname, sizeof(hostname)); //returns hostname for current process
host_info = gethostbyname(hostname);
if (host_info == NULL) {
    fprintf(stderr, "host %s not found\n", hostname);
    return EXIT_FAILURE;
  }

 memset(&hostsocket_info, 0, sizeof(hostsocket_info));
 hostsocket_info.ai_protocol = 0;
 hostsocket_info.ai_flags    = AI_PASSIVE;
 hostsocket_info.ai_family   = AF_INET;
 hostsocket_info.ai_socktype = SOCK_STREAM;

 status = getaddrinfo(hostname, argv[1], &hostsocket_info, &hostsocket_info_list);

 //create a socket
 socket_fd = socket(hostsocket_info_list->ai_family,
                     hostsocket_info_list->ai_socktype,
                     hostsocket_info_list->ai_protocol);
if (socket_fd == -1) {
    printf("Error: cannot create socket.\n");
    return EXIT_FAILURE;
  }

//Set socket details
int yes = 1;
status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
if(status < 0){
	perror("setsockopt");
    close(socket_fd);
    return EXIT_FAILURE;
}  

 status = bind(socket_fd, hostsocket_info_list->ai_addr, hostsocket_info_list->ai_addrlen);
  if (status == -1) {
    printf("Error: cannot bind socket\n");
    return EXIT_FAILURE;
  } 

 status = listen(socket_fd, num_of_players);
  if (status == -1) {
    printf("Error: cannot listen on socket\n");
    return EXIT_FAILURE;
  }

int i = 0;
while ( i < num_of_players){

socket_addr_len = sizeof(socket_addr);
int port_number_player;
int incoming_connection_fd;

incoming_connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
if (incoming_connection_fd == -1) {
    printf("Error: cannot accept connection on socket\n");
    return EXIT_FAILURE;
  }

printf("Player %d is ready to play\n", i);

//Store player hostname
char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
getnameinfo((struct sockaddr *)&socket_addr, socket_addr_len, hbuf, sizeof(hbuf), sbuf,
                       sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
memset(hbuf, 0x00, sizeof(hbuf));
memset(sbuf, 0x00, sizeof(sbuf));
strcpy(player_hostname[i], hbuf);

//Need to receive player port number and File descriptor
recv(incoming_connection_fd, &port_number_player, sizeof(int), 0);
player_port_fd[i][0] = incoming_connection_fd;
player_port_fd[i][1] = port_number_player;

//Once connection is established, we can send necessary information
//send total num of players, player id and number of hops to the player
send(player_port_fd[i][0], (char *)&num_of_players, sizeof(int), 0);
send(player_port_fd[i][0], (char*)&num_of_hops, sizeof(int), 0);
send(player_port_fd[i][0], (char*)&i, sizeof(int), 0);


//Send left and right neighbour information


}

}











}
