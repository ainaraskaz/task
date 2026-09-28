#include "inout.h"
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>

char *read_file(char *filename) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  fseek(fp, 0, SEEK_END);
  size_t file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char *buffer = (char *)malloc(file_size + 1);

  if (buffer == NULL) {
    perror("malloc");
    fclose(fp);
    exit(EXIT_FAILURE);
  }
  if (fread(buffer, 1, file_size, fp) == 0) {
    perror("fread");
    exit(EXIT_FAILURE);
  }
  buffer[file_size] = '\0';
  fclose(fp);
  return buffer;
}
cJSON *parse_json(char *filename) {
  char *buf = read_file(filename);
  cJSON *json = cJSON_Parse(buf);
  if (json == NULL) {
    perror("cJSON_Parse");
    exit(EXIT_FAILURE);
  }
  free(buf);
  return json;
}
