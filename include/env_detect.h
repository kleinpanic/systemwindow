#ifndef ENV_DETECT_H
#define ENV_DETECT_H

typedef enum {
    ENV_TTY,
    ENV_GUI,
    ENV_FRAMEBUFFER,
    ENV_UNKNOWN
} Environment;

Environment detect_environment(int argc, char *argv[]);

#endif

