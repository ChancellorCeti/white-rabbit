#include "utils.h"
#include <arpa/inet.h> // sockaddr_in, AF_INET, SOCK_STREAM, INADDR_ANY, socket etc...
#include <stdio.h>  // perror, printf
#include <stdlib.h> // exit, atoi
#include <string.h> // strlen, memset
#include <unistd.h> // write, read, close
#define BUF_SIZE 32768
#define SMALL_BUF_SIZE 1024
#define SERVER_PORT 37

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
    // to-do: fix warning on line 59 (idk what it's even about)
    filename = argv[6];
    buffer =
        concat_all(8, "1 ", argv[2], " ", argv[3], " ", argv[4], " ", argv[5]);
  } else if (strcmp(argv[1], "open_inbox") == 0) {
    if (argc != 4) {
      printf("incorrect number of arguments!");
      exit(1);
    }
    buffer = concat_all(4, "2 ", argv[2], " ", argv[3]);
  } else if (strcmp(argv[1], "open_outbox") == 0) {
    if (argc != 4) {
      printf("Incorrect number of arguments!");
      exit(1);
    }
    buffer = concat_all(4, "3 ", argv[2], " ", argv[3]);
  } else if (strcmp(argv[1], "dl_email") == 0) {
    if (argc != 5) {
      printf("Incorrect number of arguments!");
      exit(1);
    }
    // make it expect dl_email username password id
    buffer = concat_all(6, "4 ", argv[2], " ", argv[3], " ", argv[4]);
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

  if (strcmp(argv[1], "open_inbox") == 0) {
    printf("The emails you have received are as follows:\n");
    printf("JUNK_TEXT ID SENDER SUBJECT\n");
    int n;
    char line[BUF_SIZE];

    while (1) {
      n = read(serverFd, line, BUF_SIZE);
      if (n <= 0) {
        break;
        return 0;
      }
      printf("%s\n", line);
      /*int q = fwrite(line, sizeof(char), strlen(line), inbox);
      if (q != strlen(buffer)) {
          perror("wtf");
          exit(65);
      }*/
      if (strcmp("incorrect password!\n", line) == 0) {
        return 0;
      }
      bzero(line, BUF_SIZE);
    }
    return 0;
  }

  if (strcmp(argv[1], "open_outbox") == 0) {
    printf("The emails you have sent are as follows:\n");
    printf("JUNK_TEXT ID RECIPIENT SUBJECT\n");
    int n;
    char line[SMALL_BUF_SIZE];

    while (1) {
      n = read(serverFd, line, SMALL_BUF_SIZE);
      if (n <= 0) {
        break;
        return 0;
      }
      printf("%s\n", line);
      bzero(line, SMALL_BUF_SIZE);
    }
    return 0;
  }

  if (read(serverFd, recv, sizeof(recv)) < 0) {
    perror("cannot read");
    exit(4);
  }

  if (strcmp(argv[1], "dl_email") == 0) {
    if (strcmp(recv, "sending file now!\n") != 0) {
      printf("%s", recv);
      return 0;
    }
    printf("%s", recv);
    int n;
    char line[15000];
    FILE *fp;
    if (argc < 5) {
      perror("insufficient arguments");
      exit(5);
    }
    char *wanted_email_filename = concat(argv[4], ".txt");
    fp = fopen(wanted_email_filename, "w");
    free(wanted_email_filename);
    while (1) {
      n = read(serverFd, line, 15000);
      if (n <= 0) {
        break;
        return 0;
      }
      int q = fwrite(line, sizeof(char), strlen(line), fp);
      if (q != (int)strlen(line)) {
        perror("failed to write to file");
        exit(65);
      }
      bzero(line, 15000);
    }
    return 0;
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
