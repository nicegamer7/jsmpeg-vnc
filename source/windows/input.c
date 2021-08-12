#include "input.h"

input_t *input_create(int display_name) {
    input_t *self = (input_t *) malloc(sizeof(input_t));
    memset(self, 0, sizeof(input_t));

    return self;
}

void input_destroy(input_t *self) {
    if (self != NULL) {
        free(self);
    }
}

void input_mouse_input(int flags) {
    INPUT input;
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = flags;

    SendInput(1, &input, sizeof(INPUT));
}

void input_mouse_move(input_t *self, int x, int y) {
    INPUT input;
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    input.mi.dx = x;
    input.mi.dy = y;

    SendInput(1, &input, sizeof(INPUT));
}

void input_mouse_left_button(input_t *self, bool down) {
    if (down) {
        input_mouse_input(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN);
    } else {
        input_mouse_input(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP);
    }
}

void input_mouse_right_button(input_t *self, bool down) {
    if (down) {
        input_mouse_input(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTDOWN);
    } else {
        input_mouse_input(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTUP);
    }
}

void input_mouse_middle_button(input_t *self, bool down) {
    if (down) {
        input_mouse_input(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MIDDLEDOWN);
    } else {
        input_mouse_input(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MIDDLEUP);
    }
}

void input_mouse_scroll(input_t *self, int amount) {
    INPUT input;
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_WHEEL;

    if (amount > 0) {
        input.mi.mouseData = 1 * WHEEL_DELTA;
    } else {
        input.mi.mouseData = -1 * WHEEL_DELTA;
    }

    SendInput(1, &input, sizeof(INPUT));
}

void input_key_press(input_t *self, int key, bool down) {
    BYTE scan_code = MapVirtualKey(key, 4);
    DWORD flags = 0;

    if (down) {
        flags = KEYEVENTF_SCANCODE | 0;
    } else {
        flags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
    }

    switch (key) {
        case VK_LEFT:
        case VK_UP:
        case VK_RIGHT:
        case VK_DOWN:
        case VK_PRIOR:
        case VK_NEXT:
        case VK_END:
        case VK_HOME:
        case VK_INSERT:
        case VK_DELETE:
        case VK_DIVIDE:
        case VK_NUMLOCK:
            scan_code |= 0x100;
            flags |= KEYEVENTF_EXTENDEDKEY;
            break;
    }

    keybd_event(key, scan_code, flags, 0);
}

void input_get_cursor_position(input_t *self, int *x, int *y) {
    POINT cursor;
    GetCursorPos(&cursor);

    *x = cursor.x;
    *y = cursor.y;
}
