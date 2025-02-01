#include <sys/statvfs.h>
#include "disk_info.h"

DiskInfo get_disk_info() {
    DiskInfo info = {0};

    struct statvfs stat;
    if (statvfs("/", &stat) == 0) {
        info.total = (stat.f_blocks * stat.f_frsize) / 1024.0 / 1024.0 / 1024.0;
        info.used = ((stat.f_blocks - stat.f_bfree) * stat.f_frsize) / 1024.0 / 1024.0 / 1024.0;
    }

    return info;
}

