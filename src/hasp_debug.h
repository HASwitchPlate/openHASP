#ifndef HASP_DEBUG_H
#define HASP_DEBUG_H

void debugSetup(void);
void debugLoop(void);
void debugStop(void);

void serialPrintln(String debugText);

void syslogSend(uint8_t log, const char * debugText);

#endif