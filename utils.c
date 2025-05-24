#include "utils.h"

void next_id(char *res_string) {
  FILE *fp;
  int res;
  char *filename = "db/main_db.txt";
  char buffer[BUF_SIZE];
  fp = fopen(filename, "r+");
  fscanf(fp, "%d", &res);
  res += 1;
  fclose(fp);
  FILE *fp_edit;
  fp_edit = fopen(filename, "w");
  char next_id_string[BUF_SIZE];
  sprintf(next_id_string, "%d", res);
  fprintf(fp_edit, "%s", next_id_string);
  fclose(fp_edit);
  sprintf(res_string, "%d", res);
  return;
}

void add_line_to_file(char *file_name, char *content) {
  FILE *fp;
  fp = fopen(file_name, "a");
  if (fp == NULL) {
    printf("file open error");
    exit(12);
  }
  fprintf(fp, "%s", content);
  fclose(fp);
}

void write_emailfile(int sockfd, queuedSend email_metadata) {
  int n;
  FILE *fp;
  char new_id[BUF_SIZE];
  next_id(new_id);
  char *filename = concat_all(3, "db/", new_id, ".txt");
  char buffer[BUF_SIZE];
  fp = fopen(filename, "w+");
  while (1) {
    n = recv(sockfd, buffer, BUF_SIZE, 0);
    if (n <= 0) {
      break;
      return;
    }
    int q = fwrite(buffer, sizeof(char), strlen(buffer), fp);
    if (q != strlen(buffer)) {
      perror("wtf");
      exit(65);
    }
    bzero(buffer, BUF_SIZE);
  }
  fclose(fp);
  add_line_to_file(concat_all(3, "db/", email_metadata.recipient, ".txt"),
                   concat_all(7, "email_0: ", new_id, " ",
                              email_metadata.sender, " ",
                              email_metadata.subject, "\n"));
  add_line_to_file(concat_all(3, "db/", email_metadata.sender, ".txt"),
                   concat_all(7, "email_1: ", new_id, " ",
                              email_metadata.recipient, " ",
                              email_metadata.subject, "\n"));
  return;
}

int check(int exp, const char *msg) {
  if (exp == SOCKETERROR) {
    perror(msg);
    exit(1);
  }
  return exp;
}

char *concat(const char *s1, const char *s2) {
  char *result = malloc(strlen(s1) + strlen(s2) + 1);
  strcpy(result, s1);
  strcat(result, s2);
  return result;
}

char *concat_all(int count, ...) {
  va_list args;
  va_start(args, count);

  // First pass: calculate total length
  size_t total_length = 0;
  for (int i = 0; i < count; i++) {
    const char *str = va_arg(args, const char *);
    total_length += strlen(str);
  }
  va_end(args);

  // Allocate memory (+1 for null terminator)
  char *result = malloc(total_length + 1);
  if (!result) {
    perror("malloc failed");
    exit(EXIT_FAILURE);
  }

  // Second pass: copy strings
  result[0] = '\0'; // start with empty string
  va_start(args, count);
  for (int i = 0; i < count; i++) {
    const char *str = va_arg(args, const char *);
    strcat(result, str);
  }
  va_end(args);

  return result;
}

int char_count(char *s, char wanted_char) {
  int char_count = 0;
  size_t slen = strlen(s);
  for (size_t i = 0; i < slen; i++) {
    if (s[i] == wanted_char) {
      char_count = char_count + 1;
    }
  }
  return char_count;
}

char **get_arguments(char *command, int *arg_count) {
  int delimiter_count = char_count(command, ' ');
  char **args = malloc(sizeof(char *) * (delimiter_count + 1));
  *arg_count = delimiter_count + 1;
  char *command_copy = strdup(command);
  char *arg_i;
  arg_i = strtok(command_copy, " ");
  int adding_index = 0;
  while (arg_i != NULL) {
    args[adding_index] = arg_i;
    adding_index++;
    arg_i = strtok(NULL, " ");
  }
  return args;
}

char *create_user(char *username, char *password) {
  FILE *fptr;
  char *filename = concat_all(3, "db/", username, ".txt");
  fptr = fopen(filename, "r");
  if (fptr != NULL) {
    fclose(fptr);
    free(filename);
    return "user with this name already exists\n";
  }
  fptr = fopen(filename, "w+");
  free(filename);
  if (fptr == NULL) {
    fclose(fptr);
    return "failed to create file\n";
  }
  fprintf(fptr, "username:%s\npassword:%s\n", username, password);
  fclose(fptr);
  return "User successfully created!";
}

struct authenticationResult authenticate_user(char *username, char *password) {
  FILE *fptr;
  char *filename = concat_all(3, "db/", username, ".txt");
  struct authenticationResult res;
  fptr = fopen(filename, "r");
  if (fptr == NULL) {
    free(filename);
    res.result = false;
    res.message = "no such user exists\n\0";
    return res;
  }
  fclose(fptr);
  res.result = true;
  res.message = "user authentication successful\n";
  return res;
}
