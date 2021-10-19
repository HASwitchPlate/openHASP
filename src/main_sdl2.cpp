/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if defined(WINDOWS) || defined(POSIX)

#if defined(WINDOWS)

#include <windows.h>
#include <direct.h>
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

#include "hasp_conf.h"

#include "lvgl.h"
// #include "app_hal.h"
#include "display/monitor.h"

#include "hasp_debug.h"
#include "hasp_gui.h"

#include "hasp/hasp_dispatch.h"
#include "hasp/hasp.h"

#include "dev/device.h"

#if defined(HASP_USE_CUSTOM)
#include "custom/my_custom.h"
#endif

bool isConnected;
bool isRunning = 1;

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
    // Load Settings

    // Init debug log
    // debug_init();

    // Initialize lvgl environment
    lv_init();
    lv_log_register_print_cb(debugLvglLogEvent);

    haspDevice.init();      // hardware setup
    haspDevice.show_info(); // debug info
    // hal_setup();
    guiSetup();

    printf("%s %d\n", __FILE__, __LINE__);
    dispatchSetup(); // for hasp and oobe
    haspSetup();

#if HASP_USE_MQTT > 0
    printf("%s %d\n", __FILE__, __LINE__);
    mqttSetup(); // Hasp must be running
    mqttStart();
#endif

#if HASP_USE_GPIO > 0
    printf("%s %d\n", __FILE__, __LINE__);
    gpioSetup();
#endif

#if defined(HASP_USE_CUSTOM)
    custom_setup();
#endif

    mainLastLoopTime = millis(); // - 1000; // reset loop counter
    printf("%s %d\n", __FILE__, __LINE__);
    // delay(250);
}

void loop()
{
    haspLoop();
    mqttLoop();

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

#if HASP_USE_OTA > 0
        otaEverySecond(); // progressbar
#endif

#if defined(HASP_USE_CUSTOM)
        custom_every_second();
#endif

        /* Runs Every 5 Seconds */
        if(mainLoopCounter == 0 || mainLoopCounter == 5) {

            haspDevice.loop_5s();
            gpioEvery5Seconds();

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

void usage(const char* progName)
{
    std::cout << progName << " [options]" << std::endl
              << std::endl
              << "Options:" << std::endl
              << "    -h | --help        Print this help" << std::endl
              << "    -n | --name        Plate hostname used in the mqtt topic"
              << std::endl
              //   << "    -b | --broker      Mqtt broker name or ip address" << std::endl
              //   << "    -P | --port        Mqtt broker port (default: 1883)" << std::endl
              //   << "    -u | --user        Mqtt username (optional)" << std::endl
              //   << "    -p | --pass        Mqtt password (optional)" << std::endl
              //   << "    -t | --topic       Base topic of the mqtt messages (default: hasp)" << std::endl
              //   << "    -g | --group       Group topic of on which to accept incoming messages (default: plates)"
              //   << std::endl
              //   << "    -f | --fullscreen  Open the application fullscreen" << std::endl
              //   << "    -v | --verbose     Verbosity level" << std::endl
              << std::endl;
    fflush(stdout);
#if defined(WINDOWS)
    static const char s[] = "\n";
    DWORD slen            = lstrlen(s);
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), s, slen, &slen, NULL);
#endif
}

int main(int argc, char* argv[])
{
    bool showhelp = false;
    int count;

#if defined(WINDOWS)
    InitializeConsoleOutput();
    SetConsoleCP(65001); // 65001 = UTF-8
    static const char s[] = "tränenüberströmt™\n";
    DWORD slen            = lstrlen(s);
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), s, slen, &slen, NULL);

    HANDLE std_out = GetStdHandle(STD_OUTPUT_HANDLE);
    if(std_out == INVALID_HANDLE_VALUE) {
        return 66;
    }
    if(!WriteConsole(std_out, "Hello World!\n", 13, NULL, NULL)) {
        return 67;
    }
#endif

    SDL_Init(0);    // Needs to be initialized for GetPerfPath
    char buf[4096]; // never know how much is needed
    std::cout << "CWD: " << cwd(buf, sizeof buf) << std::endl;
    cd(SDL_GetPrefPath("hasp", "hasp"));
    std::cout << "CWD changed to: " << cwd(buf, sizeof buf) << std::endl;
    SDL_Quit(); // We'll properly init later

    // Change to preferences dir
    std::cout << "\nCommand-line arguments:\n";
    for(count = 0; count < argc; count++)
        std::cout << "  argv[" << count << "]   " << argv[count] << std::endl << std::flush;

    for(count = 0; count < argc; count++) {
        if(argv[count][0] == '-') {

            if(strncmp(argv[count], "--help", 6) == 0 || strncmp(argv[count], "-h", 2) == 0) {
                showhelp = true;
            }

            if(strncmp(argv[count], "--width", 7) == 0 || strncmp(argv[count], "-x", 2) == 0) {
                int w = atoi(argv[count + 1]);
                if(w > 0) tft_width = w;
            }

            if(strncmp(argv[count], "--height", 8) == 0 || strncmp(argv[count], "-y", 2) == 0) {
                int h = atoi(argv[count + 1]);
                if(h > 0) tft_height = h;
            }

            if(strncmp(argv[count], "--name", 6) == 0 || strncmp(argv[count], "-n", 2) == 0) {
                std::cout << "  argv[" << count << "]   " << argv[count] << std::endl << std::flush;
                fflush(stdout);
                if(count + 1 < argc) {
                    haspDevice.set_hostname(argv[count + 1]);
                } else {
                    showhelp = true;
                }
            }
        }
    }

    if(showhelp) {
        usage("openHASP");

#if defined(WINDOWS)
        WriteConsole(std_out, "bye\n", 3, NULL, NULL);
        std::cout << std::endl << std::flush;
        fflush(stdout);
        FreeConsole();
        exit(0);
#endif
        return 0;
    }

    // printf("%s %d\n", __FILE__, __LINE__);
    // fflush(stdout);

    debugPrintHaspHeader(stdout);
    LOG_INFO(TAG_MAIN, "resolution %d x %d", tft_width, tft_height);
    LOG_INFO(TAG_MAIN, "pre setup");

    setup();

    LOG_TRACE(TAG_MAIN, "loop started");
    while(isRunning) {
        loop();
    }
    LOG_TRACE(TAG_MAIN, "main loop completed");

    return 0;
}

#endif
