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
#define MAX_CONNECTIONS    5

static volatile int s[MAX_CONNECTIONS];
static volatile int it = 0;

void *server_function1(void *arg){ // To pass in arguments See-> man pthread_create
  char client_message[256] = "Welcome to the network.\n";
  char client_response[256];
  int check;
  
  int i = it;
  it++;

  s[i] = *(int *)arg;
  free(arg);

  send(s[i], client_message, sizeof(client_message), 0);
  while(1){
   check = recv(s[i], client_response, sizeof(client_response), 0);
   if(check > 0){
     printf("Client %d Reply: %s", s[i]-4, client_response);
     
     printf("i = %d\n", i);
     
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
     //exit(1);
   }

  }
  return NULL;
}

int main(void){
  int s;
  int client_socket;

  s = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(PORT);
  server_address.sin_addr.s_addr = inet_addr(IP_ADDRESS);

  bind(s, (struct sockaddr *)&server_address, sizeof(server_address));
  listen(s, MAX_CONNECTIONS);
  
  pthread_t server_threads[5];
  
  while(1){
  
    client_socket = accept(s, NULL, NULL);
    if(client_socket >= 4){
      printf("[+] A connection has been accepted!\n");
    }
    else{
      printf("A connection has been rejected!\n");
    }

    int * client_number = malloc(sizeof(int));
    *client_number = client_socket;
    if(it < 5){
      pthread_create(&server_threads[it], NULL, &server_function1, client_number);

      /** I REMOVED THE VERIFCATION OF THE THREAD CREATION **/
    }
    else {
      printf("Max connections have been reached.\n");
    }
    
  }

  return 0;
}

