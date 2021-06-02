#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // close function

/** IMPORTANT NOTE:
    IF YOU CLOSE THIS PROGRAM WHILE IT STILL
    NEEDS TO RECEIVE INFORMATION FROM A CLIENT
    THE SOCKET WILL NOT BE CLOSED.
    SO CLOSE THE CONNECTION FROM THE CLIENT INSTEAD
    FOR THE SOCKET CONNECTION TO BE CLOSED.
    OTHERWISE, HANDLE IT SOMEHOW.
 **/

/** PARAMETERS FOR THE SERVER **/
#define PORT               9002
#define IP_ADDRESS         "192.168.43.59"
#define MAX_CONNECTIONS    3

static volatile int s[MAX_CONNECTIONS] = {0};
static volatile int it = 0;

void *server_function1(void *arg){ // To pass in arguments See-> man pthread_create
  char client_message[256] = "Welcome to the network.\n";
  char client_response[256];
  int check;

  int i = it; // Keep this for the socket recv and send functions
  it++;       // Open space for the next thread. Solve this for if clients close a connection

  s[i] = *(int *)arg;
  free(arg);

  send(s[i], client_message, sizeof(client_message), 0);
  while(1){
   check = recv(s[i], client_response, sizeof(client_response), 0);
   if(check > 0){
     printf("Client %d Reply: %s", s[i]-4, client_response);

     /** Testing for max connections to the network **/
     //printf("i = %d\n", i);

     /** Send data to the right side of the nodes **/
     for(int j = i+1; j < MAX_CONNECTIONS; j++){
       send(s[j], client_response, sizeof(client_response), 0);
     }

     /** Send data to the left side of the nodes **/
     /** i cannot be a negative number **/
     if(i != 0){
       for(int j = i-1; j >= 0; j--){
         send(s[j], client_response, sizeof(client_response), 0);
       }
     }
   }
   else{
     close( (int)s[i]);
     s[i] = 0; // Reset the file descriptor for the socket
     it = i;   // Assign the open space for the next thread.
     printf("Client %d disconnected.\n", i);
     pthread_exit(&i);
   }
  }
  return NULL;
}

int main(void){
  int s;
  int client_socket;
  int peer_ip;

  s = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(PORT);
  server_address.sin_addr.s_addr = inet_addr(IP_ADDRESS);

  const struct sockaddr *address = (struct sockaddr *)&server_address;
  socklen_t len = sizeof(server_address);

  bind(s, address, len);
  listen(s, MAX_CONNECTIONS);

  pthread_t server_threads[MAX_CONNECTIONS];

  while(1){

    client_socket = accept(s, NULL, NULL);
    if( (client_socket >= 4) && (client_socket <= (MAX_CONNECTIONS+3)) ){
      printf("[+] A connection has been accepted!\n");

      struct sockaddr_in ip_from_socket;
      socklen_t ip_len = sizeof(ip_from_socket);

      struct sockaddr *ip_addr = (struct sockaddr *)&ip_from_socket;
      socklen_t *ip_addrlen = &ip_len;

      peer_ip = getpeername(client_socket, ip_addr, ip_addrlen);
      if(peer_ip == -1){
        printf("Error getting Ip address for client: %d", client_socket);
      }

      printf("Ip address connected: %s  ", inet_ntoa(ip_from_socket.sin_addr));
      printf("| Port %d\n", ntohs(ip_from_socket.sin_port));
    }
    else{
      printf("A connection has been rejected!\n");
      close(client_socket);
    }

    int * client_number = malloc(sizeof(int));
    *client_number = client_socket;
    int thread_id = 0;
    if(it < MAX_CONNECTIONS){
      thread_id = pthread_create(&server_threads[it], NULL, &server_function1, client_number);
      if(thread_id != 0){
        printf("Error creating a new thread for an incomming connection.\n");
      }
    }
    else {
      free(client_number);
      printf("Max connections have been reached.\n");
    }

  }

  return 0;
}

