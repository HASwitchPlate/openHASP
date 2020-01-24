#ifndef HASP_DEBUG_H
#define HASP_DEBUG_H

void debugSetup(void);
void debugLoop(void);
void debugStop(void);

void serialPrintln(String debugText);

#endif