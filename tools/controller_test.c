// controller_test.c - Test USB controller input on Linux
// Compile: gcc -o controller_test controller_test.c
// Run: ./controller_test

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>

#define JS_DEV "/dev/input/js0"

int main() {
    int js_fd;
    struct js_event e;
    int num_axes = 0;
    int num_buttons = 0;

    printf("USB Controller Test for Linux\n");
    printf("==============================\n\n");

    // Open joystick device
    js_fd = open(JS_DEV, O_RDONLY);
    if (js_fd == -1) {
        perror("Error opening joystick");
        printf("\nTroubleshooting:\n");
        printf("1. Check if controller exists: ls -la %s\n", JS_DEV);
        printf("2. Add user to input group: sudo usermod -a -G input $USER\n");
        printf("3. Then logout and login again\n");
        return 1;
    }

    // Get number of axes and buttons
    ioctl(js_fd, JSIOCGAXES, &num_axes);
    ioctl(js_fd, JSIOCGBUTTONS, &num_buttons);
    char name[128];
    if (ioctl(js_fd, JSIOCGNAME(sizeof(name)), name) < 0) {
        strncpy(name, "Unknown", sizeof(name));
    }

    printf("Controller: %s\n", name);
    printf("Axes: %d\n", num_axes);
    printf("Buttons: %d\n", num_buttons);
    printf("\nPress buttons and move sticks (Ctrl+C to exit)...\n\n");

    // Read events
    while (1) {
        read(js_fd, &e, sizeof(struct js_event));

        switch (e.type) {
            case JS_EVENT_BUTTON:
                printf("Button %d: %s\n", e.number, e.value ? "PRESSED" : "released");
                break;
            case JS_EVENT_AXIS:
                if (e.value != 0) {  // Only show non-zero axis movements
                    printf("Axis %d: %d\n", e.number, e.value);
                }
                break;
            default:
                break;
        }
    }

    close(js_fd);
    return 0;
}
