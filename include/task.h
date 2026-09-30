#ifndef TASK_H
#define TASK_H
#define DOWNLOAD_TEST_URL "http://speedtest.tele2.net/10MB.zip"
#define UPLOAD_TEST_URL "http://speedtest.tele2.net/upload.php"
#define UPLOAD_TEST_FILE "data/10MB.zip"
#define UPLOAD_TEST_FILE_OOKLA "data/25mb.zip"
#define JSON_FILE "data/speedtest_server_list.json"
typedef struct {
  char download_url[512];
  char upload_url[512];
  char server_name[512];
} SpeedTestUrls;
void download_test(char *url, char *host);
void upload_test(char *url);
void get_current_location();
SpeedTestUrls best_server_by_location(char *location);
void upload_test_ookla(char *url, char *host);
#endif
