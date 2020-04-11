#ifndef _CHAR_STREAM_H_
#define _CHAR_STREAM_H_

#include <Stream.h>
#include "Arduino.h"

class CharStream : public Stream {
  public:
    CharStream(char * s) : string(s), position(0)
    {}

    // Stream methods
    virtual int available()
    {
        return strlen(string) - position;
    }
    virtual int read()
    {
        return position < strlen(string) ? string[position++] : -1;
    }
    virtual int peek()
    {
        return position < strlen(string) ? string[position] : -1;
    }
    virtual void flush(){};
    // Print methods
    virtual size_t write(uint8_t c)
    {
        /*   char buf[2];
             buf[0] = c;
             buf[1] = '\0';
             strncat((char *)string, buf, 1);
             return 1;*/
        return 0;
    };

  private:
    char * string;
    unsigned int length;
    unsigned int position;
};

#endif // _CHAR_STREAM_H_