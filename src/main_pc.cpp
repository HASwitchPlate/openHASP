/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if HASP_TARGET_PC

#if defined(WINDOWS)

#include <windows.h>
#include <direct.h>
#include <shlobj.h>
#include <shlwapi.h>
// MSDN recommends against using getcwd & chdir names
#define cwd _getcwd
#define cd _chdir
#endif

#if defined(POSIX)
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <pwd.h>
#define cwd getcwd
#define cd chdir
#endif

#include <cstdlib>
#include <iostream>

#include "hasplib.h"

#if USE_MONITOR
#include "display/monitor.h"
#endif

#include "hasp_debug.h"

// hasp_gui.cpp
extern uint16_t tft_width;
extern uint16_t tft_height;

// main.cpp
extern void setup();
extern void loop();

#if defined(WINDOWS)
// https://gist.github.com/kingseva/a918ec66079a9475f19642ec31276a21
void BindStdHandlesToConsole()
{
    // TODO: Add Error checking.

    // Redirect the CRT standard input, output, and error handles to the console
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stderr);
    freopen("CONOUT$", "w", stdout);

    // Note that there is no CONERR$ file
    HANDLE hStdout = CreateFile("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE hStdin  = CreateFile("CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    SetStdHandle(STD_OUTPUT_HANDLE, hStdout);
    SetStdHandle(STD_ERROR_HANDLE, hStdout);
    SetStdHandle(STD_INPUT_HANDLE, hStdin);

    // Clear the error state for each of the C++ standard stream objects.
    std::wclog.clear();
    std::clog.clear();
    std::wcout.clear();
    std::cout.clear();
    std::wcerr.clear();
    std::cerr.clear();
    std::wcin.clear();
    std::cin.clear();
}

void InitializeConsoleOutput()
{
    bool isConsoleApp;

    // How to check if the program is run from a console?
    // https://stackoverflow.com/questions/9009333/how-to-check-if-the-program-is-run-from-a-console
    HWND consoleWnd = GetConsoleWindow();
    DWORD dwProcessId;
    GetWindowThreadProcessId(consoleWnd, &dwProcessId);

    if(GetCurrentProcessId() == dwProcessId) {
        isConsoleApp = true; // Opened in Console
    } else {
        isConsoleApp = false; // Opened in Windows
    }

    // Use normal console that launched the program
    AttachConsole(ATTACH_PARENT_PROCESS);

    // Test if the application was started from a Console Window
    if(!isConsoleApp) {
        // If started from Windows, use detached Log Console Window
        AllocConsole();
    }

    // Redirect all standard output streams to the console
    BindStdHandlesToConsole();
}
#endif

void usage(const char* progName, const char* version)
{
    std::cout << "\n"
              << progName << " " << version << " [options]" << std::endl
              << std::endl
              << "Options:" << std::endl
              << "    -h  | --help        Print this help" << std::endl
              << "    -q  | --quiet       Suppress console output (can improve performance)" << std::endl
#if !USE_FBDEV
              << "    -W  | --width       Width of the window" << std::endl
              << "    -H  | --height      Height of the window" << std::endl
#endif
              << "    -c  | --config      Configuration/storage directory" << std::endl
#if defined(WINDOWS)
              << "                        (default: 'AppData\\hasp\\hasp')" << std::endl
#elif defined(POSIX)
              << "                        (default: '~/.local/share/hasp/hasp')" << std::endl
#endif
              << std::endl;
    fflush(stdout);
}

int main(int argc, char* argv[])
{
    bool showhelp         = false;
    bool console          = true;
    char config[PATH_MAX] = {'\0'};

#if defined(WINDOWS)
    InitializeConsoleOutput();
    SetConsoleCP(65001); // 65001 = UTF-8
#endif

    for(int arg = 1; arg < argc; arg++) {
        if(strncmp(argv[arg], "--help", 6) == 0 || strncmp(argv[arg], "-h", 2) == 0) {
            showhelp = true;
        } else if(strncmp(argv[arg], "--quiet", 7) == 0 || strncmp(argv[arg], "-q", 2) == 0) {
#if defined(WINDOWS)
            FreeConsole();
#endif
#if defined(POSIX)
            int nullfd = open("/dev/null", O_WRONLY);
            dup2(nullfd, 1);
            close(nullfd);
#endif
#if !USE_FBDEV
        } else if(strncmp(argv[arg], "--width", 7) == 0 || strncmp(argv[arg], "-W", 2) == 0) {
            if(arg + 1 < argc) {
                int w = atoi(argv[arg + 1]);
                if(w > 0) tft_width = w;
                arg++;
            } else {
                std::cout << "Missing width value" << std::endl;
                showhelp = true;
            }
        } else if(strncmp(argv[arg], "--height", 8) == 0 || strncmp(argv[arg], "-H", 2) == 0) {
            if(arg + 1 < argc) {
                int h = atoi(argv[arg + 1]);
                if(h > 0) tft_height = h;
                arg++;
            } else {
                std::cout << "Missing height value" << std::endl;
                showhelp = true;
            }
#endif
        } else if(strncmp(argv[arg], "--config", 8) == 0 || strncmp(argv[arg], "-c", 2) == 0) {
            if(arg + 1 < argc) {
                strcpy(config, argv[arg + 1]);
                arg++;
            } else {
                std::cout << "Missing config directory" << std::endl;
                showhelp = true;
            }
        } else {
            std::cout << "Unrecognized command line parameter: " << argv[arg] << std::endl;
            showhelp = true;
        }
    }

    if(showhelp) {
        usage("openHASP", haspDevice.get_version());
        goto end;
    }

    if(config[0] == '\0') {
#if USE_MONITOR
        SDL_Init(0); // Needs to be initialized for GetPerfPath
        strcpy(config, SDL_GetPrefPath("hasp", "hasp"));
        SDL_Quit(); // We'll properly init later
#elif defined(WINDOWS)
        if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, config))) {
            PathAppendA(config, "hasp");
            PathAppendA(config, "hasp");
        }
#elif defined(POSIX)
        struct passwd* pw = getpwuid(getuid());
        strcpy(config, pw->pw_dir);
        strcat(config, "/.local/share/hasp/hasp");
#endif
    }
    cd(config);

    setup();
    while(haspDevice.pc_is_running) {
        loop();
    }

end:
#if defined(WINDOWS)
    std::cout << std::endl << std::flush;
    fflush(stdout);
    FreeConsole();
    exit(0);
#endif
    return 0;
}

#endif
