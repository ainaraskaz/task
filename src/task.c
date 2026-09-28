#include "task.h"
#include <curl/curl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static size_t write_cb(char *ptr, size_t size, size_t nmemb, void *data) {
  /* we are not interested in the downloaded bytes itself,
     so we only return the size we would have saved ... */
  (void)ptr;
  (void)data;
  return size * nmemb;
}
struct memory {
  char *response;
  size_t size;
};

static size_t cb(char *data, size_t size, size_t nmemb, void *clientp) {
  size_t realsize = nmemb;
  struct memory *mem = (struct memory *)clientp;

  char *ptr = realloc(mem->response, mem->size + realsize + 1);
  if (!ptr)
    return 0; /* out of memory */

  mem->response = ptr;
  memcpy(&(mem->response[mem->size]), data, realsize);
  mem->size += realsize;
  mem->response[mem->size] = 0;

  return realsize;
}
void download_test(char *url) {
  if (url == NULL) {
    url = DOWNLOAD_TEST_URL;
  }
  CURL *curl;
  CURLcode result;
  result = curl_global_init(CURL_GLOBAL_ALL);
  if (result != CURLE_OK)
    exit(EXIT_FAILURE);

  /* init the curl session */
  curl = curl_easy_init();
  if (curl) {

    /* specify URL to get */
    curl_easy_setopt(curl, CURLOPT_URL, url);

    /* send all data to this function */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

    /* some servers do not like requests that are made without a user-agent
       field, so we provide one */
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-speedchecker/1.0");

    /* get it! */
    result = curl_easy_perform(curl);

    if (result == CURLE_OK) {
      curl_off_t val;

      /* check for bytes downloaded */
      result = curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD_T, &val);
      if ((result == CURLE_OK) && (val > 0)) {
        long mb = (val / 1024) / 1024;
        printf("\nData downloaded: %ld mbytes.\n", mb);
      }
      /* check for total download time */
      result = curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &val);
      if ((result == CURLE_OK) && (val > 0))
        printf("Total download time: %" CURL_FORMAT_CURL_OFF_T
               ".%06" CURL_FORMAT_CURL_OFF_T " sec.\n",
               val / 1000000, val % 1000000);

      /* check for average download speed */
      result = curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD_T, &val);
      if ((result == CURLE_OK) && (val > 0)) {
        long speed_mbs = (val / 1024) / 1024;
        printf("Average download speed: %ld mbyte/sec.\n", speed_mbs);
      }
    } else {
      fprintf(stderr, "Error while fetching '%s' : %s\n", url,
              curl_easy_strerror(result));
    }

    /* cleanup curl stuff */
    curl_easy_cleanup(curl);
  }

  /* we are done with libcurl, so clean it up */
  curl_global_cleanup();
}
void upload_test(char *url) {
  if (url == NULL) {
    url = UPLOAD_TEST_URL;
  }
  CURL *curl;
  CURLcode result;
  struct stat file_info;
  curl_off_t speed_upload, total_time;
  FILE *fd;

  result = curl_global_init(CURL_GLOBAL_ALL);
  if (result != CURLE_OK)
    exit(EXIT_FAILURE);

  fd = fopen(UPLOAD_TEST_FILE, "rb");
  if (!fd) {
    curl_global_cleanup();
    exit(EXIT_FAILURE);
  }

  /* to get the file size */
  if (fstat(fileno(fd), &file_info)) {
    fclose(fd);
    curl_global_cleanup();
    exit(EXIT_FAILURE);
  }

  curl = curl_easy_init();
  if (curl) {
    /* upload to this place */
    curl_easy_setopt(curl, CURLOPT_URL, url);

    /* tell it to "upload" to the URL */
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    /* set where to read from (on Windows you need to use READFUNCTION too) */
    curl_easy_setopt(curl, CURLOPT_READDATA, fd);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

    /* and give the size of the upload (optional) */
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                     (curl_off_t)file_info.st_size);

    result = curl_easy_perform(curl);
    /* Check for errors */
    if (result != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(result));
    } else {
      /* now extract transfer info */
      curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD_T, &speed_upload);
      curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &total_time);
      long speed_mbs = (speed_upload / 1024) / 1024;

      printf("\nSpeed: %ld mbytes/sec during "
             "%" CURL_FORMAT_CURL_OFF_T ".%06" CURL_FORMAT_CURL_OFF_T
             " seconds\n",
             speed_mbs, total_time / 1000000, total_time % 1000000);
    }
    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  fclose(fd);
  curl_global_cleanup();
}
char *get_current_location() {
  CURL *curl;
  struct memory chunk = {0};
  CURLcode result = curl_global_init(CURL_GLOBAL_ALL);
  char *country = malloc(47);
  if (result != CURLE_OK)
    exit(EXIT_FAILURE);

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "http://ip-api.com/json/\?fields=1");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    result = curl_easy_perform(curl);
    /* Check for errors */
    if (result != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(result));
    } else {
      printf("%s\n", chunk.response);
      int count = 0;
      int j = 0;
      // extract the country name :P
      for (ulong i = 0; i < strlen(chunk.response); i++) {
        if (chunk.response[i] == '"') {
          count++;
          continue;
        }
        if (count == 3) {
          country[j] = chunk.response[i];
          j++;
        }
      }
      country[j] = '\0';
      printf("%s\n", country);
    }
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
  return country;
}
