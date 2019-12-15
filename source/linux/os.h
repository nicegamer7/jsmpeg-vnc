#ifndef OS_H
#define OS_H

#include <stdbool.h>

bool os_is_display(int display_number);
void os_set_clipboard(char *contents, int display_number);
double os_get_time(void);
void os_sleep(int milliseconds);
void os_save_upload(char *contents, int size, char *filename);
char *os_get_clipboard(int display_number);

#endif
