/* MIT License - Copyright (c) 2019-2023 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_FTP_H
#define HASP_FTP_H

#if HASP_USE_FTP > 0

/* ===== Default Event Processors ===== */
void ftpSetup();
IRAM_ATTR void ftpLoop(void);
void ftpEvery5Seconds(void);
void ftpEverySecond(void);
void ftpStart(void);
void ftpStop(void);

/* ===== Special Event Processors ===== */

/* ===== Getter and Setter Functions ===== */

/* ===== Read/Write Configuration ===== */
#if HASP_USE_CONFIG > 0
bool ftpSetConfig(const JsonObject& settings);
bool ftpGetConfig(const JsonObject& settings);
#endif

#endif
#endif // HASP_FTP_H
