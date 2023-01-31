/* MIT License - Copyright (c) 2019-2023 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "hasplib.h"

#if HASP_USE_FTP > 0

#include "hasp_debug.h"
#include "hasp_ftp.h"
#include "hasp_filesystem.h"

#include "../../hasp/hasp_dispatch.h"

#include "FtpServerKey.h"
#include "SimpleFTPServer.h"

FtpServer* ftpSrv; // set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial

String ftpUsername       = "";
String ftpPassword       = "";
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
            LOG_VERBOSE(TAG_FTP, "Receiving file %s (%s)", name, size);
            return;
        }
        case FTP_DOWNLOAD_START: {
            char size[16];
            Parser::format_bytes(transferredSize, size, sizeof(size));
            LOG_VERBOSE(TAG_FTP, "Sending file %s (%s)", name, size);
            return;
        }
        case FTP_UPLOAD:
        case FTP_DOWNLOAD:
            return;
        case FTP_TRANSFER_STOP: {
            char size[16];
            Parser::format_bytes(transferredSize, size, sizeof(size));
            LOG_VERBOSE(TAG_FTP, "Completed file %s (%s)", name, size);
            break;
        }
        case FTP_TRANSFER_ERROR:
            LOG_ERROR(TAG_FTP, ("Transfer error!"));
            break;
        default:
            break;
    }

    transferName = NULL;
    transferSize = 0;
};

static void ftpInitializePorts()
{
    Preferences preferences;

    nvs_user_begin(preferences, FP_FTP, true);
    ftpCtrlPort = preferences.getUShort(FP_CONFIG_PORT, ftpCtrlPort); // Read from NVS if it exists
    ftpDataPort = preferences.getUShort(FP_CONFIG_PASV, ftpDataPort); // Read from NVS if it exists
    preferences.end();
}

void ftpStop(void)
{
    if(ftpSrv) {
        ftpSrv->end();
        delete(ftpSrv);
        ftpSrv = NULL;
    }

    transferSize = 0;
    transferName = NULL;
    ftpInitializePorts();

    LOG_INFO(TAG_FTP, F(D_SERVICE_STOPPED));
}

void ftpStart()
{
    if(!ftpSrv) {
        LOG_TRACE(TAG_FTP, F(D_SERVICE_STARTING));
        Preferences preferences;

        nvs_user_begin(preferences, FP_FTP, true);
        ftpUsername = preferences.getString(FP_CONFIG_USER, ftpUsername); // Read from NVS if it exists
        ftpPassword = preferences.getString(FP_CONFIG_PASS, ftpPassword); // Read from NVS if it exists
        preferences.end();

        if(!ftpEnabled || ftpUsername == "" || ftpUsername == "anonymous" || ftpCtrlPort == 0) {
            LOG_INFO(TAG_FTP, F(D_SERVICE_DISABLED));
            return;
        }

        ftpSrv = new FtpServer(ftpCtrlPort, ftpDataPort);
        if(!ftpSrv) {
            LOG_INFO(TAG_FTP, F(D_SERVICE_START_FAILED));
            return;
        }

        ftpSrv->setCallback(ftp_callback);
        ftpSrv->setTransferCallback(ftp_transfer_callback);
        ftpSrv->begin(ftpUsername.c_str(), ftpPassword.c_str(), D_MANUFACTURER); // Password must be non-empty

        LOG_VERBOSE(TAG_FTP, F(FTP_SERVER_VERSION));
    }

    LOG_INFO(TAG_FTP, F(D_SERVICE_STARTED));
}

void ftpSetup()
{
    ftpInitializePorts();

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

////////////////////////////////////////////////////////////////////////////////////////////////////
#if HASP_USE_CONFIG > 0
bool ftpGetConfig(const JsonObject& settings)
{
    bool changed = false;
    Preferences preferences;

    nvs_user_begin(preferences, FP_FTP, true);
    String nvsUsername = preferences.getString(FP_CONFIG_USER, ftpUsername); // Read from NVS if it exists
    String nvsPassword = preferences.getString(FP_CONFIG_PASS, ftpPassword); // Read from NVS if it exists
    uint16_t nvsPort   = preferences.getUShort(FP_CONFIG_PORT, ftpCtrlPort); // Read from NVS if it exists
    uint16_t nvsData   = preferences.getUShort(FP_CONFIG_PASV, ftpDataPort); // Read from NVS if it exists
    preferences.end();

    settings[FPSTR(FP_CONFIG_ENABLE)] = ftpEnabled;

    if(nvsPort != settings[FPSTR(FP_CONFIG_PORT)].as<uint16_t>()) changed = true;
    settings[FPSTR(FP_CONFIG_PORT)] = nvsPort;

    if(nvsData != settings[FPSTR(FP_CONFIG_PASV)].as<uint16_t>()) changed = true;
    settings[FPSTR(FP_CONFIG_PASV)] = nvsData;

    if(strcmp(nvsUsername.c_str(), settings[FP_CONFIG_USER].as<String>().c_str()) != 0) changed = true;
    settings[FP_CONFIG_USER] = nvsUsername;

    if(strcmp(D_PASSWORD_MASK, settings[FP_CONFIG_PASS].as<String>().c_str()) != 0) changed = true;
    settings[FP_CONFIG_PASS] = D_PASSWORD_MASK;

    if(changed) configOutput(settings, TAG_FTP);
    return changed;
}

/** Set FTP Configuration.
 *
 * Read the settings from json and sets the application variables.
 *
 * @note: data pixel should be formated to uint32_t RGBA. Imagemagick requirements.
 *
 * @param[in] settings    JsonObject with the config settings.
 **/
bool ftpSetConfig(const JsonObject& settings)
{
    Preferences preferences;
    nvs_user_begin(preferences, FP_FTP, false);

    configOutput(settings, TAG_FTP);
    bool changed = false;

    changed |= nvsUpdateUShort(preferences, FP_CONFIG_PORT, settings[FPSTR(FP_CONFIG_PORT)]);
    changed |= nvsUpdateUShort(preferences, FP_CONFIG_PASV, settings[FPSTR(FP_CONFIG_PASV)]);

    if(!settings[FPSTR(FP_CONFIG_USER)].isNull()) {
        changed |= nvsUpdateString(preferences, FP_CONFIG_USER, settings[FPSTR(FP_CONFIG_USER)]);
    }

    if(!settings[FPSTR(FP_CONFIG_PASS)].isNull() &&
       settings[FPSTR(FP_CONFIG_PASS)].as<String>() != String(FPSTR(D_PASSWORD_MASK))) {
        changed |= nvsUpdateString(preferences, FP_CONFIG_PASS, settings[FPSTR(FP_CONFIG_PASS)]);
    }

    preferences.end();
    return changed;
}
#endif // HASP_USE_CONFIG

#endif