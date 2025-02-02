#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <sys/utsname.h>
#include <string.h>
#include <time.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BAR_WIDTH 20

// Function to get CPU temperature
float get_cpu_temperature() {
    FILE *fp;
    float temp;
    fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (fp == NULL) {
        return -1;
    }
    fscanf(fp, "%f", &temp);
    fclose(fp);
    return temp / 1000.0;
}

// Function to get CPU usage
float get_cpu_usage() {
    static long double a[4], b[4];
    FILE *fp;
    fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        return -1;
    }
    fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3]);
    fclose(fp);
    usleep(500000);
    fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        return -1;
    }
    fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &b[0], &b[1], &b[2], &b[3]);
    fclose(fp);
    return ((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3])) * 100;
}

// Function to get the number of CPU cores
int get_cpu_cores() {
    return sysconf(_SC_NPROCESSORS_ONLN);
}

// Function to read a value from a file
void read_value(const char *path, char *value, size_t size) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        strncpy(value, "Unknown", size);
        return;
    }
    fgets(value, size, fp);
    // Remove trailing newline
    value[strcspn(value, "\n")] = 0;
    fclose(fp);
}

// Function to get battery info
void get_battery_info(WINDOW *win) {
    char path[256];
    char value[256];

    snprintf(path, sizeof(path), "/sys/class/power_supply/BAT0/model_name");
    read_value(path, value, sizeof(value));
    wprintw(win, "  Model Name: %s\n", value);

    snprintf(path, sizeof(path), "/sys/class/power_supply/BAT0/manufacturer");
    read_value(path, value, sizeof(value));
    wprintw(win, "  Manufacturer: %s\n", value);

    snprintf(path, sizeof(path), "/sys/class/power_supply/BAT0/serial_number");
    read_value(path, value, sizeof(value));
    wprintw(win, "  Serial Number: %s\n", value);

    snprintf(path, sizeof(path), "/sys/class/power_supply/BAT0/capacity");
    read_value(path, value, sizeof(value));
    wprintw(win, "  Capacity: %s%%", value);

    snprintf(path, sizeof(path), "/sys/class/power_supply/BAT0/status");
    read_value(path, value, sizeof(value));
    wprintw(win, " (Status: %s)\n", value);

    snprintf(path, sizeof(path), "/sys/class/power_supply/BAT0/charge_now");
    read_value(path, value, sizeof(value));
    wprintw(win, "  Charge Now: %s\n", value);

    snprintf(path, sizeof(path), "/sys/class/power_supply/BAT0/charge_full");
    read_value(path, value, sizeof(value));
    wprintw(win, "  Charge Full: %s\n", value);

    snprintf(path, sizeof(path), "/sys/class/power_supply/BAT0/cycle_count");
    read_value(path, value, sizeof(value));
    wprintw(win, "  Cycle Count: %s\n", value);

    snprintf(path, sizeof(path), "/sys/class/power_supply/BAT0/voltage_now");
    read_value(path, value, sizeof(value));
    wprintw(win, "  Voltage Now: %s uV\n", value);

    snprintf(path, sizeof(path), "/sys/class/power_supply/BAT0/current_now");
    read_value(path, value, sizeof(value));
    wprintw(win, "  Current Now: %s uA\n", value);

    // Calculate voltage consumption (assuming voltage_now * current_now / 1,000,000 for mW)
    long voltage_now = atol(value);
    snprintf(path, sizeof(path), "/sys/class/power_supply/BAT0/current_now");
    read_value(path, value, sizeof(value));
    long current_now = atol(value);
    long power_consumption = (voltage_now * current_now) / 1000000;
    wprintw(win, "  Power Consumption: %ld mW\n", power_consumption);

    // Retrieve battery temperature using sensors command
    FILE *fp = popen("sensors | awk '/Package id 0/ {gsub(/[^0-9.]/, \"\", $4); print int($4)}'", "r");
    if (fp == NULL) {
        wprintw(win, "  Temperature: Unknown\n");
    } else {
        fgets(value, sizeof(value), fp);
        wprintw(win, "  Temperature(°C): %s \n", value);
        pclose(fp);
    }
}

// Function to get disk usage
void get_disk_usage(long *total, long *used, float *usage_percentage) {
    struct statvfs stat;
    if (statvfs("/", &stat) != 0) {
        *total = -1;
        *used = -1;
        *usage_percentage = -1;
        return;
    }
    *total = stat.f_blocks * stat.f_frsize;
    *used = (stat.f_blocks - stat.f_bfree) * stat.f_frsize;
    *usage_percentage = 100.0 * (*used) / (*total);
}

// Function to get driver information
void get_driver_info(char *driver, size_t size, const char *grep_pattern) {
    FILE *fp;
    char command[256];
    snprintf(command, sizeof(command), "lspci -k | grep -A 3 '%s' | grep 'Kernel driver in use' | cut -d ':' -f2", grep_pattern);
    fp = popen(command, "r");
    if (fp == NULL) {
        strncpy(driver, "Unknown", size);
        return;
    }
    fgets(driver, size, fp);
    pclose(fp);
    // Remove trailing newline
    driver[strcspn(driver, "\n")] = 0;
}

// Function to get private IP address
void get_private_ip(char *ip, size_t size) {
    struct ifaddrs *ifaddr, *ifa;
    int family;
    if (getifaddrs(&ifaddr) == -1) {
        strncpy(ip, "Unknown", size);
        return;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET) {
            getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), ip, size, NULL, 0, NI_NUMERICHOST);
            if (strncmp(ip, "127.", 4) != 0) break;
        }
    }

    freeifaddrs(ifaddr);
}

// Function to get public IP address
void get_public_ip(char *ip, size_t size) {
    FILE *fp = popen("curl -s ifconfig.me", "r");
    if (fp == NULL) {
        strncpy(ip, "Unknown", size);
        return;
    }
    fgets(ip, size, fp);
    pclose(fp);
    // Remove trailing newline
    ip[strcspn(ip, "\n")] = 0;
}

// Function to get CPU model name
void get_cpu_model(char *model, size_t size) {
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (fp == NULL) {
        strncpy(model, "Unknown", size);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "model name", 10) == 0) {
            strncpy(model, line + 13, size);
            // Remove trailing newline
            model[strcspn(model, "\n")] = 0;
            break;
        }
    }
    fclose(fp);
}

// Function to get the top CPU-consuming process
void get_top_cpu_process(char *process, size_t size) {
    FILE *fp = popen("ps -eo %cpu,%mem,comm --sort=-%cpu | head -2 | tail -1", "r");
    if (fp == NULL) {
        strncpy(process, "Error getting top CPU process", size);
        return;
    }

    fgets(process, size, fp);
    pclose(fp);
    // Remove trailing newline
    process[strcspn(process, "\n")] = 0;
}

// Function to get sound driver
void get_sound_driver(char *driver, size_t size) {
    FILE *fp;
    fp = popen("cat /proc/asound/cards | grep -m 1 -oP '\\[.*\\]: \\K.*'", "r");
    if (fp == NULL) {
        strncpy(driver, "Unknown", size);
        return;
    }
    fgets(driver, size, fp);
    pclose(fp);
    driver[strcspn(driver, "\n")] = 0;
}

// Function to get the number of installed packages
void get_package_counts(int *apt_count, int *dpkg_count, int *flatpak_count) {
    FILE *fp;

    fp = popen("apt list --installed 2>/dev/null | wc -l", "r");
    if (fp == NULL) {
        *apt_count = -1;
    } else {
        fscanf(fp, "%d", apt_count);
        pclose(fp);
    }

    fp = popen("dpkg -l | wc -l", "r");
    if (fp == NULL) {
        *dpkg_count = -1;
    } else {
        fscanf(fp, "%d", dpkg_count);
        pclose(fp);
    }

    fp = popen("flatpak list | wc -l", "r");
    if (fp == NULL) {
        *flatpak_count = -1;
    } else {
        fscanf(fp, "%d", flatpak_count);
        pclose(fp);
    }
}

// Function to display bar graph
void display_bar(WINDOW *win, float percentage) {
    int bar_length = (percentage / 100.0) * BAR_WIDTH;
    wprintw(win, "[");
    for (int i = 0; i < BAR_WIDTH; i++) {
        if (i < bar_length) {
            wprintw(win, "#");
        } else {
            wprintw(win, " ");
        }
    }
    wprintw(win, "] %.2f%%\n", percentage);
}

// Function to display all information
void display_info(WINDOW *win) {
    struct sysinfo info;
    sysinfo(&info);

    char hostname[256];
    gethostname(hostname, sizeof(hostname));

    struct utsname buffer;
    if (uname(&buffer) != 0) {
        wprintw(win, "Error getting system info\n");
    }

    char debian_version[256];
    read_value("/etc/debian_version", debian_version, sizeof(debian_version));

    char *shell = getenv("SHELL");

    wprintw(win, "Hostname: %s | OS: %s %s | Debian: %s | Shell: %s\n", hostname, buffer.sysname, buffer.release, debian_version, shell ? shell : "Unknown");

    char private_ip[NI_MAXHOST];
    get_private_ip(private_ip, sizeof(private_ip));

    char public_ip[NI_MAXHOST];
    get_public_ip(public_ip, sizeof(public_ip));

    wprintw(win, "Private IP: %s | Public IP: %s\n", private_ip, public_ip);

    char driver[256];
    get_driver_info(driver, sizeof(driver), "Ethernet controller");
    wprintw(win, "Network Driver: %s\n", driver);

    get_sound_driver(driver, sizeof(driver));
    wprintw(win, "Sound Driver: %s\n", driver);

    int apt_count, dpkg_count, flatpak_count;
    get_package_counts(&apt_count, &dpkg_count, &flatpak_count);
    wprintw(win, "APT installs: %d | DPKG installs: %d | Flatpak installs: %d\n", apt_count, dpkg_count, flatpak_count);

    char cpu_model[256];
    get_cpu_model(cpu_model, sizeof(cpu_model));
    wprintw(win, "CPU Model: %s\n", cpu_model);

    float cpu_temp = get_cpu_temperature();
    float cpu_usage = get_cpu_usage();
    int cpu_cores = get_cpu_cores();

    long total_disk, used_disk;
    float disk_usage_percentage;
    get_disk_usage(&total_disk, &used_disk, &disk_usage_percentage);

    // Format uptime
    int days = info.uptime / 86400;
    int hours = (info.uptime % 86400) / 3600;
    int minutes = (info.uptime % 3600) / 60;

    wprintw(win, "Uptime: %d days, %d hours, %d minutes\n", days, hours, minutes);

    wprintw(win, "RAM:\n");
    wprintw(win, "  Total: %ld MB\n", info.totalram / 1024 / 1024);
    wprintw(win, "  Free: %ld MB\n", info.freeram / 1024 / 1024);
    wprintw(win, "  Used: %ld MB\n", (info.totalram - info.freeram) / 1024 / 1024);
    wprintw(win, "  Usage: ");
    display_bar(win, 100.0 * (info.totalram - info.freeram) / info.totalram);

    wprintw(win, "CPU:\n");
    wprintw(win, "  Usage: ");
    display_bar(win, cpu_usage);
    wprintw(win, "  Cores: %d\n", cpu_cores);
    wprintw(win, "  Temperature: %.2f°C\n", cpu_temp);

    // Read CPU speed, cache size, and load average
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (fp != NULL) {
        char line[256];
        int speed_displayed = 0;
        int cache_displayed = 0;

        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "cpu MHz", 7) == 0 && !speed_displayed) {
                char *newline_pos = strchr(line, '\n');
                if (newline_pos != NULL) {
                    *newline_pos = ' '; // Replace newline with space
                }
                wprintw(win, "  Speed: %s MHz\n", line + 11);
                speed_displayed = 1;
            } else if (strncmp(line, "cache size", 10) == 0 && !cache_displayed) {
                wprintw(win, "  Cache Size: %s", line + 13);
                cache_displayed = 1;
            }

            if (speed_displayed && cache_displayed) {
                break;
            }
        }
        fclose(fp);
    }

    double loadavg[3];
    if (getloadavg(loadavg, 3) != -1) {
        wprintw(win, "  Load Average: [1 min: %.2f, 5 min: %.2f, 15 min: %.2f]\n", loadavg[0], loadavg[1], loadavg[2]);
    }

    char top_cpu_process[256];
    get_top_cpu_process(top_cpu_process, sizeof(top_cpu_process));
    wprintw(win, "  Top CPU Process: %s\n", top_cpu_process);

    wprintw(win, "Disk:\n");
    wprintw(win, "  Total: %ld MB\n", total_disk / 1024 / 1024);
    wprintw(win, "  Used: %ld MB\n", used_disk / 1024 / 1024);
    wprintw(win, "  Free: %ld MB\n", (total_disk - used_disk) / 1024 / 1024);
    wprintw(win, "  Usage: ");
    display_bar(win, disk_usage_percentage);

    wprintw(win, "Battery:\n");
    get_battery_info(win);

    wrefresh(win);
}

int main() {
    initscr();
    noecho();
    curs_set(FALSE);

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    WINDOW *main_win = newwin(max_y, max_x, 0, 0);

    while (1) {
        wclear(main_win);
        display_info(main_win);
        usleep(500000);  // Sleep for 500 milliseconds to reduce CPU usage
    }

    delwin(main_win);
    endwin();

    return 0;
}
