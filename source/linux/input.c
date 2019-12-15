#include <stdlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#include <string.h>
#include <stdio.h>

#include "input.h"

input_t *input_create(int display_number) {

	input_t *self = (input_t *) malloc(sizeof(input_t));
  	memset(self, 0, sizeof(input_t));

    char display_name[100] = {0};
    if (display_number > 0) {
        sprintf(display_name, ":%d", display_number);
    }

    self->display = XOpenDisplay(display_name);
    if (self->display == NULL) {
        free(self);

        return NULL;
    }

	self->window = XDefaultRootWindow(self->display);

	return self;
}

void input_destroy(input_t *self) {

   if (self != NULL) {
        if (self->display != NULL) {
            XCloseDisplay(self->display);
        }

		free(self);
  	}
}

void input_mouse_move(input_t *self, int x, int y) {
    XWarpPointer(self->display, None, self->window, 0, 0, 0, 0, x, y);
    XFlush(self->display);
}

void input_mouse_left_button(input_t *self, bool down) {
	XTestFakeButtonEvent(self->display, Button1, down, CurrentTime);
	XFlush(self->display);
}

void input_mouse_right_button(input_t *self, bool down) {
	XTestFakeButtonEvent(self->display, Button3, down, CurrentTime);
	XFlush(self->display);
}

void input_mouse_middle_button(input_t *self, bool down) {
	XTestFakeButtonEvent(self->display, Button2, down, CurrentTime);
	XFlush(self->display);
}

void input_get_cursor_position(input_t *self, int *x, int *y) {

    Window root_return, child_return;
    int win_x, win_y;
    unsigned int mask;

    XQueryPointer(self->display, self->window, &root_return, &child_return, x, y, &win_x, &win_y, &mask);
}

void input_mouse_scroll(input_t *self, int amount) {

	if (amount >= 0) {
		XTestFakeButtonEvent(self->display, Button4, 1, CurrentTime);
		XTestFakeButtonEvent(self->display, Button4, 0, CurrentTime);
	} else {
		XTestFakeButtonEvent(self->display, Button5, 1, CurrentTime);
		XTestFakeButtonEvent(self->display, Button5, 0, CurrentTime);
	}

	XFlush(self->display);
}

void input_key_press(input_t *self, int key, bool down) {

	KeySym symbol;
	KeyCode code;

	switch (key) {
		case  8: symbol = XK_BackSpace; break;
		case  9: symbol = XK_Tab; break;
		case 12: symbol = XK_numbersign; break;
		case 13: symbol = XK_Return; break;
		case 16: symbol = XK_Shift_L; break;
		case 17: symbol = XK_Control_L; break;
		case 18: symbol = XK_Alt_L; break;
		case 20: symbol = XK_Caps_Lock; break;
		case 27: symbol = XK_Escape; break;
		case 37: symbol = XK_Left; break;
		case 38: symbol = XK_Up; break;
		case 39: symbol = XK_Right; break;
		case 40: symbol = XK_Down; break;
	    case 46: symbol = XK_Delete; break;
	    case 96: symbol = XK_KP_0; break;
	    case 97: symbol = XK_KP_1; break;
	    case 98: symbol = XK_KP_2; break;
	    case 99: symbol = XK_KP_3; break;
	    case 100: symbol = XK_KP_4; break;
	    case 101: symbol = XK_KP_5; break;
	    case 102: symbol = XK_KP_6; break;
	    case 103: symbol = XK_KP_7; break;
	    case 104: symbol = XK_KP_8; break;
	    case 105: symbol = XK_KP_9; break;
	    case 107: symbol = XK_KP_Add; break;
	    case 109: symbol = XK_KP_Subtract; break;
	    case 110: symbol = XK_KP_Decimal; break;
	    case 112: symbol = XK_F1; break;
	    case 113: symbol = XK_F2; break;
	    case 114: symbol = XK_F3; break;
	    case 115: symbol = XK_F4; break;
	    case 116: symbol = XK_F5; break;
	    case 117: symbol = XK_F6; break;
	    case 118: symbol = XK_F7; break;
	    case 119: symbol = XK_F8; break;
	    case 120: symbol = XK_F9; break;
	    case 121: symbol = XK_F10; break;
	    case 122: symbol = XK_F11; break;
	    case 123: symbol = XK_F12; break;
		case 144: symbol = XK_Num_Lock; break;
   		case 145: symbol = XK_Scroll_Lock; break;
		case 186: symbol = XK_colon; break;
		case 187: symbol = XK_equal; break;
		case 188: symbol = XK_comma; break;
		case 189: symbol = XK_minus; break;
		case 190: symbol = XK_period; break;
		case 191: symbol = XK_slash; break;
		case 192: symbol = XK_apostrophe; break;
		case 219: symbol = XK_bracketleft; break;
		case 220: symbol = XK_backslash; break;
		case 221: symbol = XK_bracketright; break;
		case 222: symbol = XK_numbersign; break;
		case 223: symbol = XK_grave; break;

		default: symbol = key; break;
	}

	code = XKeysymToKeycode(self->display, symbol);

	if (code != 0) {
		XTestFakeKeyEvent(self->display, code, down, 0);
		XFlush(self->display);
	}
}
