#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "env_detect.h"

Environment detect_environment(int argc, char *argv[]) {
    // Check for command-line flags
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--gui") == 0) {
            return ENV_GUI;
        }
    }

    // Check if running in TTY1
    if (isatty(STDIN_FILENO)) {
        char tty_name[128];
        if (readlink("/proc/self/fd/0", tty_name, sizeof(tty_name)) != -1) {
            if (strstr(tty_name, "tty1")) {
                return ENV_FRAMEBUFFER;
            }
        }
        return ENV_TTY; // Default for terminal
    }

    // Check for GUI environment
    if (getenv("DISPLAY") != NULL || getenv("WAYLAND_DISPLAY") != NULL) {
        return ENV_GUI;
    }

    // Check for framebuffer
    if (access("/dev/fb0", F_OK) == 0) {
        return ENV_FRAMEBUFFER;
    }

    return ENV_UNKNOWN;
}

