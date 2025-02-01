#include <stdio.h>
#include "memory_info.h"

MemoryInfo get_memory_info() {
    MemoryInfo info = {0};

    // Parse /proc/meminfo
    FILE *meminfo = fopen("/proc/meminfo", "r");
    if (meminfo) {
        char line[256];
        long total, free, buffers, cached;
        while (fgets(line, sizeof(line), meminfo)) {
            if (sscanf(line, "MemTotal: %ld kB", &total) == 1) {
                info.total = total / 1024.0 / 1024.0;
            } else if (sscanf(line, "MemFree: %ld kB", &free) == 1) {
                // Additional fields for better accuracy
                long available = free;
                if (sscanf(line, "Buffers: %ld kB", &buffers) == 1) {
                    available += buffers;
                }
                if (sscanf(line, "Cached: %ld kB", &cached) == 1) {
                    available += cached;
                }
                info.used = info.total - (available / 1024.0 / 1024.0);
                break;
            }
        }
        fclose(meminfo);
    }

    return info;
}

