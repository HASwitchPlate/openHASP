#ifndef _CHAR_STREAM_H_
#define _CHAR_STREAM_H_

#include <Stream.h>

class CharStream : public Stream {
  public:
    CharStream(char * s) : string(s), position(0)
    {
        length = strlen(s);
    }

    // Stream methods
    virtual int available()
    {
        return length - position;
    }
    virtual int read()
    {
        return position < length ? string[position++] : -1;
    }
    virtual int peek()
    {
        return position < length ? string[position] : -1;
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