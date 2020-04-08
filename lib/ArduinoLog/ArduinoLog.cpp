/*
    _   ___ ___  _   _ ___ _  _  ___  _    ___   ___
   /_\ | _ \   \| | | |_ _| \| |/ _ \| |  / _ \ / __|
  / _ \|   / |) | |_| || || .` | (_) | |_| (_) | (_ |
 /_/ \_\_|_\___/ \___/|___|_|\_|\___/|____\___/ \___|

  Log library for Arduino
  version 1.0.3
  https://github.com/thijse/Arduino-Log

Licensed under the MIT License <http://opensource.org/licenses/MIT>.

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "ArduinoLog.h"

void Logging::begin(int level, bool showLevel)
{
#ifndef DISABLE_LOGGING
    setLevel(0, level);
    setLevel(1, level);
    setLevel(2, level);
    setShowLevel(0, showLevel);
#endif
}

void Logging::registerOutput(uint8_t slot, Print * logOutput, int level, bool showLevel)
{
#ifndef DISABLE_LOGGING
    setLevel(0, level);
    setShowLevel(0, showLevel);
    if(slot >= 3) return;
    _logOutput[slot] = logOutput;
#endif
}

void Logging::unregisterOutput(uint8_t slot)
{
#ifndef DISABLE_LOGGING
    if(slot >= 3) return;
    _logOutput[slot] = NULL;
#endif
}

void Logging::setLevel(uint8_t slot, int level)
{
#ifndef DISABLE_LOGGING
    _level[slot] = constrain(level, LOG_LEVEL_SILENT, LOG_LEVEL_VERBOSE);
#endif
}

int Logging::getLevel(uint8_t slot) const
{
#ifndef DISABLE_LOGGING
    return _level[slot];
#else
    return 0;
#endif
}

void Logging::setShowLevel(uint8_t slot, bool showLevel)
{
#ifndef DISABLE_LOGGING
    _showLevel[slot] = showLevel;
#endif
}

bool Logging::getShowLevel(uint8_t slot) const
{
#ifndef DISABLE_LOGGING
    return _showLevel[slot];
#else
    return false;
#endif
}

void Logging::setPrefix(printfunction f)
{
#ifndef DISABLE_LOGGING
    _prefix = f;
#endif
}

void Logging::setSuffix(printfunction f)
{
#ifndef DISABLE_LOGGING
    _suffix = f;
#endif
}

void Logging::print(Print * logOutput, const __FlashStringHelper * format, va_list args)
{
#ifndef DISABLE_LOGGING
    PGM_P p = reinterpret_cast<PGM_P>(format);
    char c  = pgm_read_byte(p++);
    for(; c != 0; c = pgm_read_byte(p++)) {
        if(c == '%') {
            c = pgm_read_byte(p++);
            printFormat(logOutput, c, (va_list *)&args);
        } else {
            logOutput->print(c);
        }
    }
#endif
}

void Logging::print(Print * logOutput, const char * format, va_list args)
{
#ifndef DISABLE_LOGGING
    for(; *format != 0; ++format) {
        if(*format == '%') {
            ++format;
            printFormat(logOutput, *format, (va_list *)&args);
        } else {
            //_logOutput->print(*format);
            logOutput->print(*format);
        }
    }
#endif
}

void Logging::printFormat(Print * logOutput, const char format, va_list * args)
{
#ifndef DISABLE_LOGGING
    if(format == '%') {
        logOutput->print(format);
    } else if(format == 's') {
        register char * s = (char *)va_arg(*args, int);
        logOutput->print(s);
    } else if(format == 'S') {
        register __FlashStringHelper * s = (__FlashStringHelper *)va_arg(*args, int);
        logOutput->print(s);
    } else if(format == 'd' || format == 'i') {
        logOutput->print(va_arg(*args, int), DEC);
    } else if(format == 'u') {
        logOutput->print(va_arg(*args, unsigned int), DEC);
    } else if(format == 'D' || format == 'F') {
        logOutput->print(va_arg(*args, double));
    } else if(format == 'x') {
        logOutput->print(va_arg(*args, int), HEX);
    } else if(format == 'X') {
        logOutput->print("0x");
        logOutput->print(va_arg(*args, int), HEX);
    } else if(format == 'b') {
        logOutput->print(va_arg(*args, int), BIN);
    } else if(format == 'B') {
        logOutput->print("0b");
        logOutput->print(va_arg(*args, int), BIN);
    } else if(format == 'l') {
        logOutput->print(va_arg(*args, long), DEC);
    } else if(format == 'c') {
        logOutput->print((char)va_arg(*args, int));
    } else if(format == 't') {
        if(va_arg(*args, int) == 1) {
            logOutput->print("T");
        } else {
            logOutput->print("F");
        }
    } else if(format == 'T') {
        if(va_arg(*args, int) == 1) {
            logOutput->print(F("true"));
        } else {
            logOutput->print(F("false"));
        }
    }
#endif
}

Logging Log = Logging();
