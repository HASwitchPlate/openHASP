/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
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
#define cwd getcwd
#define cd chdir
#endif

#include <cstdlib>
#include <iostream>

#include "hasplib.h"

// #include "app_hal.h"
#if USE_MONITOR
#include "display/monitor.h"
#endif

#include "hasp_debug.h"
#include "hasp_gui.h"

#include "dev/device.h"

bool isConnected;

uint8_t mainLoopCounter        = 0;
unsigned long mainLastLoopTime = 0;

#ifdef HASP_USE_STAT_COUNTER
uint16_t statLoopCounter = 0; // measures the average looptime
#endif

extern uint16_t tft_width;
extern uint16_t tft_height;

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

void setup()
{
    // Initialize lvgl environment
    lv_init();
    lv_log_register_print_cb(debugLvglLogEvent);

    // Read & Apply User Configuration
#if HASP_USE_CONFIG > 0
    configSetup();
#endif

    haspDevice.init(); // hardware setup
    // hal_setup();
    guiSetup();

    dispatchSetup(); // for hasp and oobe
    haspSetup();

#if HASP_USE_MQTT > 0
    mqttSetup(); // Hasp must be running
    mqttStart();
#endif

#if HASP_USE_GPIO > 0
    gpioSetup();
#endif

#if defined(HASP_USE_CUSTOM)
    custom_setup();
#endif

    mainLastLoopTime = millis(); // - 1000; // reset loop counter
    // delay(250);
}

void loop()
{
    haspLoop();
#if HASP_USE_MQTT
    mqttLoop();
#endif

    //    debugLoop(); // Console
    haspDevice.loop();
    guiLoop();

#if HASP_USE_GPIO > 0
    gpioLoop();
#endif

#if defined(HASP_USE_CUSTOM)
    custom_loop();
#endif

#ifdef HASP_USE_STAT_COUNTER
    statLoopCounter++; // measures the average looptime
#endif

    /* Timer Loop */
    if(millis() - mainLastLoopTime >= 1000) {
        /* Runs Every Second */
        haspEverySecond();     // sleep timer
        dispatchEverySecond(); // sleep timer

#if HASP_USE_ARDUINOOTA > 0
        otaEverySecond(); // progressbar
#endif

#if defined(HASP_USE_CUSTOM)
        custom_every_second();
#endif

        /* Runs Every 5 Seconds */
        if(mainLoopCounter == 0 || mainLoopCounter == 5) {

            haspDevice.loop_5s();
#if HASP_USE_GPIO > 0
            gpioEvery5Seconds();
#endif

#if HASP_USE_MQTT
            mqttEvery5Seconds(true);
#endif

#if defined(HASP_USE_CUSTOM)
            custom_every_5seconds();
#endif
        }

        /* Reset loop counter every 10 seconds */
        if(mainLoopCounter >= 9) {
            mainLoopCounter = 0;
        } else {
            mainLoopCounter++;
        }
        mainLastLoopTime += 1000;
    }
    // delay(6);
}

void usage(const char* progName, const char* version)
{
    std::cout
        << "\n"
        << progName << " " << version << " [options]" << std::endl
        << std::endl
        << "Options:" << std::endl
        << "    -h  | --help        Print this help" << std::endl
        << "    -W  | --width       Width of the window" << std::endl
        << "    -H  | --height      Height of the window" << std::endl
        << "    -C  | --config      Configuration directory (default: '~/.local/share/hasp' or 'AppData\\hasp\\hasp')"
        << std::endl
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
        } else if(strncmp(argv[arg], "--config", 8) == 0 || strncmp(argv[arg], "-C", 2) == 0) {
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
#elif USE_WIN32DRV
        if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, config))) {
            PathAppendA(config, "hasp");
            PathAppendA(config, "hasp");
        }
#endif
    }
    cd(config);

    setup();
#if USE_MONITOR
    while(1) {
        loop();
    }
#elif USE_WIN32DRV
    extern bool lv_win32_quit_signal;
    while(!lv_win32_quit_signal) {
        loop();
    }
#endif

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
