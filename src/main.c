#include "inout.h"
#include "task.h"
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define JSON_FILENAME "data/speedtest_server_list.json"
#define CHKSPEED_VERSION "1.0"
#define URL_1M "file_1M.bin"
#define URL_2M "file_2M.bin"
#define URL_5M "file_5M.bin"
#define URL_10M "file_10M.bin"
#define URL_20M "file_20M.bin"
#define URL_50M "file_50M.bin"
#define URL_100M "file_100M.bin"
int main(int argc, char *argv[]) {
  CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
  if (res != CURLE_OK) {
    fprintf(stderr, "curl_global_init() failed: %s\n", curl_easy_strerror(res));
    return EXIT_FAILURE;
  }
  int opt;
  int u = 0, d = 0;
  char *url;
  while ((opt = getopt(argc, argv, "U:D:du")) != -1) {
    switch (opt) {
    case 'U':
      url = optarg;
      u = 1;
      break;
    case 'd':
      d = 1;
      break;
    case 'u':
      u = 1;
      break;
    case 'D':
      url = optarg;
      d = 1;
      break;
    default: /* '?' */
      fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }
  get_current_location();
  if (d) {
    download_test(NULL);
  }
  if (u) {
    upload_test(NULL);
  }
  best_server_by_location(NULL);
  curl_global_cleanup();
  return 0;
}
