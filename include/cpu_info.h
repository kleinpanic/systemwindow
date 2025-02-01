#ifndef CPU_INFO_H
#define CPU_INFO_H

typedef struct {
    float usage;         // Overall CPU usage
    int cores;           // Number of cores
    float core_usage[64]; // Usage percentage per core (support up to 64 cores)
    char model[256];     // CPU model name
    float load_avg;      // Load average
} CPUInfo;

// Retrieve CPU information
CPUInfo get_cpu_info();

#endif

