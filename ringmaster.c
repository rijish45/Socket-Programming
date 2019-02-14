
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
  			printf("Input parameters should be numbers. Check input.\n");
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

POTATO potato; //Potato instantiation

int connection_initiate; //signal for connection with neighbours
int pass_potato; // signal for passing the potato
int game_end; //signal for ending the game
int random_player; //random player selection using rand()

if(argc != 4){
		printf("Correct format is : ringmaster <port_num> <num_players> <num_hops>\n");
    	return EXIT_FAILURE;
}

int error = check_error_input(argv);

if(!error){

//Get the values from the command line
int port_num = atoi(argv[1]);
int num_of_players = atoi(argv[2]);
int num_of_hops = atoi(argv[3]);

//Set the number of hops
potato.hop_num = num_of_hops;

srand((unsigned int)time(NULL) + num_of_players); //seed

int player_port_fd[num_of_players][2]; //For storing player file-descriptor and port-number
char player_hostname[num_of_players][64]; //For storing player host-name
struct sockaddr_storage socket_addr;
socklen_t socket_addr_len;


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
 hostsocket_info.ai_socktype = SOCK_STREAM;
 hostsocket_info.ai_family   = AF_INET;
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

 //Bind system call
 status = bind(socket_fd, hostsocket_info_list->ai_addr, hostsocket_info_list->ai_addrlen);
  if (status == -1) {
    printf("Error: cannot bind socket\n");
    return EXIT_FAILURE;
  } 

 //Listen on the socket
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


i++; //incrr
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

//Now everything is set, we can begin the game

if(num_of_hops == 0){
   game_end = 1;
  
  for(int i = 0; i < num_of_players; i++){
      send(player_port_fd[i][0], (char *)&(game_end),sizeof(int), 0);
      close(player_port_fd[i][0]);
}
  close(socket_fd); //close the TCP socket
  return EXIT_SUCCESS;
}


else{

    random_player = ((rand()) % num_of_players);
    printf("Ready to start the game, sending potato to player %d\n", random_player );
    
    pass_potato = 1;
    send(player_port_fd[random_player][0], (char *)&pass_potato, sizeof(int), 0);
    int temp = sizeof(char)*(4 * num_of_hops);
 
    //Send the potato to the random player

    memset(potato.hop_trace, '\0', temp);
    strcat(potato.hop_trace, "");
    //Convert the struct into buffer
    char buffer[temp + sizeof(int)];
    memcpy(buffer, &potato, temp + sizeof(int));
    send(player_port_fd[random_player][0], buffer, temp + sizeof(int), 0);

    //The code is based on the linux manual instructions
 
    int max_file_descriptor = player_port_fd[0][0]; //Initialize the max-file descriptor as the first one
    fd_set read_file_descriptors;
    FD_ZERO(&read_file_descriptors);


    //Determine max_file_descriptor to use the select system() call
    int k = 0;
    while( k < num_of_players){
      FD_SET(player_port_fd[i][0], &read_file_descriptors);
      if(player_port_fd[i][0] > max_file_descriptor)
          max_file_descriptor = player_port_fd[i][0];
      k++;
  }

  //monitor the file-descriptors
  select(max_file_descriptor + 1, &read_file_descriptors, NULL, NULL, NULL);

  int index = 0;
  while(index < num_of_players){
     if (FD_ISSET(player_port_fd[i][0], &read_file_descriptors)){
        char received_buffer[temp + sizeof(int)];
        recv(player_port_fd[i][0], received_buffer, temp + sizeof(int), 0);
        memcpy(&potato, received_buffer, temp + sizeof(int));
     }

     index++;

  }

  printf("Trace of potato:\n%s\n", potato.hop_trace);
  
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




