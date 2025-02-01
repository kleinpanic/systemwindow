#include <stdio.h>
#include "env_detect.h"
#include "tui.h"
#include "gui.h"
#include "framebuffer.h"

int main(int argc, char *argv[]) {
    Environment env = detect_environment(argc, argv);

    switch (env) {
        case ENV_TTY:
            start_tui();
            break;
        case ENV_GUI:
            start_gui();
            break;
        case ENV_FRAMEBUFFER:
            start_framebuffer_animation();
            break;
        default:
            fprintf(stderr, "Unknown environment. Exiting.\n");
            return 1;
    }

    return 0;
}

