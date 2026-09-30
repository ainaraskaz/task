#include "task.h"
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
  if (res != CURLE_OK) {
    fprintf(stderr, "curl_global_init() failed: %s\n", curl_easy_strerror(res));
    return EXIT_FAILURE;
  }
  int opt;
  int u = 0, d = 0, D = 0, U = 0, a = 0, l = 0, L = 0;
  char *download_url = NULL;
  char *upload_url = NULL;
  char *location = NULL;
  while ((opt = getopt(argc, argv, "U:D:L:dual")) != -1) {
    switch (opt) {
    case 'U':
      if ((strncmp(optarg, "https://", 8) != 0) &&
          (strncmp(optarg, "http://", 7) != 0)) {
        printf("Please provide full link to the speed test.\n");
        exit(EXIT_FAILURE);
      }
      upload_url = optarg;
      U = 1;
      break;
    case 'u':
      u = 1;
      break;
    case 'D':
      if ((strncmp(optarg, "https://", 8) != 0) &&
          (strncmp(optarg, "http://", 7) != 0)) {
        printf("Please provide full link to the speed test.\n");
        exit(EXIT_FAILURE);
      }
      download_url = optarg;
      D = 1;
      break;
    case 'd':
      d = 1;
      break;
    case 'l':
      l = 1;
      break;
    case 'L':
      location = optarg;
      L = 1;
      break;
    case 'a':
      a = 1;
      break;
    default: /* '?' */
      fprintf(stderr,
              "Usage: %s [-u] [-d] [-a] [-l] [-U url] [-D url] [-L location]\n",
              argv[0]);
      exit(EXIT_FAILURE);
    }
  }
  get_current_location();
  if (d) {
    download_test(NULL, NULL);
  }
  if (D) {
    download_test(download_url, download_url);
  }
  if (u) {
    upload_test(NULL);
  }
  if (U) {
    upload_test(upload_url);
  }
  if (l) {
    best_server_by_location(NULL);
  }
  if (L) {
    best_server_by_location(location);
  }
  if (a) {
    SpeedTestUrls urls;
    urls = best_server_by_location(location);
    upload_test_ookla(urls.upload_url, urls.server_name);
    download_test(urls.download_url, urls.server_name);
  }
  curl_global_cleanup();
  return 0;
}
