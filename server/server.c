#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 38200
#define BUFFER_SIZE 1024

int main()
{
  int server_fd;

  //vytvorenie socketu
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(server_fd < 0) {
    perror("Chyba pri vytvarani socketu");
    return -1;
  }
  printf("Server Socket bol vytvoreny\n");

  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  //nastavenie ip adresy servera
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT); //host to network -> short / long

  if(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    perror("Chyba pri priradeni adresy");
    close(server_fd);
    return -2;
  }
  printf("Socket naviazany na port: %d\n", PORT);

  if(listen(server_fd, 5) < 0) {
    perror("Listen zlyhal");
    close(server_fd);
    return -3;
  }
  printf("Cakam na pripojenie\n");

  int client_fd;
  struct sockaddr_in client_addr;
  socklen_t client_len;

  client_len = sizeof(client_addr);
  client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

  if(client_fd < 0) {
    perror("Accept zlyhal");
    return -4;
  }
  printf("Klient pripojeny %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));


  char buffer[BUFFER_SIZE];
  while(1) {
    memset(buffer, 0, BUFFER_SIZE);

    int bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
    if(bytes_read <= 0) {
      printf("Klient sa odpojil\n");
      break;
    }
    printf("Prijata sprava: %s", buffer);

    if(strncmp(buffer, "quit", 4) == 0){
      printf("Ukoncujem spojenie\n");
      break;
    }

    send(client_fd, buffer, strlen(buffer), 0);
  }


  close(client_fd);
  close(server_fd);
  printf("Server je ukonceny\n");

  return 0;
}
