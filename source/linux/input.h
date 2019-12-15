#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

#include <X11/X.h>
#include <X11/Xlib.h>

typedef struct {
	Display *display;
	Window window;
} input_t;

input_t *input_create(int display_number);
void input_destroy(input_t *self);
void input_mouse_move(input_t *self, int x, int y);
void input_mouse_left_button(input_t *self, bool down);
void input_mouse_right_button(input_t *self, bool down);
void input_mouse_middle_button(input_t *self, bool down);
void input_mouse_scroll(input_t *self, int amount);
void input_key_press(input_t *self, int key, bool down);
void input_get_cursor_position(input_t *self, int *x, int *y);

#endif
