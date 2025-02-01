To model your program, you can adopt a modular architecture with clear separation of concerns. Here’s a proposed structure:

### 1. **Project Structure**
Organize your program into directories for better maintainability:

```
system_hub/
├── src/                 # Source files
│   ├── main.c           # Entry point of the program
│   ├── env_detect.c     # Detects the running environment
│   ├── tui.c            # Handles the Terminal User Interface
│   ├── gui.c            # Handles the Graphical User Interface
│   ├── framebuffer.c    # Handles framebuffer animations
│   ├── cpu_info.c       # Module for CPU monitoring
│   ├── memory_info.c    # Module for Memory monitoring
│   ├── disk_info.c      # Module for Disk monitoring
│   └── ...              # Add more as needed (e.g., network_info.c, processes.c)
├── include/             # Header files
│   ├── env_detect.h
│   ├── tui.h
│   ├── gui.h
│   ├── framebuffer.h
│   ├── cpu_info.h
│   ├── memory_info.h
│   └── ...
├── obj/                 # Compiled object files
├── build/               # Binary outputs
├── Makefile             # Build instructions
└── README.md            # Documentation
```

### 2. **Core Features**
#### a. **Environment Detection (`env_detect.c`)**
Detect whether the program is running in:
1. A TTY (Text-only environment).
2. A graphical environment (X11 or Wayland).
3. A framebuffer (no graphical environment).

Use system calls and environment variables:
- Check `DISPLAY` or `WAYLAND_DISPLAY` for GUI.
- Check framebuffer devices (`/dev/fb0`) for framebuffer animation.
- Default to TTY otherwise.

#### b. **Modular Monitoring Components**
Each system information aspect (CPU, memory, disk, etc.) should have its own module:
- **CPU**: Use `/proc/stat` for usage, `/sys/devices/system/cpu` for core details.
- **Memory**: Use `/proc/meminfo`.
- **Disk**: Use `statvfs` for filesystem stats.
- **Network**: Use `/proc/net/dev` for traffic stats.
- **Processes**: Use `/proc` for process information.

#### c. **Interface Modules**
1. **TUI (`tui.c`)**: Create a text-based dashboard using `ncurses`.
2. **GUI (`gui.c`)**: Use a lightweight library like GTK or SDL for graphical visualization.
3. **Framebuffer (`framebuffer.c`)**: Render animations directly to `/dev/fb0` for non-graphical environments.

Each interface module should have functions to:
- Initialize the environment.
- Display system stats in the respective interface.
- Handle user inputs or refresh cycles.

#### d. **Daemon Mode**
Design the program to run indefinitely:
- Use a main event loop to refresh data and update the UI.
- Implement signal handling (`SIGINT`, `SIGTERM`) for graceful shutdown.
- Use periodic timers (`select`, `poll`, or `epoll`) to update data at intervals.

### 3. **Main Program Flow (`main.c`)**
```c
#include "env_detect.h"
#include "tui.h"
#include "gui.h"
#include "framebuffer.h"

int main(int argc, char *argv[]) {
    // Detect environment
    Environment env = detect_environment();

    // Start appropriate interface
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
```

### 4. **Development Steps**
1. **Set Up Environment Detection**: Write a function to detect GUI, TTY, or framebuffer.
2. **Build System Information Modules**: Implement CPU, memory, and disk monitoring.
3. **Create TUI Interface**: Use `ncurses` for a simple dashboard.
4. **Add GUI Support**: Use GTK or SDL for graphical environments.
5. **Implement Framebuffer Animation**: Use direct rendering to `/dev/fb0`.
6. **Integrate Modules**: Combine all components in `main.c`.

### 5. **Tools and Libraries**
- **TUI**: `ncurses`
- **GUI**: GTK, SDL, or similar
- **Framebuffer**: Direct pixel manipulation
- **System Information**: Linux `/proc` and `/sys` files

### 6. **Future Enhancements**
- Add remote monitoring via a web interface.
- Log data to files for historical analysis.
- Add support for plugin modules for extensibility.

Let me know which part you'd like to tackle first!
