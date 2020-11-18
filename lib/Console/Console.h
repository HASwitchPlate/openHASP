#ifndef CONSOLE_H
#define CONSOLE_H

#include <Arduino.h>

class Console : public Stream {

  private:
    Stream * stream;

    char esc_sequence[10]; // escape sequence buffer
    char input_buf[220];   // input buffer and with history

    uint16_t last_read;
    size_t caret_pos;
    size_t history_index;

    bool insert_mode;
    bool debug_mode;
    bool enable_history;
    bool auto_update;

    uint16_t end_sequence(uint16_t key);
    int16_t append_esc_char();

    void do_backspace();
    void do_delete();

  public:
    // Declaration, initialization.
    static const int KEY_NONE    = -1;
    static const int KEY_UNKNOWN = 401;

    static const int KEY_LF        = 0x0a;
    static const int KEY_CR        = 0x0d;
    static const int KEY_PAUSE     = 0x1a;
    static const int KEY_ESC       = 0x1b;
    static const int KEY_BACKSPACE = 0x7f;

    static const int KEY_UP    = 256;
    static const int KEY_DOWN  = 257;
    static const int KEY_LEFT  = 258;
    static const int KEY_RIGHT = 259;

    static const int KEY_PAGE_UP   = 260;
    static const int KEY_PAGE_DOWN = 261;
    static const int KEY_INSERT    = 262;
    static const int KEY_DELETE    = 263;
    static const int KEY_HOME      = 264;
    static const int KEY_END       = 265;

    static const int MOD_SHIFT  = 1 << 10;
    static const int MOD_CTRL   = 1 << 11;
    static const int MOD_CMND   = 1 << 12;
    static const int MOD_ALT    = 1 << 13;
    static const int MOD_ALT_GR = 1 << 14;

    Console(Stream *);

    int16_t readKey();
    int16_t getChar(uint8_t index);

    bool insertCharacter(char ch);
    bool insertCharacter(char ch, size_t pos);

    void setCaret(int16_t index);
    int16_t getCaret(void);
    void update(void);

    void lineCallback();
    const char * getLine();
    void pushLine();
    void clearLine();

    size_t debugHistorycount();
    size_t debugHistoryIndex(size_t num);
    void debugShowHistory();

    virtual int available(void);
    virtual int peek(void);
    virtual int read(void);
    virtual void flush(void);
    virtual size_t write(uint8_t);

    using Print::write;
};

#endif