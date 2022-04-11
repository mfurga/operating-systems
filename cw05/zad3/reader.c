#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DATA_BUFF 1024 * 16
#define READ_BUFF_SIZE 1024
#define WRITE_BUFF_SIZE 1024

char buff[DATA_BUFF];

void write_at_row(FILE *f, char *data, int length, int row) {
  int count = 0, end = 1, size, pos;
  int offset = 0;

  fseek(f, 0, SEEK_END);
  size = ftell(f);
  fseek(f, 0, SEEK_SET);

  (void)!fread(buff, 1, size, f);

  for (pos = 0; pos < size; pos++) {
    count += (buff[pos] == '\n');
    if (count == row) {
      end = 0;
      pos++;
      break;
    }
  }

  if (end) {
    for (int i = 0; i < row - count; i++) {
      buff[pos++] = '\n';
    }
  } else {
    while (pos < size && buff[pos] != '\n') {
      pos++;
    }
    offset = size - pos;
    memmove(buff + pos + length, buff + pos, offset);
  }

  memcpy(buff + pos, data, length);

  fseek(f, 0, SEEK_SET);
  fwrite(buff, 1, pos + length + offset, f);
  fseek(f, 0, SEEK_SET);
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <pipe fifo> <filename> <number of chars>\n", argv[0]);
    return 1;
  }

  int row, num_chars = atoi(argv[3]);
  size_t read, write;

  char read_buff[READ_BUFF_SIZE];
  char write_buff[WRITE_BUFF_SIZE];

  FILE *pipe_handler = fopen(argv[1], "r");
  if (pipe_handler == NULL) {
    fprintf(stderr, "fopen failed.\n");
    return 1;
  }

  FILE *file_handler = fopen(argv[2], "r+");
  if (file_handler == NULL) {
    fprintf(stderr, "fopen failed.\n");
    return 1;
  }

  setvbuf(pipe_handler, NULL, _IOLBF, 0);
  setvbuf(file_handler, NULL, _IOLBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);

  while ((read = fread(read_buff, 1, num_chars + 1, pipe_handler)) > 0) {
    read_buff[read] = '\0';
    row = read_buff[0] - '0';
    write_at_row(file_handler, read_buff + 1, read - 1, row);
  }

  fclose(pipe_handler);
  fclose(file_handler);
  return 0;
}

