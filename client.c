#include "utils.h"
#include <arpa/inet.h> // sockaddr_in, AF_INET, SOCK_STREAM, INADDR_ANY, socket etc...
#include <stdio.h>  // perror, printf
#include <stdlib.h> // exit, atoi
#include <string.h> // strlen, memset
#include <unistd.h> // write, read, close
#define BUF_SIZE 32768
#define SERVER_PORT 37

void send_file(FILE *fp, int sockfd) {
  int n;
  char data[BUF_SIZE] = {0};
  while (fgets(data, BUF_SIZE, fp) != NULL) {
    if (send(sockfd, data, sizeof(data), 0) == -1) {
      perror("[-]Error in sending file.");
      exit(1);
    }
    bzero(data, BUF_SIZE);
  }
  close(sockfd);
}

int create_connection(const char *server_ip) {
  int sockfd;
  struct sockaddr_in serv_addr;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERVER_PORT);

  if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
    perror("Invalid address");
    exit(EXIT_FAILURE);
  }

  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("Connection failed");
    exit(EXIT_FAILURE);
  }

  return sockfd;
}

int main(int argc, char const *argv[]) {
  char *filename;
  int serverFd;
  struct sockaddr_in server;
  int len;
  int port = 37;
  char *server_ip = "127.0.0.1";
  char *buffer;
  if (argc == 1) {
    printf("error must supply an argument");
    exit(1);
  }
  if (strcmp(argv[1], "create_user") == 0) {
    if (argc != 4) {
      printf("incorrect number of arguments!");
      exit(1);
    }
    buffer = concat_all(4, "0 ", argv[2], " ", argv[3]);
  } else if (strcmp(argv[1], "send_email") == 0) {
    if (argc != 7) {
      printf("incorrect number of arguments!");
      exit(1);
    }
    filename = argv[6];
    buffer =
        concat_all(8, "1 ", argv[2], " ", argv[3], " ", argv[4], " ", argv[5]);
  }
  serverFd = socket(AF_INET, SOCK_STREAM, 0);
  if (serverFd < 0) {
    perror("Cannot create socket");
    exit(1);
  }
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(server_ip);
  server.sin_port = htons(port);
  len = sizeof(server);
  if (connect(serverFd, (struct sockaddr *)&server, len) < 0) {
    perror("Cannot connect to server");
    exit(2);
  }

  if (write(serverFd, buffer, strlen(buffer)) < 0) {
    perror("Cannot write");
    exit(3);
  }
  free(buffer);
  char recv[BUF_SIZE];
  memset(recv, 0, sizeof(recv));
  ssize_t total_read = 0;
  ssize_t n;
  if (read(serverFd, recv, sizeof(recv)) < 0) {
    perror("cannot read");
    exit(4);
  }
  if (recv[0] == '6') {
    printf("got the okay to send file");
    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL) {
      perror("[-]Error in reading file.");
      exit(1);
    } else {
    }
    int file_sock = create_connection(server_ip);
    send_file(fp, file_sock);
    // send a file now
  } else {
    printf("%s", recv);
  }
  close(serverFd);
  return 0;
}
