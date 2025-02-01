#ifndef DISK_INFO_H
#define DISK_INFO_H

typedef struct {
    float total;  // Total disk space in GB
    float used;   // Used disk space in GB
} DiskInfo;

// Retrieve disk information
DiskInfo get_disk_info();

#endif

