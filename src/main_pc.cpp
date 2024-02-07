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

    haspDevice.init();      // hardware setup
    haspDevice.show_info(); // debug info
    // hal_setup();
    guiSetup();

    LOG_DEBUG(TAG_MAIN, "%s %d", __FILE__, __LINE__);
    dispatchSetup(); // for hasp and oobe
    haspSetup();

#if HASP_USE_MQTT > 0
    LOG_DEBUG(TAG_MAIN, "%s %d", __FILE__, __LINE__);
    mqttSetup(); // Hasp must be running
    mqttStart();
#endif

#if HASP_USE_GPIO > 0
    LOG_DEBUG(TAG_MAIN, "%s %d", __FILE__, __LINE__);
    gpioSetup();
#endif

#if defined(HASP_USE_CUSTOM)
    custom_setup();
#endif

    mainLastLoopTime = millis(); // - 1000; // reset loop counter
    LOG_DEBUG(TAG_MAIN, "%s %d", __FILE__, __LINE__);
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
            gpioEvery5Seconds();

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
    std::cout << "\n\n"
              << progName << " " << version << " [options]" << std::endl
              << std::endl
              << "Options:" << std::endl
              << "    -?  | --help        Print this help" << std::endl
              << "    -w  | --width       Width of the window" << std::endl
              << "    -h  | --height      Height of the window" << std::endl
              << "    --mqttname          MQTT device name topic (default: computer hostname)" << std::endl
              << "    --mqtthost          MQTT broker hostname or IP address" << std::endl
              << "    --mqttport          MQTT broker port (default: 1883)" << std::endl
              << "    --mqttuser          MQTT username" << std::endl
              << "    --mqttpass          MQTT password" << std::endl
              << "    --mqttgroup         MQTT groupname (default: plates)" << std::endl
              << std::endl
              //   << "    -t | --topic       Base topic of the mqtt messages (default: hasp)" << std::endl
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

    char buf[4096]; // never know how much is needed
    std::cout << "CWD: " << cwd(buf, sizeof buf) << std::endl;
#if USE_MONITOR
    SDL_Init(0); // Needs to be initialized for GetPerfPath
    cd(SDL_GetPrefPath("hasp", "hasp"));
    SDL_Quit(); // We'll properly init later
#elif USE_WIN32DRV
    if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, buf))) {
        PathAppendA(buf, "hasp");
        PathAppendA(buf, "hasp");
        cd(buf);
    }
#endif
    std::cout << "CWD changed to: " << cwd(buf, sizeof buf) << std::endl;

    // Change to preferences dir
    std::cout << "\nCommand-line arguments:\n";
    for(count = 0; count < argc; count++)
        std::cout << "  argv[" << count << "]   " << argv[count] << std::endl << std::flush;

    StaticJsonDocument<1024> settings;

    for(count = 0; count < argc; count++) {
        if(argv[count][0] == '-') {

            if(strncmp(argv[count], "--help", 6) == 0 || strncmp(argv[count], "-?", 2) == 0) {
                showhelp = true;
            }

            if(strncmp(argv[count], "--width", 7) == 0 || strncmp(argv[count], "-w", 2) == 0) {
                int w = atoi(argv[count + 1]);
                if(w > 0) tft_width = w;
            }

            if(strncmp(argv[count], "--height", 8) == 0 || strncmp(argv[count], "-h", 2) == 0) {
                int h = atoi(argv[count + 1]);
                if(h > 0) tft_height = h;
            }

            if(strncmp(argv[count], "--mqttname", 10) == 0 || strncmp(argv[count], "-n", 2) == 0) {
                std::cout << "  argv[" << count << "]   " << argv[count] << std::endl << std::flush;
                fflush(stdout);
                if(count + 1 < argc) {
                    haspDevice.set_hostname(argv[count + 1]);
                    settings["mqtt"]["name"] = argv[count + 1];
                } else {
                    showhelp = true;
                }
            }

            if(strncmp(argv[count], "--mqtthost", 10) == 0) {
                std::cout << "  argv[" << count << "]   " << argv[count] << std::endl << std::flush;
                fflush(stdout);
                if(count + 1 < argc) {
                    settings["mqtt"]["host"] = argv[count + 1];
                } else {
                    showhelp = true;
                }
            }

            if(strncmp(argv[count], "--mqttport", 10) == 0) {
                std::cout << "  argv[" << count << "]   " << argv[count] << std::endl << std::flush;
                fflush(stdout);
                if(count + 1 < argc) {
                    settings["mqtt"]["port"] = atoi(argv[count + 1]);
                } else {
                    showhelp = true;
                }
            }

            if(strncmp(argv[count], "--mqttuser", 10) == 0) {
                std::cout << "  argv[" << count << "]   " << argv[count] << std::endl << std::flush;
                fflush(stdout);
                if(count + 1 < argc) {
                    settings["mqtt"]["user"] = argv[count + 1];
                } else {
                    showhelp = true;
                }
            }

            if(strncmp(argv[count], "--mqttpass", 10) == 0) {
                std::cout << "  argv[" << count << "]   " << argv[count] << std::endl << std::flush;
                fflush(stdout);
                if(count + 1 < argc) {
                    settings["mqtt"]["pass"] = argv[count + 1];
                } else {
                    showhelp = true;
                }
            }
        }
    }

    if(showhelp) {
        usage("openHASP", haspDevice.get_version());

#if defined(WINDOWS)
        WriteConsole(std_out, "bye\n\n", 3, NULL, NULL);
        std::cout << std::endl << std::flush;
        fflush(stdout);
        FreeConsole();
        exit(0);
#endif
        return 0;
    }

    char buffer[2048];
    serializeJson(settings, buffer, sizeof(buffer));
    std::cout << buffer << std::endl << std::flush;
    fflush(stdout);
#if HASP_USE_MQTT
    mqttSetConfig(settings["mqtt"]);
#endif
    // printf("%s %d\n", __FILE__, __LINE__);
    // fflush(stdout);

    debugPrintHaspHeader(stdout);
    LOG_INFO(TAG_MAIN, "resolution %d x %d", tft_width, tft_height);
    LOG_INFO(TAG_MAIN, "pre setup");

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

#if defined(WINDOWS)
    WriteConsole(std_out, "bye\n\n", 3, NULL, NULL);
    std::cout << std::endl << std::flush;
    fflush(stdout);
    FreeConsole();
    exit(0);
#endif

    return 0;
}

#endif
