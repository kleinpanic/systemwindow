#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cpu_info.h"

CPUInfo get_cpu_info() {
    CPUInfo info = {0};

    // Get CPU model
    FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
    if (cpuinfo) {
        char line[256];
        while (fgets(line, sizeof(line), cpuinfo)) {
            if (sscanf(line, "model name : %[^\n]", info.model) == 1) {
                break;
            }
        }
        fclose(cpuinfo);
    }

    // Get number of cores
    FILE *stat = fopen("/proc/stat", "r");
    if (stat) {
        char line[256];
        while (fgets(line, sizeof(line), stat)) {
            if (strncmp(line, "cpu ", 4) == 0) {
                // Calculate CPU usage (simplified example)
                int user, nice, system, idle;
                sscanf(line, "cpu %d %d %d %d", &user, &nice, &system, &idle);
                info.usage = 100.0 - (idle * 100.0 / (user + nice + system + idle));
                break;
            }
        }
        fclose(stat);
    }

    // Count cores
    FILE *cores = fopen("/proc/stat", "r");
    if (cores) {
        char line[256];
        while (fgets(line, sizeof(line), cores)) {
            if (strncmp(line, "cpu", 3) == 0 && line[3] != ' ') {
                info.cores++;
            }
        }
        fclose(cores);
    }

    return info;
}

