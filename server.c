#include "utils.h"
#include <arpa/inet.h>
#include <limits.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 37
#define SOCKETERROR (-1)
#define BUF_SIZE 32768
#define MAX_QUEUE 10
typedef struct sockaddr SA;
typedef struct sockaddr_in SA_IN;

const char bad_arguments[] =
    "You have either used the incorrect number of arguments or put spaces in "
    "one of the arguments\n";
const char bad_recipient[] = "recipient of that name does not exist\n";

queuedSend find_queue_member(int member_address, queuedSend *queue) {
  queuedSend res;
  res.emailSocket = member_address;
  for (int i = 0; i < MAX_QUEUE; i++) {
    if (queue[i].emailSocket == member_address) {
      res.recipient = queue[i].recipient;
      res.sender = queue[i].sender;
      res.subject = queue[i].subject;
    }
  }
  return res;
}

int main() {
  int serverFd, clientFd;
  struct sockaddr_in server, client;
  int option = 1;
  serverFd = socket(AF_INET, SOCK_STREAM, 0);
  setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  int server_len = sizeof(server);
  char buffer[BUF_SIZE];
  queuedSend email_send_queue[MAX_QUEUE];
  int queue_length = 0;

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(PORT);

  if (serverFd < 0) {
    perror("Cannot create the socket");
    exit(1);
  }
  if (bind(serverFd, (struct sockaddr *)&server, server_len) < 0) {
    perror("Cannot bind socket");
    exit(2);
  }
  if (listen(serverFd, 10) < 0) {
    perror("Listen error");
    exit(3);
  }
  fd_set current_sockets, ready_sockets, email_send_sockets;
  FD_ZERO(&email_send_sockets);
  FD_ZERO(&current_sockets);
  FD_SET(serverFd, &current_sockets);
  int max_fd = serverFd;
  while (1) {
    ready_sockets = current_sockets;
    if (select(max_fd + 1, &ready_sockets, NULL, NULL, NULL) < 0) {
      printf("select error");
      exit(70);
    }
    for (int i = 0; i <= max_fd; i++) {
      if (FD_ISSET(i, &ready_sockets)) {
        if (i == serverFd) {
          socklen_t len = sizeof(client);
          printf("(Listening for connection)\n");
          if ((clientFd = accept(serverFd, (struct sockaddr *)&client, &len)) <
              0) {
            perror("accept error");
            exit(4);
          }
          char *client_ip = inet_ntoa(client.sin_addr);
          printf("Accepted new connection from a client %s:%d\n", client_ip,
                 ntohs(client.sin_port));
          memset(buffer, 0, sizeof(buffer));
          int size = read(clientFd, buffer, sizeof(buffer));
          if (size < 0) {
            perror("Read error");
            exit(5);
          }
          if (buffer[0] == '0') {
            // buffer should be of form "0 username password"
            int arg_count;
            char *user_creation_output;
            char **args = get_arguments(buffer + 2, &arg_count);
            if (arg_count != 2) {
              if (write(clientFd, bad_arguments, sizeof(bad_arguments)) < 0) {
                perror("Write error");
                exit(6);
              }
            } else {
              char *username = args[0];
              char *password = args[1];
              free(args);
              user_creation_output = create_user(username, password);
              if (write(clientFd, user_creation_output,
                        strlen(user_creation_output)) < 0) {
                perror("write error");
                exit(6);
              }
            }
          } else if (buffer[0] == '1') {
            // expect output of form "1 username password recipient subject"
            int arg_count;
            char **args = get_arguments(buffer + 2, &arg_count);
            if (arg_count != 4) {
              if (write(clientFd, bad_arguments, sizeof(bad_arguments)) < 0) {
                perror("Write error");
                exit(6);
              }
            } else {
              struct authenticationResult authres;
              authres = authenticate_user(args[0], args[1]);
              if (authres.result == false) {
                if (write(clientFd, authres.message, strlen(authres.message)) <
                    0) {
                  printf("Write error");
                  exit(6);
                }
                continue;
              }
              struct authenticationResult recipient_exists;
              recipient_exists = authenticate_user(args[2], "");
              if (strcmp("no such user exists\n\0", recipient_exists.message) ==
                  0) {
                if (write(clientFd, bad_recipient, strlen(bad_recipient)) < 0) {
                  printf("Write error");
                  exit(6);
                }
                continue;
              }
              char start_send_prompt[] = "6SSF_SSF";
              if (write(clientFd, start_send_prompt,
                        sizeof(start_send_prompt)) < 0) {
                printf("Write error");
                exit(6);
              }
              int filerecvSocket;
              int addr_size = sizeof(SA_IN);
              SA_IN filerecv_addr;
              check(filerecvSocket = accept(serverFd, (SA *)&filerecv_addr,
                                            (socklen_t *)&addr_size),
                    "accept failed");
              queuedSend queue_member;
              queue_member.emailSocket = filerecvSocket;
              queue_member.recipient = args[2];
              queue_member.sender = args[0];
              queue_member.subject = args[3];
              if (queue_length == MAX_QUEUE) {
                printf("max queue length reached, client must try again");
                continue;
              }
              email_send_queue[queue_length] = queue_member;
              queue_length += 1;
              FD_SET(filerecvSocket, &current_sockets);
              FD_SET(filerecvSocket, &email_send_sockets);
              if (filerecvSocket > max_fd)
                max_fd = filerecvSocket;
            }
          } else if (buffer[0] == '2') {

            int arg_count;
            char **args = get_arguments(buffer + 2, &arg_count);
            if (arg_count != 2) {
              if (write(clientFd, bad_arguments, sizeof(bad_arguments)) < 0) {
                perror("Write error");
                exit(6);
              }
            } else {
              struct authenticationResult authres;
              authres = authenticate_user(args[0], args[1]);
              if (authres.result == false) {
                if (write(clientFd, authres.message, strlen(authres.message)) <
                    0) {
                  printf("Write error");
                  exit(6);
                }
                continue;
              }
              // do stuff to make it get list of received emails
              int matches;
              char **inbox = extract_emails_starting_with_char(
                  concat_all(3, "db/", args[0], ".txt"), "email_0", &matches);
              for (int j = 0; j < matches; j++) {
                if (send(clientFd, inbox[j], strlen(inbox[j]), 0) == -1) {
                  perror("[-]Error in sending inbox.");
                  exit(1);
                }
              }
            }
          } else if (buffer[0] == '3') {
            int arg_count;
            char **args = get_arguments(buffer + 2, &arg_count);
            if (arg_count != 2) {
              if (write(clientFd, bad_arguments, sizeof(bad_arguments)) < 0) {
                perror("Write error");
                exit(6);
              }
            } else {
              struct authenticationResult authres;
              authres = authenticate_user(args[0], args[1]);
              if (authres.result == false) {
                if (write(clientFd, authres.message, strlen(authres.message)) <
                    0) {
                  printf("Write error");
                  exit(6);
                }
                continue;
              }
              // do stuff to make it get list of received emails
              int matches;
              char **inbox = extract_emails_starting_with_char(
                  concat_all(3, "db/", args[0], ".txt"), "email_1", &matches);
              for (int j = 0; j < matches; j++) {
                if (send(clientFd, inbox[j], strlen(inbox[j]), 0) == -1) {
                  perror("[-]Error in sending inbox.");
                  exit(1);
                }
              }
            }
          } else if (buffer[0] == '4') {
            int arg_count;
            char **args = get_arguments(buffer + 2, &arg_count);
            if (arg_count != 3) {
              if (write(clientFd, bad_arguments, sizeof(bad_arguments)) < 0) {
                perror("Write error");
                exit(6);
              }
            } else {
              struct authenticationResult authres;
              authres = authenticate_user(args[0], args[1]);
              if (authres.result == false) {
                if (write(clientFd, authres.message, strlen(authres.message)) <
                    0) {
                  printf("Write error");
                  exit(6);
                }
                continue;
              }
              // do stuff to make it check if email exists and then send it
              dl_email(args[2], args[0], clientFd);
            }
          }

        } else {
          if (FD_ISSET(i, &email_send_sockets)) {
            queuedSend socket_metadata = find_queue_member(i, email_send_queue);
            write_emailfile(i, socket_metadata);
          }
          FD_CLR(i, &current_sockets);
        }
      }

      close(clientFd);
    }
  }

  close(serverFd);
  return 0;
}
