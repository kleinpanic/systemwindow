#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <sys/sysinfo.h>
#include "cpu_info.h"
#include "memory_info.h"
#include "disk_info.h"

#define NUM_TABS 4
const char *tabs[NUM_TABS] = {"CPU", "RAM", "Processes", "Disk"};

void draw_top_bar() {
    CPUInfo cpu = get_cpu_info();
    MemoryInfo mem = get_memory_info();

    mvprintw(0, 0, "CPU Usage:");
    for (int i = 0; i < cpu.cores; i++) {
        mvprintw(1, i * 10, "Core %d: %.1f%%", i + 1, cpu.core_usage[i]);
    }

    mvprintw(2, 0, "Memory: %.2f/%.2f GB", mem.used, mem.total);

    struct sysinfo info;
    sysinfo(&info);
    mvprintw(3, 0, "Load Avg: %.2f %.2f %.2f | Uptime: %ld days, %02ld:%02ld:%02ld",
             info.loads[0] / 65536.0, info.loads[1] / 65536.0, info.loads[2] / 65536.0,
             info.uptime / 86400, (info.uptime % 86400) / 3600, (info.uptime % 3600) / 60,
             info.uptime % 60);
}

void draw_tabs(int current_tab) {
    for (int i = 0; i < NUM_TABS; i++) {
        if (i == current_tab) {
            attron(A_REVERSE);
        }
        mvprintw(5, i * 15, "[ %s ]", tabs[i]);
        if (i == current_tab) {
            attroff(A_REVERSE);
        }
    }
}

void draw_cpu_tab() {
    CPUInfo cpu = get_cpu_info();
    mvprintw(7, 0, "CPU Model: %s", cpu.model);
    mvprintw(8, 0, "Cores: %d", cpu.cores);
    for (int i = 0; i < cpu.cores; i++) {
        mvprintw(10 + i, 0, "Core %d: %.2f%%", i + 1, cpu.core_usage[i]);
    }
}

void draw_memory_tab() {
    MemoryInfo mem = get_memory_info();
    mvprintw(7, 0, "Total Memory: %.2f GB", mem.total);
    mvprintw(8, 0, "Used Memory: %.2f GB", mem.used);
    mvprintw(9, 0, "Free Memory: %.2f GB", mem.total - mem.used);
}

void draw_processes_tab() {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        mvprintw(7, 0, "Error: Cannot open /proc/stat");
        return;
    }
    mvprintw(7, 0, "Processes:");
    char line[256];
    int row = 8;
    while (fgets(line, sizeof(line), fp) && row < LINES) {
        mvprintw(row++, 0, "%s", line);
    }
    fclose(fp);
}

void draw_disk_tab() {
    DiskInfo disk = get_disk_info();
    mvprintw(7, 0, "Total Disk Space: %.2f GB", disk.total);
    mvprintw(8, 0, "Used Disk Space: %.2f GB", disk.used);
    mvprintw(9, 0, "Free Disk Space: %.2f GB", disk.total - disk.used);
}

void start_tui() {
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS, NULL);

    int current_tab = 0;
    MEVENT event;

    while (1) {
        clear();
        draw_top_bar();
        draw_tabs(current_tab);

        // Draw current tab
        switch (current_tab) {
            case 0: draw_cpu_tab(); break;
            case 1: draw_memory_tab(); break;
            case 2: draw_processes_tab(); break;
            case 3: draw_disk_tab(); break;
        }

        // Handle input
        int ch = getch();
        if (ch == 'q') break; // Quit
        if (ch == KEY_LEFT || ch == 'h') current_tab = (current_tab - 1 + NUM_TABS) % NUM_TABS;
        if (ch == KEY_RIGHT || ch == 'l') current_tab = (current_tab + 1) % NUM_TABS;
        if (ch == KEY_MOUSE && getmouse(&event) == OK) {
            if (event.y == 5) { // Check mouse click on tabs
                for (int i = 0; i < NUM_TABS; i++) {
                    if (event.x >= i * 15 && event.x < (i + 1) * 15) {
                        current_tab = i;
                    }
                }
            }
        }
        refresh();
        usleep(200000); // Refresh rate: 0.2 seconds
    }

    endwin();
}

