#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define die(e) do { fprintf(stderr, "%s\n", e); exit(EXIT_FAILURE); } while (0);

typedef struct stat META;

char *concat(const char *s1, const char *s2);
char *execute(const char *command);
META *get_meta_data(const char *path);
char **get_words(const char *str);

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("bad arguments");
    return 1;
  }

  char *current_dir = execute("pwd");
  char *filename = argv[1];
  char *path = concat(current_dir, filename);
  time_t last_time_modification = get_meta_data(path)->st_mtime;

  while (1) {
    META *data = get_meta_data(path);
    if (!data) continue;
    time_t time_modification = data->st_mtime;
    if (time_modification != last_time_modification) {
      printf("main: modified!!!\n");
      last_time_modification = time_modification;
    }
    usleep(500000);
  }
  

  return 0;
}

char *concat(const char *s1, const char *s2) {
  char *output = malloc(strlen(s1) + strlen(s2) + 1);
  strcpy(output, s1);
  strcat(output, s2);
  return output;
}

char *execute(const char *command) {
  int fds[2];
  pid_t child_pid;
  char *output;

  if (pipe(fds) == -1)
    die("pipe");

  if ((child_pid = fork()) == -1)
    die("fork");

  if (child_pid == 0) {
    // fds[1] to write
    dup2(fds[1], STDOUT_FILENO); // stdout now fds[1]
    close(fds[0]);
    execlp(command, command, NULL);
    die("execlp");
  } else {
    // fds[0] to read
    wait(NULL);
    close(fds[1]);
    int buffer_size = 1024;
    char *buffer = malloc(buffer_size);
    int count = read(fds[0], buffer, buffer_size);
    output = malloc(count);
    for (int i = 0; i <= count; i++)
      output[i] = buffer[i];
    free(buffer);
    output[count - 1] = '/';
    printf("execute: %d %s\n", count, output);
  }
  return output;
}

META *get_meta_data(const char *path) {
  struct stat *file_meta = malloc(sizeof(struct stat));
  if (stat(path, file_meta) == -1) {
    free(file_meta);
    return NULL;
  }
  return file_meta;
}

char **get_words(const char *str) {
  int start = 0;
  int output_size = 1;
  char **output = malloc(sizeof(char *) * output_size);
  for (int i = 0; str[i]; i++) {
    if (str[i] == ' ' || str[i + 1] == '\0') {
      int size = i - start + (str[i + 1] == '\0' ? 2 : 1);
      char *word = malloc(size);
      for (int j = 0; j < size - 1; j++)
	word[j] = str[start + j];
      word[size - 1] = '\0';
      start = i + 1;

      output[output_size - 1] = word;
      output = realloc(output, ++output_size);
    }
  }

  output[output_size - 1] = NULL;
  
  return output;
}
