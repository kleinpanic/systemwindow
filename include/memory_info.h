#ifndef MEMORY_INFO_H
#define MEMORY_INFO_H

typedef struct {
    float total;  // Total memory in GB
    float used;   // Used memory in GB
} MemoryInfo;

// Retrieve memory information
MemoryInfo get_memory_info();

#endif

