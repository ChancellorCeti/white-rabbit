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
  char new_id[1024];
  next_id(new_id);
  char *filename = concat_all(3, "db/", new_id, ".txt");
  char buffer[15000];
  fp = fopen(filename, "w+");
  while (1) {
    n = recv(sockfd, buffer, 15000, 0);
    if (n <= 0) {
      break;
      return;
    }
    int q = fwrite(buffer, sizeof(char), strlen(buffer), fp);
    if (q != strlen(buffer)) {
      perror("wtf");
      exit(65);
    }
    bzero(buffer, 15000);
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
  char *line;
  size_t line_size = 0;
  int line_one = getline(&line, &line_size, fptr);
  bzero(line, line_size);
  int line_two = getline(&line, &line_size, fptr);
  char real_pwd[strlen(line) - 8];
  strncpy(real_pwd, line + 9, strlen(line) - 9);
  real_pwd[strlen(line) - 9] = '\0';
  if (strcmp(real_pwd, concat(password, "\n")) != 0) {
    res.result = false;
    res.message = "incorrect password!\n";
    return res;
  }
  res.result = true;
  res.message = "user authentication successful\n";

  fclose(fptr);
  return res;
}

char **extract_emails_starting_with_char(char *filename, char *start,
                                         int *matches) {
  char **res = NULL;
  FILE *fp = fopen(filename, "r");
  int res_count = 0;
  if (fp == NULL) {
    perror("error opening file");
    exit(1);
  }
  char line[LINE_LEN];
  while (fgets(line, sizeof(line), fp)) {
    if (strncmp(line, start, strlen(start)) == 0) {
      line[strcspn(line, "\n")] = '\0';
      char *line_copy = strdup(line);
      if (!line_copy) {
        perror("strdup failed");
        for (int i = 0; i < res_count; ++i)
          free(res[i]);
        free(res);
        fclose(fp);
        return NULL;
      }
      char **temp = realloc(res, (res_count + 1) * sizeof(char *));
      if (!line_copy) {
        perror("realloc failed");
        for (int i = 0; i < res_count; ++i)
          free(res[i]);
        free(res);
        fclose(fp);
        return NULL;
      }
      res = temp;
      res[res_count++] = line_copy;
    }
  }
  fclose(fp);
  if (matches)
    *matches = res_count;
  return res;
}
void dl_email(char *id, char *username, int clientFd) {
  FILE *fp;
  fp = fopen(concat_all(3, "db/", id, ".txt"), "r");
  if (fp == NULL) {
    char client_message[] = "no email exists with the provided ID\n";
    if (write(clientFd, client_message, strlen(client_message)) < 0) {
      printf("Write error");
      exit(6);
    }
    return;
  }
  char *user_db_filename = concat_all(3, "db/", username, ".txt");
  int sender_matches;
  int recipient_matches;
  char **is_sender = extract_emails_starting_with_char(
      user_db_filename, concat("email_1: ", id), &sender_matches);
  char **is_recipient = extract_emails_starting_with_char(
      user_db_filename, concat("email_0: ", id), &recipient_matches);
  if (!(sender_matches == 1) && !(recipient_matches == 1)) {
    char client_message[] = "you do not have rights to download this email\n";
    if (write(clientFd, client_message, strlen(client_message)) < 0) {
      printf("Write error");
      exit(6);
    }
    return;
  }
  char good_dl_message[] = "sending file now!\n";
  if (write(clientFd, good_dl_message, strlen(good_dl_message)) < 0) {
    printf("Write error");
    exit(6);
  }
  send_file(fp, clientFd);
}

void send_file(FILE *fp, int sockfd) {
  char data[15000] = {0};
  while (fgets(data, 15000, fp) != NULL) {
    if (send(sockfd, data, strlen(data), 0) == -1) {
      perror("[-]Error in sending file.");
      exit(1);
    }
    bzero(data, 15000);
  }
  close(sockfd);
}
