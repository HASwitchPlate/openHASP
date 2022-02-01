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

FtpServer* ftpSrv; // set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial

uint16_t ftpCtrlPort     = 21;
uint16_t ftpDataPort     = 50009;
uint8_t ftpEnabled       = true;
size_t transferSize      = 0;
const char* transferName = NULL;

void ftp_callback(FtpOperation ftpOperation, unsigned int freeSpace, unsigned int totalSpace)
{
    switch(ftpOperation) {
        case FTP_CONNECT:
            LOG_VERBOSE(TAG_FTP, F(D_SERVICE_CONNECTED));
            break;
        case FTP_DISCONNECT:
            LOG_VERBOSE(TAG_FTP, F(D_SERVICE_DISCONNECTED));
            break;
        case FTP_FREE_SPACE_CHANGE:
            filesystemInfo();
            break;
        default:
            break;
    }
};
void ftp_transfer_callback(FtpTransferOperation ftpOperation, const char* name, unsigned int transferredSize)
{
    transferName = name;
    transferSize = transferredSize;

    switch(ftpOperation) {
        case FTP_UPLOAD_START: {
            char size[16];
            Parser::format_bytes(transferredSize, size, sizeof(size));
            LOG_VERBOSE(TAG_FTP, "Start upload of file %s (%s)", name, size);
            return;
        }
        case FTP_DOWNLOAD_START: {
            char size[16];
            Parser::format_bytes(transferredSize, size, sizeof(size));
            LOG_VERBOSE(TAG_FTP, "Start download of file %s (%s)", name, size);
            return;
        }
        case FTP_UPLOAD:
        case FTP_DOWNLOAD:
            return;
        case FTP_TRANSFER_STOP: {
            char size[16];
            Parser::format_bytes(transferredSize, size, sizeof(size));
            LOG_VERBOSE(TAG_FTP, "Completed transfer of file %s (%s)", name, size);
            break;
        }
        case FTP_TRANSFER_ERROR:
            LOG_VERBOSE(TAG_FTP, ("Transfer error!"));
            break;
        default:
            break;
    }

    transferName = NULL;
    transferSize = 0;

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
    if(ftpSrv) {
        ftpSrv->end();
        delete(ftpSrv);
        ftpSrv = NULL;
    }

    transferSize = 0;
    transferName = NULL;

    LOG_INFO(TAG_FTP, F(D_SERVICE_STOPPED));
}

void ftpStart()
{
    LOG_TRACE(TAG_FTP, F(D_SERVICE_STARTING));

    ftpSrv = new FtpServer(ftpCtrlPort, ftpDataPort);
    if(!ftpSrv) {
        LOG_INFO(TAG_FTP, F(D_SERVICE_START_FAILED));
        return;
    }

    ftpSrv->setCallback(ftp_callback);
    ftpSrv->setTransferCallback(ftp_transfer_callback);

#if HASP_USE_HTTP > 0 || HASP_USE_HTTP_ASYNC > 0
    ftpSrv->begin(http_config.username, http_config.password, D_MANUFACTURER); // Password must be non-empty
#else
    ftpSrv.begin("ftpuser", "haspadmin"); // username, password for ftp.   (default 21, 50009 for PASV)
#endif

    LOG_VERBOSE(TAG_FTP, F(FTP_SERVER_VERSION));
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
    if(ftpSrv) ftpSrv->handleFTP(); // make sure in loop you call handleFTP()!!
}

void ftpEverySecond(void)
{
    if(!transferSize || !transferName) return;

    char size[16];
    Parser::format_bytes(transferSize, size, sizeof(size));
    LOG_VERBOSE(TAG_FTP, D_BULLET "%s (%s)", transferName, size);
}

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