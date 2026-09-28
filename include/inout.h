#ifndef INOUTUTILS_H
#include <cjson/cJSON.h>
char *read_file(char *filename);
cJSON *parse_json(char *filename);
#endif
