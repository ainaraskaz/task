#ifndef TASK_H
#define DOWNLOAD_TEST_URL "http://speedtest.tele2.net/10MB.zip"
#define UPLOAD_TEST_URL "http://speedtest.tele2.net/upload.php"
#define UPLOAD_TEST_FILE "data/10MB.zip"
#define JSON_FILE "data/speedtest_server_list.json"
void download_test(char *url);
void upload_test(char *url);
void get_current_location();
void best_server_by_location(char *location);
#endif
