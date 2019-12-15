#ifndef OS_H
#define OS_H

void os_set_clipboard(char *contents);
double os_get_time(void);
int os_sleep(int milliseconds);
void os_save_upload(char *contents, int size, char *filename);

#endif
