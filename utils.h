#ifndef utils
#define utils

#include <arpa/inet.h>
#include <limits.h>
#include <netinet/in.h>
#include <stdarg.h>
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
#define LINE_LEN 4096

typedef struct sockaddr SA;
typedef struct sockaddr_in SA_IN;

typedef struct {
  int emailSocket;
  char *recipient;
  char *sender;
  char *subject;
} queuedSend;

void write_emailfile(int sockfd, queuedSend email_metadata);
struct authenticationResult {
  bool result;
  char *message;
};
int check(int exp, const char *msg);
char *concat(const char *s1, const char *s2);
int char_count(char *s, char wanted_char);
char **get_arguments(char *command, int *arg_count);
char *create_user(char *username, char *password);
char *concat_all(int count, ...);
struct authenticationResult authenticate_user(char *username, char *password);
void next_id();
char **extract_emails_starting_with_char(char *filename, char *start,
                                         int *matches);
void dl_email(char *id, char *username, int clientFd);

void send_file(FILE *fp, int sockfd);
#endif
