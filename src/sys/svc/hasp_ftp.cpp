/* MIT License - Copyright (c) 2019-2022 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasplib.h"

#if HASP_USE_FTP > 0

#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp_ftp.h"
#include "hasp_http.h"
#include "hasp_filesystem.h"

#include "../../hasp/hasp_dispatch.h"

#include "FtpServerKey.h"
#include "SimpleFTPServer.h"

#if HASP_USE_HTTP > 0 || HASP_USE_HTTP_ASYNC > 0
extern hasp_http_config_t http_config;
#endif

FtpServer ftpSrv; // set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial

uint16_t ftpPort   = 23;
uint8_t ftpEnabled = true; // Enable telnet debug output

void _callback(FtpOperation ftpOperation, unsigned int freeSpace, unsigned int totalSpace)
{
    switch(ftpOperation) {
        case FTP_CONNECT:
            LOG_VERBOSE(TAG_FTP, F(D_SERVICE_CONNECTED));
            break;
        case FTP_DISCONNECT:
            LOG_VERBOSE(TAG_FTP, F(D_SERVICE_DISCONNECTED));
            break;
        case FTP_FREE_SPACE_CHANGE:
            LOG_VERBOSE(TAG_FTP, "Free space change, free %u of %u!\n", freeSpace, totalSpace);
            break;
        default:
            break;
    }
};
void _transferCallback(FtpTransferOperation ftpOperation, const char* name, unsigned int transferredSize)
{
    switch(ftpOperation) {
        case FTP_UPLOAD_START:
            LOG_VERBOSE(TAG_FTP, "Start upload of file %s byte %u\n", name, transferredSize);
            break;
        case FTP_UPLOAD:
            LOG_VERBOSE(TAG_FTP, "Upload of file %s byte %u\n", name, transferredSize);
            break;
        case FTP_TRANSFER_STOP:
            LOG_VERBOSE(TAG_FTP, "Completed upload of file %s byte %u\n", name, transferredSize);
            break;
        case FTP_TRANSFER_ERROR:
            LOG_VERBOSE(TAG_FTP, ("Transfer error!"));
            break;
        default:
            break;
    }

    /* FTP_UPLOAD_START = 0,
     * FTP_UPLOAD = 1,
     *
     * FTP_DOWNLOAD_START = 2,
     * FTP_DOWNLOAD = 3,
     *
     * FTP_TRANSFER_STOP = 4,
     * FTP_DOWNLOAD_STOP = 4,
     * FTP_UPLOAD_STOP = 4,
     *
     * FTP_TRANSFER_ERROR = 5,
     * FTP_DOWNLOAD_ERROR = 5,
     * FTP_UPLOAD_ERROR = 5
     */
};

void ftpStop(void)
{
    // LOG_WARNING(TAG_FTP, F("Service cannot be stopped"));
    ftpSrv.end();
    LOG_INFO(TAG_FTP, F(D_SERVICE_STOPPED));
}

void ftpStart()
{
    LOG_TRACE(TAG_FTP, F(D_SERVICE_STARTING));
    ftpSrv.setCallback(_callback);
    ftpSrv.setTransferCallback(_transferCallback);

#if HASP_USE_HTTP > 0 || HASP_USE_HTTP_ASYNC > 0
    ftpSrv.begin(http_config.username, http_config.password); // Password must be non-empty
#else
    ftpSrv.begin("ftpuser", "haspadmin"); // username, password for ftp.   (default 21, 50009 for PASV)
#endif

    LOG_INFO(TAG_FTP, F(D_SERVICE_STARTED));
}

void ftpSetup()
{
#if HASP_START_FTP
    ftpStart();
#endif
}

IRAM_ATTR void ftpLoop()
{
    ftpSrv.handleFTP(); // make sure in loop you call handleFTP()!!
}

void ftpEverySecond(void)
{}

#if HASP_USE_CONFIG > 0
bool ftpGetConfig(const JsonObject& settings)
{
    bool changed = false;

    if(changed) configOutput(settings, TAG_FTP);
    return changed;
}

/** Set FTP Configuration.
 *
 * Read the settings from json and sets the application variables.
 *
 * @note: read config.json into memory
 *
 * @param[in] settings    JsonObject with the config settings.
 **/
bool ftpSetConfig(const JsonObject& settings)
{
    configOutput(settings, TAG_FTP);
    bool changed = false;

    return changed;
}
#endif // HASP_USE_CONFIG

#endif