#include "task.h"
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <fcntl.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <uuid/uuid.h>

static size_t write_cb(char *ptr, size_t size, size_t nmemb, void *data) {
  /* we are not interested in the downloaded bytes itself,
     so we only return the size we would have saved ... */
  (void)ptr;
  (void)data;
  return size * nmemb;
}
static size_t read_callback(char *ptr, size_t size, size_t nmemb,
                            void *userdata) {
  FILE *readhere = (FILE *)userdata;
  curl_off_t nread;

  /* copy as much data as possible into the 'ptr' buffer, but no more than
     'size' * 'nmemb' bytes. */
  size_t retcode = fread(ptr, size, nmemb, readhere);

  nread = (curl_off_t)retcode;

  return retcode;
}
struct memory {
  char *response;
  size_t size;
};

char country[47];
SpeedTestUrls urls;
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
char *concat(const char *s1, const char *s2) {
  char *result = malloc(strlen(s1) + strlen(s2) + 1);
  if (result == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  strcpy(result, s1);
  strcat(result, s2);
  return result;
}
void get_current_location() {
  CURL *curl;
  struct memory chunk = {0};
  CURLcode result;

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "http://ip-api.com/json/\?fields=1");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    result = curl_easy_perform(curl);
    /* Check for errors */
    if (result == CURLE_OK && chunk.response != NULL) {
      cJSON *root = cJSON_Parse(chunk.response);
      if (root != NULL) {
        // Get "country" key directly from root object
        cJSON *country_item = cJSON_GetObjectItemCaseSensitive(root, "country");

        if (cJSON_IsString(country_item) &&
            (country_item->valuestring != NULL)) {
          // Safely copy string into your destination buffer
          snprintf(country, sizeof(country), "%s", country_item->valuestring);
        }
        cJSON_Delete(root); // Always clean up
      }
    } else {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(result));
    }
    curl_easy_cleanup(curl);
  }
}
char *gen_uuid() {
  uuid_t binuuid;
  uuid_generate_random(binuuid);

  char *uuid = malloc(37);

  uuid_unparse_lower(binuuid, uuid);
  uuid_unparse(binuuid, uuid);
  return uuid;
}
int build_download_url(char *buffer, size_t buf_size, const char *host,
                       long size) {
  char *cache_uuid = gen_uuid();
  char *guid = gen_uuid();

  return snprintf(buffer, buf_size,
                  "https://%s/download?nocache=%s&size=%ld&guid=%s", host,
                  cache_uuid, size, guid);
}
int build_upload_url(char *buffer, size_t buf_size, const char *host) {
  char *cache_uuid = gen_uuid();
  char *guid = gen_uuid();

  return snprintf(buffer, buf_size,
                  "https://%s/"
                  "upload?nocache=%s&guid=%s",
                  host, cache_uuid, guid);
}
SpeedTestUrls best_server_by_location(char *location) {
  if (location == NULL) {
    location = country;
  }
  FILE *fp = fopen(JSON_FILE, "r");
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
    free(buffer);
    exit(EXIT_FAILURE);
  }
  buffer[file_size] = '\0';
  fclose(fp);
  cJSON *root = cJSON_Parse(buffer);
  free(buffer);
  if (root == NULL) {
    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL) {
      fprintf(stderr, "Error before: %s\n", error_ptr);
    }
    exit(EXIT_FAILURE);
  }
  if (!cJSON_IsArray(root)) {
    fprintf(stderr, "Expected a JSON array\n");
    cJSON_Delete(root);
    exit(EXIT_FAILURE);
  }
  cJSON *item = NULL;
  double best_time = DBL_MAX;
  int best_id = -1;
  char *best_city = NULL;
  char *best_provider = NULL;
  char *best_host = NULL;
  cJSON_ArrayForEach(item, root) {
    cJSON *c = cJSON_GetObjectItemCaseSensitive(item, "country");
    cJSON *city = cJSON_GetObjectItemCaseSensitive(item, "city");
    cJSON *provider = cJSON_GetObjectItemCaseSensitive(item, "provider");
    cJSON *host = cJSON_GetObjectItemCaseSensitive(item, "host");
    cJSON *id = cJSON_GetObjectItemCaseSensitive(item, "id");
    if (cJSON_IsString(c) && (c->valuestring != NULL) && cJSON_IsString(host) &&
        (host->valuestring != NULL) && cJSON_IsNumber(id)) {
      if (strcmp(location, c->valuestring) == 0) {

        printf("\r\033[KSearching for best server in %s... Testing [%s]",
               location, host->valuestring);
        fflush(stdout);

        CURL *curl = curl_easy_init();
        if (curl) {
          CURLcode result;
          double total;
          char *full_url = concat("https://", host->valuestring);
          curl_easy_setopt(curl, CURLOPT_URL, full_url);
          curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
          curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
          result = curl_easy_perform(curl);
          if (result == CURLE_OK) {
            result = curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total);
            if (result == CURLE_OK) {
              if (total < best_time) {
                best_time = total;
                best_id = id->valueint;

                free(best_city);
                free(best_host);
                free(best_provider);
                best_city = (cJSON_IsString(city) && city->valuestring)
                                ? strdup(city->valuestring)
                                : NULL;
                best_host = strdup(host->valuestring);
                best_provider =
                    (cJSON_IsString(provider) && provider->valuestring)
                        ? strdup(provider->valuestring)
                        : NULL;
              }
            }
          }
          free(full_url);
          curl_easy_cleanup(curl);
        }
      }
    }
  }
  printf("\r\033[K");
  fflush(stdout);
  if (best_host != NULL) {
    printf("\n--- Best Server Match ---\n");
    printf("ID:       %d\n", best_id);
    printf("Host:     %s\n", best_host);
    printf("City:     %s\n", best_city);
    printf("Provider: %s\n", best_provider);
    printf("Latency:  %.3f s\n", best_time);
  } else {
    printf("No matching servers found for location: %s\n", location);
  }
  build_download_url(urls.download_url, sizeof(urls.download_url), best_host,
                     25000000);
  build_upload_url(urls.upload_url, sizeof(urls.upload_url), best_host);
  strcpy(urls.server_name, best_host);
  free(best_city);
  free(best_host);
  free(best_provider);
  cJSON_Delete(root);
  return urls;
}

void download_test(char *url, char *host) {
  if (url == NULL) {
    url = DOWNLOAD_TEST_URL;
  }
  if (host == NULL) {
    host = DOWNLOAD_TEST_URL;
  }
  CURL *curl;
  CURLcode result;
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
      /* check for average download speed */
      result = curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD_T, &val);
      if ((result == CURLE_OK) && (val > 0)) {
        double speed_mbs = ((double)val * 8.0) / (1024.0 * 1024.0);
        printf("\nDownload speed: %f mb/s\n"
               "Upload speed: - mb/s\n"
               "Server name: %s\n"
               "Location of the user: %s\n",
               speed_mbs, host, country);
      }
    } else {
      fprintf(stderr, "Error while fetching '%s' : %s\n", url,
              curl_easy_strerror(result));
    }

    /* cleanup curl stuff */
    curl_easy_cleanup(curl);
  }
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

  fd = fopen(UPLOAD_TEST_FILE, "rb");
  if (!fd) {
    exit(EXIT_FAILURE);
  }

  /* to get the file size */
  if (fstat(fileno(fd), &file_info)) {
    fclose(fd);
    exit(EXIT_FAILURE);
  }

  curl = curl_easy_init();
  if (!curl) {
    perror("curl_easy_init");
    exit(EXIT_FAILURE);
  }
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
      double speed_mbs = ((double)speed_upload * 8.0) / (1024.0 * 1024.0);
      printf("\nDownload speed: - mb/s\n"
             "Upload speed: %f mb/s\n"
             "Server name: %s\n"
             "Location of the user: %s\n",
             speed_mbs, url, country);
    }
    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  fclose(fd);
}
void upload_test_ookla(char *url, char *host) {
  FILE *fd;
  struct stat file_info;
  CURL *curl = curl_easy_init();
  curl_off_t speed_upload, total_time;
  fd = fopen(UPLOAD_TEST_FILE_OOKLA, "rb");
  if (!fd) {
    exit(EXIT_FAILURE);
  }

  /* to get the file size */
  if (fstat(fileno(fd), &file_info)) {
    fclose(fd);
    exit(EXIT_FAILURE);
  }
  if (curl) {
    CURLcode result;
    struct curl_slist *headers = NULL;
    headers =
        curl_slist_append(headers, "Content-Type: application/octet-stream");
    headers = curl_slist_append(headers, "Expect:");
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    curl_easy_setopt(curl, CURLOPT_READDATA, fd);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE,
                     (curl_off_t)file_info.st_size);

    result = curl_easy_perform(curl);
    if (result != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(result));
    } else {
      curl_off_t size;
      /* now extract transfer info */
      curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD_T, &speed_upload);
      curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &total_time);
      curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD_T, &size);
      double speed_mbs = ((double)speed_upload * 8.0) / (1024.0 * 1024.0);
      printf("\nDownload speed: - mb/s\n"
             "Upload speed: %f mb/s\n"
             "Server name: %s\n"
             "Location of the user: %s\n",
             speed_mbs, host, country);
      printf("Uploaded %" CURL_FORMAT_CURL_OFF_T " bytes\n", size);
    }
    curl_easy_cleanup(curl);
  }
  fclose(fd);
}
