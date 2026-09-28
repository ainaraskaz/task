#ifndef TASK_H
#define DOWNLOAD_TEST_URL "http://speedtest.tele2.net/10MB.zip"
#define UPLOAD_TEST_URL "http://speedtest.tele2.net/upload.php"
#define UPLOAD_TEST_FILE "data/10MB.zip"
void download_test(char *url);
void upload_test(char *url);
char *get_current_location();
#endif
