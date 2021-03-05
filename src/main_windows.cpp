/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if defined(WINDOWS)
#include <windows.h>
#endif
#if defined(POSIX)
#include <netdb.h>
#include <unistd.h>
#endif

#include <cstdlib>
#include <iostream>

#include "hasp_conf.h"

#include "lvgl.h"
#include "app_hal.h"
#include "display/monitor.h"

#include "hasp_debug.h"
#include "hasp_gui.h"

#include "hasp/hasp_dispatch.h"
#include "hasp/hasp.h"

#include "dev/device.h"

bool isConnected;
bool isRunning = 1;

uint8_t mainLoopCounter        = 0;
unsigned long mainLastLoopTime = 0;

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

void debugLvglLogEvent(lv_log_level_t level, const char* file, uint32_t line, const char* funcname, const char* descr)
{
    printf("%s %d\n", file, line);
}

void setup()
{
    // Load Settings

    // Init debug log
    // debug_init();

    // Initialize lvgl environment
    lv_log_register_print_cb(debugLvglLogEvent);
    lv_init();

    haspDevice.init(); // hardware setup
    // hal_setup();
    guiSetup();

    //    debugSetup(); // Init the console

    printf("%s %d\n", __FILE__, __LINE__);
    dispatchSetup(); // for hasp and oobe
    haspSetup();

#if HASP_USE_MQTT > 0
    printf("%s %d\n", __FILE__, __LINE__);
    mqttSetup(); // Hasp must be running
    mqttStart();
#endif

    mainLastLoopTime = millis() - 1000; // reset loop counter
    delay(250);
    printf("%s %d\n", __FILE__, __LINE__);
}

void loop()
{
    printf("1 \n");
    haspLoop();
    printf("2 \n");
    mqttLoop();

    //    debugLoop(); // Console
    haspDevice.loop();
    guiLoop();

    /* Timer Loop */
    if(millis() - mainLastLoopTime >= 1000) {
        /* Runs Every Second */
        haspEverySecond();     // sleep timer
        dispatchEverySecond(); // sleep timer

#if HASP_USE_OTA > 0
        otaEverySecond(); // progressbar
#endif

        /* Runs Every 5 Seconds */
        if(mainLoopCounter == 0 || mainLoopCounter == 5) {

            haspDevice.loop_5s();
        }

        /* Reset loop counter every 10 seconds */
        if(mainLoopCounter >= 9) {
            mainLoopCounter = 0;
        } else {
            mainLoopCounter++;
        }
        mainLastLoopTime += 1000;
    }
    printf("loop\n");
    // delay(6);
}

#if defined(WINDOWS) || defined(POSIX)

void usage(char* progName)
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
#ifdef WINDOWS
    InitializeConsoleOutput();
#endif

    haspDevice.show_info();

    char hostbuffer[256];
    char* IPbuffer;
    struct hostent* host_entry;
    int hostname;

    // To retrieve hostname
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    // checkHostName(hostname);

    // To retrieve host information
    host_entry = gethostbyname(hostbuffer);
    // checkHostEntry(host_entry);

    // To convert an Internet network
    // address into ASCII string
    // IPbuffer = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));

    printf("Hostname: %s", hostbuffer);
    // printf("Host IP: %s", IPbuffer);

    // Display each command-line argument.
    std::cout << "\nCommand-line arguments:\n";
    for(count = 0; count < argc; count++)
        std::cout << "  argv[" << count << "]   " << argv[count] << "\n" << std::endl << std::flush;

#if defined(WINDOWS)
    SetConsoleCP(65001); // 65001 = UTF-8
    static const char s[] = "tränenüberströmt™\n";
    DWORD slen            = lstrlen(s);
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), s, slen, &slen, NULL);

    HANDLE std_out = GetStdHandle(STD_OUTPUT_HANDLE);
    if(std_out == INVALID_HANDLE_VALUE) {
        //   return 66;
    }
    if(!WriteConsole(std_out, "Hello World!", 12, NULL, NULL)) {
        // return 67;
    }
#endif
    for(count = 0; count < argc; count++) {
        if(argv[count][0] == '-') {

            if(strncmp(argv[count], "--help", 6) == 0 || strncmp(argv[count], "-h", 2) == 0) {
                std::cout << "  argv[" << count << "]   " << argv[count] << "\n" << std::endl << std::flush;
                fflush(stdout);
                exit(0);
            }

            if(strncmp(argv[count], "--name", 6) == 0) {
                std::cout << "  argv[" << count << "]   " << argv[count] << "\n" << std::endl << std::flush;
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
        usage("hasp-lvgl");

#if defined(WINDOWS)
        WriteConsole(std_out, "bye", 3, NULL, NULL);

        FreeConsole();
#endif
        return 0;
    }

    // printf("%s %d\n", __FILE__, __LINE__);
    // fflush(stdout);
    printf("pre setup\n");
    setup();
    printf("to loop\n");

    while(isRunning) {
        loop();
        // std::cout << "HSetup OK\n";
    }
    printf("endrunning\n");

    return 0;
}


#endif
