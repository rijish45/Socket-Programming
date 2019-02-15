
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
#include <errno.h>


//Input Error Checking
int check_error_input(char * argv[]){

//Input parameter checking 

for(int i = 1; i <= 3; i++){
	int flag = 0;
	int j = 0;
	while( j < strlen(argv[i])){
  		flag = isdigit(argv[i][j]);
  		if (flag == 0){
  			printf("Input parameters should be numbers. Check input.\n");
  			return EXIT_FAILURE;
  		}
  		j++;
  	}
  }

 if ( atoi(argv[1]) < 1024 || atoi(argv[1]) > 65535 ) {
    printf("Please input valid port number above 1024 and below 65535.\n");
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

POTATO hot_potato; //Potato instantiation

int connection_initiate; //signal for connection with neighbours
int pass_potato; // signal for passing the potato
int game_end; //signal for ending the game
int random_player; //random player selection using rand()

if(argc > 4 || argc < 4){
		printf("Correct format is : ringmaster <port_num> <num_players> <num_hops>\n");
    	return EXIT_FAILURE;
}

int error = check_error_input(argv);

if(!error){

//Get the values from the command line
int port_num = atoi(argv[1]);
int num_of_players = atoi(argv[2]);
int num_of_hops = atoi(argv[3]);

srand((unsigned int)time(NULL)); //seed
int player_port_fd[num_of_players][2]; //For storing player file-descriptor and port-number
char player_hostname[num_of_players][64]; //For storing player host-name

struct sockaddr_storage socket_addr;
socklen_t socket_addr_len;


//Initialization
printf("Potato Ringmaster\n");
printf("Players = %d\n", num_of_players);
printf("Hops = %d\n", num_of_hops);


//Get host info
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
 hostsocket_info.ai_socktype = SOCK_STREAM;
 hostsocket_info.ai_family   = AF_INET;
 int address_status = getaddrinfo(hostname, argv[1], &hostsocket_info, &hostsocket_info_list);

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
int sock_status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
if(sock_status < 0){
	perror("setsockopt");
    close(socket_fd);
    return EXIT_FAILURE;
}  

 //Bind system call
 int bind_status = bind(socket_fd, hostsocket_info_list->ai_addr, hostsocket_info_list->ai_addrlen);
  if (bind_status == -1) {
    printf("Error: cannot bind socket\n");
    return EXIT_FAILURE;
  } 

 //Listen on the socket
 int listen_status = listen(socket_fd, num_of_players);
  if (listen_status == -1) {
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
else{
printf("Player %d is ready to play\n", i);
}

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


i++; //increment 
}


//Need to send portnumber and host-name of the neighbours
int j = 0;
while(j < num_of_players){

  if(j == 0){
    send(player_port_fd[j][0], (char *)&player_port_fd[num_of_players-1][1],sizeof(int), 0);
    send(player_port_fd[j][0], (char*)&player_hostname[num_of_players-1], 64, 0);
  }
  else{
    send(player_port_fd[j][0], (char *)&player_port_fd[j+1][1],sizeof(int), 0);
    send(player_port_fd[j][0], (char*)&player_hostname[j+1], 64, 0);
  }
  j++;
}


//Send signal to initiate connections between the neighbouring players

connection_initiate = 1;
for(int i = 0; i < num_of_players; i++){
  send(player_port_fd[i][0], (char *)&(connection_initiate),sizeof(int), 0);
}

//Make sure that all players are ready

for (int i = 0; i < num_of_players; i++){
	int ready = 0;
	int player_ready_status = recv(player_port_fd[i][0],&ready, sizeof(int), 0 );
	if(player_ready_status == -1){
		printf("Player %d is not ready. Restart the game again.", i);
		EXIT_FAILURE;
	}
}

sleep(5); //Wait for some time

//Now everything is set, we can begin the game

if(num_of_hops == 0){
  
   game_end = 1;
   for(int i = 0; i < num_of_players; i++){
      send(player_port_fd[i][0], (char *)&(game_end),sizeof(int), 0);
    }
  close(socket_fd); //close the TCP socket
  for(int i = 0; i < num_of_players; i++){
   close(player_port_fd[i][0]);
  }

  return EXIT_SUCCESS;
}


else{

    random_player = ((rand()) % num_of_players);
    printf("Ready to start the game, sending potato to player %d\n", random_player );
    

    //Send potato to the random player
    pass_potato = 1;
    send(player_port_fd[random_player][0], (char *)&pass_potato, sizeof(int), 0);
    int ack = 0;
    recv(player_port_fd[random_player][0], &ack, sizeof(int), 0);
   
    //Set the number of hops
	  hot_potato.hop_num = num_of_hops;
    struct potato buffer[1];
    int size_of_buffer = sizeof(int) + 512 * sizeof(int);
    buffer[0] = hot_potato;
    send(player_port_fd[random_player][0], buffer, size_of_buffer, 0);
    ack = recv(player_port_fd[random_player][0], &ack, sizeof(int), 0);
    
 
    int max_file_descriptor = player_port_fd[num_of_players - 1][0]; 
    fd_set read_file_descriptors;
    FD_ZERO(&read_file_descriptors);

  //monitor the file-descriptors
  int select_status = select(max_file_descriptor + 1, &read_file_descriptors, NULL, NULL, NULL);
  if(select_status < 0 && errno!= EINTR){
    printf("Select error"); 
    return EXIT_FAILURE;
  }

  //Receive the potato
  int index = 0;
  while(index < num_of_players){
     if (FD_ISSET(player_port_fd[index][0], &read_file_descriptors)){
        
        ssize_t receive = recv(player_port_fd[index][0], buffer, size_of_buffer , 0);
     	
     	if (receive < 0){
	  		printf(("Error receiving potato at the end of the game."));
	  		return EXIT_FAILURE;
	  	}
     }

     index++;

  }

  hot_potato = buffer[0];
  printf("Trace of potato:\n");
  for (int i = 0; i < num_of_hops; i++ ){
  	if(i != (num_of_hops -1 ))
  		printf("%d, ", hot_potato.hop_trace[i]);
  	else
  		printf("%d\n", hot_potato.hop_trace[i]);
  }

  //Game is over, send signal to all players
  
  game_end = 1;
  for(int i = 0; i < num_of_players; i++){
      send(player_port_fd[i][0], (char *)&(game_end),sizeof(int), 0);
      close(player_port_fd[i][0]);
}

 close(socket_fd); //close the TCP socket
 return EXIT_SUCCESS;


}
       }

}




