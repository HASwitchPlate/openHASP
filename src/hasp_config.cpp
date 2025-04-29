/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if HASP_USE_CONFIG > 0

#include "hasplib.h"

#include "hasp_config.h"
#include "hasp_debug.h"
#include "hasp_gui.h"
#include "sys/net/hasp_time.h"
#if HASP_TARGET_ARDUINO
#include "hal/hasp_hal.h"
#endif

// #include "hasp_ota.h" included in conf
// #include "hasp_filesystem.h" included in conf
// #include "hasp_telnet.h" included in conf
// #include "hasp_gpio.h" included in conf

// #include "hasp_eeprom.h"

#if HASP_USE_EEPROM > 0
#include "EEPROM.h"
#endif

#if HASP_USE_EEPROM > 0
#include "StreamUtils.h" // For EEPromStream
#endif

extern uint16_t dispatchTelePeriod;
extern uint32_t dispatchLastMillis;

extern gui_conf_t gui_settings;
extern dispatch_conf_t dispatch_settings;

#if HASP_TARGET_ARDUINO
void confDebugSet(const __FlashStringHelper* fstr_name)
{
    /*char buffer[128];
    snprintf_P(buffer, sizeof(buffer), PSTR("   * %s set"), name);
    debugPrintln(buffer);*/
    LOG_VERBOSE(TAG_CONF, F(D_BULLET "%S set"), fstr_name);
}
#endif
void confDebugSet(const char* fstr_name)
{
    /*char buffer[128];
    snprintf_P(buffer, sizeof(buffer), PSTR("   * %s set"), name);
    debugPrintln(buffer);*/
    LOG_VERBOSE(TAG_CONF, F(D_BULLET "%s set"), fstr_name);
}

#if HASP_TARGET_ARDUINO
bool configSet(bool& value, const JsonVariant& setting, const __FlashStringHelper* fstr_name)
{
    if(!setting.isNull()) {
        bool val = setting.as<bool>();
        if(value != val) {
            confDebugSet(fstr_name);
            value = val;
            return true;
        }
    }
    return false;
}
bool configSet(int8_t& value, const JsonVariant& setting, const __FlashStringHelper* fstr_name)
{
    if(!setting.isNull()) {
        int8_t val = setting.as<int8_t>();
        if(value != val) {
            confDebugSet(fstr_name);
            value = val;
            return true;
        }
    }
    return false;
}
bool configSet(uint8_t& value, const JsonVariant& setting, const __FlashStringHelper* fstr_name)
{
    if(!setting.isNull()) {
        uint8_t val = setting.as<uint8_t>();
        if(value != val) {
            confDebugSet(fstr_name);
            value = val;
            return true;
        }
    }
    return false;
}
bool configSet(uint16_t& value, const JsonVariant& setting, const __FlashStringHelper* fstr_name)
{
    if(!setting.isNull()) {
        uint16_t val = setting.as<uint16_t>();
        if(value != val) {
            confDebugSet(fstr_name);
            value = val;
            return true;
        }
    }
    return false;
}
bool configSet(int32_t& value, const JsonVariant& setting, const __FlashStringHelper* fstr_name)
{
    if(!setting.isNull()) {
        int32_t val = setting.as<int32_t>();
        if(value != val) {
            confDebugSet(fstr_name);
            value = val;
            return true;
        }
    }
    return false;
}
bool configSet(lv_color_t& value, const JsonVariant& setting, const __FlashStringHelper* fstr_name)
{
    lv_color32_t c32;
    if(!setting.isNull() && Parser::haspPayloadToColor(setting.as<const char*>(), c32)) {
        lv_color_t val = lv_color_make(c32.ch.red, c32.ch.green, c32.ch.blue);
        if(value.full != val.full) {
            confDebugSet(fstr_name);
            value = val;
            return true;
        }
    }
    return false;
}
bool configSet(char *value, size_t size, const JsonVariant& setting, const __FlashStringHelper* fstr_name)
{
    if(!setting.isNull()) {
        const char *val = setting;
        if(strcmp(value, val) != 0) {
            confDebugSet(fstr_name);
            strncpy(value, val, size - 1);
            value[size - 1] = '\0';
            return true;
        }
    }
    return false;
}
#endif

bool configSet(bool& value, const JsonVariant& setting, const char* fstr_name)
{
    if(!setting.isNull()) {
        bool val = setting.as<bool>();
        if(value != val) {
            confDebugSet(fstr_name);
            value = val;
            return true;
        }
    }
    return false;
}
bool configSet(int8_t& value, const JsonVariant& setting, const char* fstr_name)
{
    if(!setting.isNull()) {
        int8_t val = setting.as<int8_t>();
        if(value != val) {
            confDebugSet(fstr_name);
            value = val;
            return true;
        }
    }
    return false;
}
bool configSet(uint8_t& value, const JsonVariant& setting, const char* fstr_name)
{
    if(!setting.isNull()) {
        uint8_t val = setting.as<uint8_t>();
        if(value != val) {
            confDebugSet(fstr_name);
            value = val;
            return true;
        }
    }
    return false;
}
bool configSet(uint16_t& value, const JsonVariant& setting, const char* fstr_name)
{
    if(!setting.isNull()) {
        uint16_t val = setting.as<uint16_t>();
        if(value != val) {
            confDebugSet(fstr_name);
            value = val;
            return true;
        }
    }
    return false;
}
bool configSet(int32_t& value, const JsonVariant& setting, const char* fstr_name)
{
    if(!setting.isNull()) {
        int32_t val = setting.as<int32_t>();
        if(value != val) {
            confDebugSet(fstr_name);
            value = val;
            return true;
        }
    }
    return false;
}
bool configSet(lv_color_t& value, const JsonVariant& setting, const char* fstr_name)
{
    lv_color32_t c32;
    if(!setting.isNull() && Parser::haspPayloadToColor(setting.as<const char*>(), c32)) {
        lv_color_t val = lv_color_make(c32.ch.red, c32.ch.green, c32.ch.blue);
        if(value.full != val.full) {
            confDebugSet(fstr_name);
            value = val;
            return true;
        }
    }
    return false;
}

void configSetupDebug(JsonDocument& settings)
{
#if HASP_TARGET_ARDUINO
    debugSetup(settings[FPSTR(FP_DEBUG)]);
#endif
    debugStart(); // Debug started, now we can use it; HASP header sent
}

void configStorePasswords(JsonDocument& settings, String& wifiPass, String& mqttPass, String& httpPass, String &wgPrivKey)
{
    const char* pass = ("pass");

    wifiPass = settings[FPSTR(FP_WIFI)][pass].as<String>();
    mqttPass = settings[FPSTR(FP_MQTT)][pass].as<String>();
    httpPass = settings[FPSTR(FP_HTTP)][pass].as<String>();
    wgPrivKey = settings[FPSTR(FP_WG)][FPSTR(FP_CONFIG_PRIVATE_KEY)].as<String>();
}

void configRestorePasswords(JsonDocument& settings, String& wifiPass, String& mqttPass, String& httpPass, String& wgPrivKey)
{
    const char* pass = ("pass");

    if(!settings[FPSTR(FP_WIFI)][pass].isNull()) settings[FPSTR(FP_WIFI)][pass] = wifiPass;
    if(!settings[FPSTR(FP_MQTT)][pass].isNull()) settings[FPSTR(FP_MQTT)][pass] = mqttPass;
    if(!settings[FPSTR(FP_HTTP)][pass].isNull()) settings[FPSTR(FP_HTTP)][pass] = httpPass;
    if(!settings[FPSTR(FP_WG)][FPSTR(FP_CONFIG_PRIVATE_KEY)].isNull()) settings[FPSTR(FP_WG)][FPSTR(FP_CONFIG_PRIVATE_KEY)] = wgPrivKey;
}

void configMaskPasswords(JsonDocument& settings)
{
    String passmask = F(D_PASSWORD_MASK);
    configRestorePasswords(settings, passmask, passmask, passmask, passmask);
}

DeserializationError configParseFile(String& configFile, JsonDocument& settings)
{
    DeserializationError result = DeserializationError::InvalidInput;
#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
    File file = HASP_FS.open(configFile, "r");

    if(file) {
        // size_t size = file.size();
        // if(size > 1024) {
        //     LOG_ERROR(TAG_CONF, F("Config file size is too large"));
        //     return DeserializationError::NoMemory;
        // }
        result = deserializeJson(settings, file);
        file.close();
        return result;
    }
    return DeserializationError::InvalidInput;
#elif HASP_TARGET_PC
    lv_fs_file_t f;
    lv_fs_res_t res;
    lv_fs_open(&f, "L:/config.json", LV_FS_MODE_RD);
    if(res == LV_FS_RES_OK) {
        uint32_t size = 0, read = 0;
        if(lv_fs_size(&f, &size) == LV_FS_RES_OK && size != 0) {
            char* buf = (char*)malloc(size + 1);
            if(lv_fs_read(&f, buf, size, &read) == LV_FS_RES_OK && read == size) {
                result = deserializeJson(settings, buf);
            }
        }
        lv_fs_close(&f);
        return result;
    }
    LOG_ERROR(TAG_HASP, F("Opening config.json from FS failed %d"), res);
    return result;
#else
    return result;
#endif
}

DeserializationError configRead(JsonDocument& settings, bool setupdebug)
{
    String configFile;
    configFile.reserve(32);
    configFile = String(FPSTR(FP_HASP_CONFIG_FILE));
    DeserializationError error;

    if(setupdebug) configSetupDebug(settings); // Now we can use log

#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0 || HASP_TARGET_PC
    error = configParseFile(configFile, settings);
    if(!error) {
        String output, wifiPass, mqttPass, httpPass, wgPrivKey;

        /* Load Debug params */
        if(setupdebug) {
            LOG_INFO(TAG_CONF, F("SPI flash FS mounted"));

#if HASP_TARGET_ARDUINO
            filesystemInfo();
            filesystemList();
#endif
        }

        LOG_TRACE(TAG_CONF, F(D_FILE_LOADING), configFile.c_str());
        configStorePasswords(settings, wifiPass, mqttPass, httpPass, wgPrivKey);

        // Output settings in log with masked passwords
        configMaskPasswords(settings);
        serializeJson(settings, output);
        LOG_VERBOSE(TAG_CONF, output.c_str());

        configRestorePasswords(settings, wifiPass, mqttPass, httpPass, wgPrivKey);
        LOG_INFO(TAG_CONF, F(D_FILE_LOADED), configFile.c_str());

        // if(setupdebug) debugSetup();
        return error;
    }

#else

#if HASP_USE_EEPROM > 0
    EepromStream eepromStream(0, 1024);
    error = deserializeJson(settings, eepromStream);
#endif

#endif

#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
    LOG_ERROR(TAG_CONF, F(D_FILE_LOAD_FAILED), configFile.c_str());
#endif

#if HASP_USE_EEPROM > 0
    configFile = F("EEPROM");
    LOG_TRACE(TAG_CONF, F(D_FILE_LOADING), configFile.c_str());
    LOG_INFO(TAG_CONF, F(D_FILE_LOADED), configFile.c_str());
#endif

#if HASP_USE_CONFIG > 0 && defined(HASP_GPIO_TEMPLATE)
    // Load custom GPIO template
    char json[96];
    snprintf(json, sizeof(json), PSTR("{\"%s\":%s}"), (char*)(FPSTR(FP_GPIO_CONFIG)),
             (char*)(FPSTR(HASP_GPIO_TEMPLATE)));
    dispatch_config((char*)(FPSTR(FP_GPIO)), json, TAG_CONF);
#endif

    return error;
}

/*
void configBackupToEeprom()
{
#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
    String configFile((char *)0);
    configFile.reserve(32);
    configFile = String(FPSTR(FP_HASP_CONFIG_FILE));

    EEPROM.begin(1024);
    uint8_t buffer[128];
    size_t index = 0;

    File file = HASP_FS.open(configFile, "r");
    if(file) {

        while(size_t count = file.read(buffer, sizeof(buffer)) > 0) {
            for(size_t i = 0; i < count; i++) {
                EEPROM.write(index, buffer[i]);
                index++;
            }
        }

        file.close();
        EEPROM.commit();

        LOG_INFO(TAG_CONF,F("Written %u to EEPROM"), index);
    }
#endif
}
*/
void configWrite()
{
    String configFile;
    configFile.reserve(32);
    configFile = String(FPSTR(FP_HASP_CONFIG_FILE));

    String settingsChanged;
    settingsChanged.reserve(128);
    settingsChanged = F(D_CONFIG_CHANGED);

    /* Read Config File */
    DynamicJsonDocument doc(MAX_CONFIG_JSON_ALLOC_SIZE);
    LOG_TRACE(TAG_CONF, F(D_FILE_LOADING), configFile.c_str());
    configRead(doc, false);
    LOG_INFO(TAG_CONF, F(D_FILE_LOADED), configFile.c_str());

    // Make sure we have a valid JsonObject to start from
    JsonObject settings;
    if(doc.as<JsonObject>().isNull()) {
        settings = doc.to<JsonObject>(); // Settings are invalid, force creation of an empty JsonObject
    } else {
        settings = doc.as<JsonObject>(); // Settings are valid, cast as a JsonObject
    }

    bool writefile = false;
    bool changed   = false;
    const __FlashStringHelper* module;

#if HASP_USE_WIFI > 0
    module = FPSTR(FP_WIFI);
    if(settings[module].as<JsonObject>().isNull()) settings.createNestedObject(module);
    changed = wifiGetConfig(settings[module]);
    if(changed) {
        LOG_VERBOSE(TAG_WIFI, settingsChanged.c_str());
        configOutput(settings[module], TAG_WIFI);
        writefile = true;
    }
#endif

#if HASP_USE_WIREGUARD > 0
    module = FPSTR(FP_WG);
    if(settings[module].as<JsonObject>().isNull()) settings.createNestedObject(module);
    changed = wgGetConfig(settings[module]);
    if(changed) {
        LOG_VERBOSE(TAG_WG, settingsChanged.c_str());
        configOutput(settings[module], TAG_WG);
        writefile = true;
    }
#endif

#if HASP_USE_MQTT > 0
    module = FPSTR(FP_MQTT);
    if(settings[module].as<JsonObject>().isNull()) settings.createNestedObject(module);
    changed = mqttGetConfig(settings[module]);
    if(changed) {
        LOG_VERBOSE(TAG_MQTT, settingsChanged.c_str());
        configOutput(settings[module], TAG_MQTT);
        writefile = true;
    }
#endif

#if HASP_USE_TELNET > 0
    module = F("telnet");
    if(settings[module].as<JsonObject>().isNull()) settings.createNestedObject(module);
    changed = telnetGetConfig(settings[module]);
    if(changed) {
        LOG_VERBOSE(TAG_TELN, settingsChanged.c_str());
        configOutput(settings[module], TAG_TELN);
        writefile = true;
    }
#endif

#if HASP_USE_MDNS > 0
    module = FPSTR(FP_MDNS);
    if(settings[module].as<JsonObject>().isNull()) settings.createNestedObject(module);
    changed = mdnsGetConfig(settings[module]);
    if(changed) {
        LOG_VERBOSE(TAG_MDNS, settingsChanged.c_str());
        configOutput(settings[module], TAG_MDNS);
        writefile = true;
    }
#endif

#if HASP_USE_HTTP > 0
    if(settings[FPSTR(FP_HTTP)].as<JsonObject>().isNull()) settings.createNestedObject(F("http"));
    changed = httpGetConfig(settings[FPSTR(FP_HTTP)]);
    if(changed) {
        LOG_VERBOSE(TAG_HTTP, settingsChanged.c_str());
        configOutput(settings[FPSTR(FP_HTTP)], TAG_HTTP);
        writefile = true;
    }
#endif

#if HASP_USE_FTP > 0
    if(settings[FPSTR(FP_FTP)].as<JsonObject>().isNull()) settings.createNestedObject(F("ftp"));
    changed = ftpGetConfig(settings[FPSTR(FP_FTP)]);
    if(changed) {
        LOG_VERBOSE(TAG_FTP, settingsChanged.c_str());
        configOutput(settings[FPSTR(FP_FTP)], TAG_FTP);
        writefile = true;
    }
#endif

#if HASP_USE_CONFIG > 0
    if(settings[FPSTR(FP_TIME)].as<JsonObject>().isNull()) settings.createNestedObject(F("time"));
    changed = timeGetConfig(settings[FPSTR(FP_TIME)]);
    if(changed) {
        LOG_VERBOSE(TAG_TIME, settingsChanged.c_str());
        configOutput(settings[FPSTR(FP_TIME)], TAG_TIME);
        writefile = true;
    }
#endif

#if HASP_USE_GPIO > 0
    module = FPSTR(FP_GPIO);
    if(settings[module].as<JsonObject>().isNull()) settings.createNestedObject(module);
    changed = gpioGetConfig(settings[module]);
    if(changed) {
        LOG_VERBOSE(TAG_GPIO, settingsChanged.c_str());
        configOutput(settings[module], TAG_GPIO);
        writefile = true;
    }
#endif

#if HASP_TARGET_ARDUINO
    module = FPSTR(FP_DEBUG);
    if(settings[module].as<JsonObject>().isNull()) settings.createNestedObject(module);
    changed = debugGetConfig(settings[module]);
    if(changed) {
        LOG_VERBOSE(TAG_DEBG, settingsChanged.c_str());
        configOutput(settings[module], TAG_DEBG);
        writefile = true;
    }
#endif

    if(settings[FPSTR(FP_GUI)].as<JsonObject>().isNull()) settings.createNestedObject(FPSTR(FP_GUI));
    changed = guiGetConfig(settings[FPSTR(FP_GUI)]);
    if(changed) {
        LOG_VERBOSE(TAG_GUI, settingsChanged.c_str());
        configOutput(settings[FPSTR(FP_GUI)], TAG_GUI);
        writefile = true;
    }

    if(settings[FPSTR(FP_HASP)].as<JsonObject>().isNull()) settings.createNestedObject(FPSTR(FP_HASP));
    changed = haspGetConfig(settings[FPSTR(FP_HASP)]);
    if(changed) {
        LOG_VERBOSE(TAG_HASP, settingsChanged.c_str());
        configOutput(settings[FPSTR(FP_HASP)], TAG_HASP);
        writefile = true;
    }

    // changed |= otaGetConfig(settings[F("ota")].as<JsonObject>());

    if(writefile) {
#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
        File file = HASP_FS.open(configFile, "w");
        if(file) {
            LOG_TRACE(TAG_CONF, F(D_FILE_SAVING), configFile.c_str());
            WriteBufferingStream bufferedFile(file, 256);
            size_t size = serializeJson(doc, bufferedFile);
            bufferedFile.flush();
            file.close();
            if(size > 0) {
                LOG_INFO(TAG_CONF, F(D_FILE_SAVED), configFile.c_str());
                // configBackupToEeprom();
            } else {
                LOG_ERROR(TAG_CONF, F(D_FILE_SAVE_FAILED), configFile.c_str());
            }
        } else {
            LOG_ERROR(TAG_CONF, F(D_FILE_SAVE_FAILED), configFile.c_str());
        }
#endif

        // Method 1
        // LOG_INFO(TAG_CONF,F("Writing to EEPROM"));
        // EepromStream eepromStream(0, 1024);
        // WriteBufferingStream bufferedWifiClient{eepromStream, 512};
        // serializeJson(doc, bufferedWifiClient);
        // bufferedWifiClient.flush(); // <- OPTIONAL
        // eepromStream.flush();       // (for ESP)

#if defined(STM32F4xx)
        // Method 2
        LOG_INFO(TAG_CONF, F(D_FILE_SAVING), "EEPROM");
        char buffer[1024 + 128];
        size_t size = serializeJson(doc, buffer, sizeof(buffer));
        if(size > 0) {
            uint16_t i;
            for(i = 0; i < size; i++) eeprom_buffered_write_byte(i, buffer[i]);
            eeprom_buffered_write_byte(i, 0);
            eeprom_buffer_flush();
            LOG_INFO(TAG_CONF, F(D_FILE_SAVED), "EEPROM");
        } else {
            LOG_ERROR(TAG_CONF, F(D_FILE_SAVE_FAILED), "EEPROM");
        }
#endif

    } else {
        LOG_INFO(TAG_CONF, F(D_CONFIG_NOT_CHANGED));
    }
    configOutput(settings, TAG_CONF);
}

void configSetup()
{
    DynamicJsonDocument settings(MAX_CONFIG_JSON_ALLOC_SIZE);

    for(uint32_t i = 0; i < 2; i++) {
        if(i == 0) {
#if HASP_USE_SPIFFS > 0
            EepromStream eepromStream(0, 2048);
            DeserializationError err = deserializeJson(settings, eepromStream);
#else
            continue;
#endif
        } else {
#if HASP_USE_SPIFFS > 0 || HASP_USE_LITTLEFS > 0
            if(!filesystemSetup()) {
                LOG_ERROR(TAG_CONF, F("FILE: SPI flash init failed. Unable to mount FS: Using default settings..."));
                // return; // Keep going and initialize the console with default settings
            }
#endif
            configRead(settings, true);
        }

#if HASP_TARGET_ARDUINO
        LOG_INFO(TAG_DEBG, F("Loading debug settings"));
        debugSetConfig(settings[FPSTR(FP_DEBUG)]);
#endif
        LOG_INFO(TAG_GPIO, F("Loading GUI settings"));
        guiSetConfig(settings[FPSTR(FP_GUI)]);
        LOG_INFO(TAG_HASP, F("Loading HASP settings"));
        haspSetConfig(settings[FPSTR(FP_HASP)]);
        // otaGetConfig(settings[F("ota")]);

#if HASP_USE_WIFI > 0
        LOG_INFO(TAG_WIFI, F("Loading WiFi settings"));
        wifiSetConfig(settings[FPSTR(FP_WIFI)]);
#endif

#if HASP_USE_WIREGUARD > 0
        LOG_INFO(TAG_WG, F("Loading WireGuard settings"));
        wgSetConfig(settings[FPSTR(FP_WG)]);
#endif

#if HASP_USE_MQTT > 0
        LOG_INFO(TAG_MQTT, F("Loading MQTT settings"));
        mqttSetConfig(settings[FPSTR(FP_MQTT)]);
#endif

#if HASP_USE_TELNET > 0
        LOG_INFO(TAG_TELN, F("Loading Telnet settings"));
        telnetSetConfig(settings[F("telnet")]);
#endif

#if HASP_USE_MDNS > 0
        LOG_INFO(TAG_MDNS, F("Loading MDNS settings"));
        mdnsSetConfig(settings[FPSTR(FP_MDNS)]);
#endif

#if HASP_USE_HTTP > 0
        LOG_INFO(TAG_HTTP, F("Loading HTTP settings"));
        httpSetConfig(settings[FPSTR(FP_HTTP)]);
#endif

#if HASP_USE_FTP > 0
    LOG_INFO(TAG_FTP, F("Loading FTP settings"));
    ftpSetConfig(settings[FPSTR(FP_FTP)]);
#endif

#if HASP_USE_CONFIG > 0
    LOG_INFO(TAG_TIME, F("Loading time settings"));
    timeSetConfig(settings[FPSTR(FP_TIME)]);
#endif

#if HASP_USE_GPIO > 0
        LOG_INFO(TAG_GPIO, F("Loading GPIO settings"));
        gpioSetConfig(settings[FPSTR(FP_GPIO)]);
#endif

        // target-specific config
#if defined(POSIX)
        LOG_INFO(TAG_CONF, F("Loading POSIX-specific settings"));
        haspDevice.set_config(settings[F("posix")]);
#endif

        LOG_INFO(TAG_CONF, F(D_CONFIG_LOADED));
    }
    // #endif
}

void configLoop(void)
{}

void configOutput(const JsonObject& settings, uint8_t tag)
{
    String output;
    output.reserve(128);
    serializeJson(settings, output);

    String passmask;
    passmask.reserve(128);
    passmask = F("\"pass\":\"" D_PASSWORD_MASK "\"");

    String password;
    password.reserve(128);

    String pass = F("pass");
    if(!settings[pass].isNull()) {
        password = F("\"pass\":\"");
        password += settings[pass].as<String>();
        password += F("\"");
#if HASP_TARGET_ARDUINO
        output.replace(password, passmask);
#elif HASP_TARGET_PC
        size_t pos = 0;
        if((pos = output.find(password)) != std::string::npos) output.replace(pos, password.size(), passmask);
#endif
    }

    if(!settings[FPSTR(FP_WIFI)][pass].isNull()) {
        password = F("\"pass\":\"");
        password += settings[FPSTR(FP_WIFI)][pass].as<String>();
        password += F("\"");
#if HASP_TARGET_ARDUINO
        output.replace(password, passmask);
#elif HASP_TARGET_PC
        size_t pos = 0;
        if((pos = output.find(password)) != std::string::npos) output.replace(pos, password.size(), passmask);
#endif
    }

    if(!settings[FPSTR(FP_MQTT)][pass].isNull()) {
        password = F("\"pass\":\"");
        password += settings[FPSTR(FP_MQTT)][pass].as<String>();
        password += F("\"");
#if HASP_TARGET_ARDUINO
        output.replace(password, passmask);
#elif HASP_TARGET_PC
        size_t pos = 0;
        if((pos = output.find(password)) != std::string::npos) output.replace(pos, password.size(), passmask);
#endif
    }

    if(!settings[FPSTR(FP_HTTP)][pass].isNull()) {
        password = F("\"pass\":\"");
        password += settings[FPSTR(FP_HTTP)][pass].as<String>();
        password += F("\"");
#if HASP_TARGET_ARDUINO
        output.replace(password, passmask);
#elif HASP_TARGET_PC
        size_t pos = 0;
        if((pos = output.find(password)) != std::string::npos) output.replace(pos, password.size(), passmask);
#endif
    }

    if(!settings[FPSTR(FP_WG)][FPSTR(FP_CONFIG_PRIVATE_KEY)].isNull()) {
        password = F("\"privkey\":\"");
        password += settings[FPSTR(FP_WG)][FPSTR(FP_CONFIG_PRIVATE_KEY)].as<String>();
        password += F("\"");
        passmask = F("\"privkey\":\"" D_PASSWORD_MASK "\"");
#if HASP_TARGET_ARDUINO
        output.replace(password, passmask);
#elif HASP_TARGET_PC
        size_t pos = 0;
        if((pos = output.find(password)) != std::string::npos) output.replace(pos, password.size(), passmask);
#endif
    }

    LOG_VERBOSE(tag, output.c_str());
}

bool configClearEeprom()
{
#if defined(STM32F4xx)
    LOG_TRACE(TAG_CONF, F("Clearing EEPROM"));
    char buffer[1024 + 128];
    memset(buffer, 1, sizeof(buffer));
    if(sizeof(buffer) > 0) {
        uint16_t i;
        for(i = 0; i < sizeof(buffer); i++) eeprom_buffered_write_byte(i, buffer[i]);
        eeprom_buffered_write_byte(i, 0);
        eeprom_buffer_flush();
        LOG_INFO(TAG_CONF, F("Cleared EEPROM"));
        return true;
    } else {
        LOG_ERROR(TAG_CONF, F("Failed to clear to EEPROM"));
        return false;
    }
#else
    return true; // nothing to clear
#endif
}

#endif // HAS_USE_CONFIG
