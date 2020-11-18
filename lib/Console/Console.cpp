/* MIT License - Copyright (c) 2020 Francis Van Roie francis@netwize.be
   For full license information read the LICENSE file in the project folder */

#include "Console.h"
#define TERM_CLEAR_LINE "\e[1000D\e[0K"
#define KEY_BUFFERED 0
#define KEY_FN 512
#define KEY_CTRL(n) (n - 64)

// Definitions
const int Console::KEY_NONE;
const int Console::KEY_UNKNOWN;

const int Console::KEY_BACKSPACE;
const int Console::KEY_LF;
const int Console::KEY_CR;

const int Console::KEY_UP;
const int Console::KEY_DOWN;
const int Console::KEY_LEFT;
const int Console::KEY_RIGHT;

const int Console::KEY_PAGE_UP;
const int Console::KEY_PAGE_DOWN;
const int Console::KEY_INSERT;
const int Console::KEY_DELETE;
const int Console::KEY_HOME;
const int Console::KEY_END;

const int Console::MOD_SHIFT;
const int Console::MOD_CTRL;
const int Console::MOD_CMND;
const int Console::MOD_ALT;
const int Console::MOD_ALT_GR;

uint16_t Console::end_sequence(uint16_t key)
{
    if(key != 0) {
        stream->println((int16_t)key);
    }

    //  Clear escape sequence buffer
    memset(esc_sequence, 0, sizeof(esc_sequence));
    return key;
}

Console::Console(Stream * serial)
{
    stream = serial;

    end_sequence(0);
    memset(input_buf, 0, sizeof(input_buf));

    insert_mode   = true;
    debug_mode    = false;
    auto_update   = true;
    history_index = 0;
    caret_pos     = 0;
    last_read     = millis();
}

int Console::available(void)
{
    return stream->available();
}

int Console::peek(void)
{
    return stream->peek();
}

int Console::read(void)
{
    last_read = millis();
    return stream->read();
}

void Console::flush(void)
{
    stream->flush();
}

size_t Console::write(uint8_t c)
{
    return stream->write(c);
}

void Console::do_backspace()
{
    history_index = 0;

    // history scrolling can make it go out-of-bounds
    size_t len = strnlen(input_buf, sizeof(input_buf));
    if(caret_pos > len) caret_pos = len;

    if(caret_pos <= 0) return;
    caret_pos--;

    char * src = input_buf + caret_pos + 1;
    char * dst = input_buf + caret_pos;
    memmove(dst, src, len - caret_pos);
}

void Console::do_delete()
{
    history_index = 0;

    size_t len = strnlen(input_buf, sizeof(input_buf));
    char * dst = input_buf + caret_pos;
    char * src = input_buf + caret_pos + 1;
    memmove(dst, src, len - caret_pos);
}

bool Console::insertCharacter(char ch, size_t pos)
{
    history_index = 0;

    // stream->print(ch);
    size_t len = strnlen(input_buf, sizeof(input_buf));

    // history invoke can make the index go out-of-bounds
    if(pos > len) pos = len;

    if(pos == len && pos < sizeof(input_buf) - 2) {
        // expand 1 character to the right
        if(input_buf[pos + 1] != 0) {
            // shift right needed
            char * dst = input_buf + len + 1;
            char * src = input_buf + len;
            memmove(dst, src, sizeof(input_buf) - len - 1);
        } else {
            // we still have room
        }
    }

    // Insert character if we have room
    if(pos < sizeof(input_buf) - 2) {
        if(pos + 1 >= len) input_buf[pos + 1] = 0;
        input_buf[pos] = ch;
        pos++;
        return true;
    }

    // Buffer is full
    return false;
}

bool Console::insertCharacter(char ch)
{
    if(insertCharacter(ch, caret_pos)) {
        caret_pos++;
        return true;
    }

    return false;
}

// Get the position of the caret on the input buffer
int16_t Console::getCaret()
{
    return caret_pos;
}

// Set the position of the caret on the input buffer
void Console::setCaret(int16_t index)
{
    history_index = 0;
    size_t len    = strnlen(input_buf, sizeof(input_buf));

    if(index > (int16_t)len) {
        caret_pos = len;
    } else if(index < 0) {
        caret_pos = 0;
    } else {
        caret_pos = index;
    }
}

// Print current input buffer
void Console::update()
{
    stream->print(F(TERM_CLEAR_LINE)); // Move all the way left + Clear the line
    stream->print(F("hasp > "));

    if(debug_mode) {

        for(uint i = 0; i < sizeof(input_buf); i++) {
            if(input_buf[i] == 0) {
                stream->print("|");
            } else {
                stream->print((char)input_buf[i]);
            }
        }
        stream->print(history_index);
        stream->print("/");
        /*stream->print(debugHistorycount());*/

    } else {

        stream->print(input_buf);
    }

    stream->print("\e[1000D"); // Move all the way left again

    /*if(caret_pos > 0)*/ {
        stream->print("\e[");
        stream->print(caret_pos + 7); // Move caret to index
        stream->print("C");
    }
}

int16_t Console::append_esc_char()
{
    char key;

    // no input available
    if(!stream->available()) {
        last_read = millis();
        return 0;
    }

    // buffer position not available, read but don't buffer
    if(esc_sequence[sizeof(esc_sequence) - 1] != 0) return stream->read();

    // it's zero terminated
    size_t pos        = strnlen(esc_sequence, sizeof(esc_sequence));
    key               = stream->read();
    esc_sequence[pos] = key;

    for(int i = 1; i < sizeof(esc_sequence); i++) {
        stream->printf("%2X %c", esc_sequence[i], esc_sequence[i]);
    }
    stream->println("append");

    return key;
}

int16_t Console::getChar(uint8_t index)
{
    int16_t key;

    // flush buffer if sequence is not closed in timely fashion
    if(esc_sequence[index] != 0x00 && ((uint16_t)millis() - last_read) > 100) {
        key = esc_sequence[index];
        memmove(esc_sequence, esc_sequence + 1, sizeof(esc_sequence) - 1);
        esc_sequence[sizeof(esc_sequence) - 1] = 0;
        return key; // flush one char at the time
    }

    // no input available
    if(!stream->available()) {
        last_read = millis();
        return 0;
    }

    // buffer position not available
    if(index >= sizeof(esc_sequence)) return stream->read();

    // buffer position available and used
    if(esc_sequence[index] != 0x00) return esc_sequence[index];

    // buffer position available but not used
    key                 = stream->read();
    esc_sequence[index] = key;

    for(int i = 1; i < sizeof(esc_sequence); i++) {
        stream->printf("%2X %c", esc_sequence[i], esc_sequence[i]);
    }
    stream->println("get");

    return key;
}

void Console::lineCallback()
{}

const char * Console::getLine()
{
    return input_buf;
}

void Console::pushLine()
{}

void Console::clearLine()
{
    size_t len = strnlen(input_buf, sizeof(input_buf));
    memset(input_buf, 0, len);
    caret_pos = 0;
}

size_t Console::debugHistorycount()
{
    size_t count = 0;
    for(size_t i = 1; i < sizeof(input_buf); i++) {
        if(input_buf[i] == 0 && input_buf[i - 1] != 0) count++;
    }
    return count;
}

size_t Console::debugHistoryIndex(size_t num)
{
    size_t pos = 0;
    while(num > 0 && pos < sizeof(input_buf) - 2) {
        if(input_buf[pos] == 0) {
            num--;
            // skip extra \0s
            while(input_buf[pos] == 0) {
                pos++;
            }
        } else {
            pos++;
        }
    }

    return pos;
}

void Console::debugShowHistory()
{
    size_t num = debugHistorycount();
    stream->println();
    for(size_t i = 0; i <= num; i++) {
        stream->print("[");
        stream->print(i);
        stream->print("] ");
        size_t pos = debugHistoryIndex(i);
        if(pos < sizeof(input_buf)) stream->println((char *)(input_buf + pos));
    }
}

// Read a key from the terminal or -1 if no key pressed
int16_t Console::readKey()
{
    int16_t key;

    key = getChar(0);
    if(key <= 0) return 0;

    if(key == 0x1b) { /* escape sequence */

        key = getChar(1);
        if(key <= 0) return 0;

        switch(key) {
            case '[': { // CSI mode
                char appended_key = append_esc_char();
                if(appended_key <= 0) return 0;

                switch(appended_key) {

                    case 0x30 ... 0x3F:  // parameter bytes
                    case 0x20 ... 0x2F:  // intermediate bytes
                        return KEY_NONE; // More data in flight

                    case 'A':
                        // size_t count = debugHistorycount();
                        //     if(history_index < count) {
                        //         history_index++;
                        //         debugGetHistoryLine(history_index);
                        //     }
                        end_sequence(0);
                        return KEY_UP;

                    case 'B':
                        // if(history_index > 0) {
                        //         history_index--;
                        //         debugGetHistoryLine(history_index);
                        //     }
                        end_sequence(0);
                        return KEY_DOWN;

                    case 'C':
                        setCaret(caret_pos + 1);
                        end_sequence(0);
                        return KEY_RIGHT;

                    case 'D':
                        setCaret(caret_pos - 1);
                        end_sequence(0);
                        return KEY_LEFT;

                    case 'E' ... 0x7D: // End Characters, esc_sequence is complete
                    case '@':
                        stream->println("UNKNOWN");
                        end_sequence(0);
                        return KEY_UNKNOWN;

                    case '~':
                        if(!strncmp_P(esc_sequence, PSTR("\e[1~"), sizeof(esc_sequence))) {
                            setCaret(strlen(input_buf));
                            end_sequence(0);
                            return KEY_HOME;
                        } else if(!strncmp_P(esc_sequence, PSTR("\e[2~"), sizeof(esc_sequence))) {
                            end_sequence(0);
                            return KEY_INSERT;
                        } else if(!strncmp_P(esc_sequence, PSTR("\e[3~"), sizeof(esc_sequence))) {
                            do_delete();
                            end_sequence(0);
                            return KEY_DELETE;
                        } else if(!strncmp_P(esc_sequence, PSTR("\e[4~"), sizeof(esc_sequence))) {
                            setCaret(strlen(input_buf));
                            end_sequence(0);
                            return KEY_END;
                        } else if(!strncmp_P(esc_sequence, PSTR("\e[5~"), sizeof(esc_sequence))) {
                            end_sequence(0);
                            return KEY_PAGE_UP;
                        } else if(!strncmp_P(esc_sequence, PSTR("\e[6~"), sizeof(esc_sequence))) {
                            end_sequence(0);
                            return KEY_PAGE_DOWN;
                        } else if(!strncmp_P(esc_sequence, PSTR("\e[15~"), sizeof(esc_sequence))) {
                            stream->println("F5");
                            end_sequence(0);
                        } else if(!strncmp_P(esc_sequence, PSTR("\e[17~"), sizeof(esc_sequence))) {
                            stream->println("F6");
                            end_sequence(0);
                        } else if(!strncmp_P(esc_sequence, PSTR("\e[18~"), sizeof(esc_sequence))) {
                            stream->println("F7");
                            end_sequence(0);
                        } else if(!strncmp_P(esc_sequence, PSTR("\e[19~"), sizeof(esc_sequence))) {
                            stream->println("F8");
                            end_sequence(0);
                        } else if(!strncmp_P(esc_sequence, PSTR("\e[20~"), sizeof(esc_sequence))) {
                            stream->println("F9");
                            end_sequence(0);
                        } else if(!strncmp_P(esc_sequence, PSTR("\e[21~"), sizeof(esc_sequence))) {
                            stream->println("F10");
                            end_sequence(0);
                        } else if(!strncmp_P(esc_sequence, PSTR("\e[23~"), sizeof(esc_sequence))) {
                            stream->println("F11");
                            end_sequence(0);
                        } else if(!strncmp_P(esc_sequence, PSTR("\e[24~"), sizeof(esc_sequence))) {
                            stream->println("F12");
                            end_sequence(0);
                        }

                        end_sequence(0);
                        return KEY_UNKNOWN;

                    default:
                        // should not happen
                        stream->println("WARNGING !!!");
                        return KEY_UNKNOWN;
                }
                break;

            } // CSI mode

            case 'O': { // CSO mode

                key = getChar(2);
                if(key < 0) return 0;

                switch(key) {
                    case 'P':
                        stream->println("F1");
                        end_sequence(0);
                        return KEY_FN + 1;
                    case 'Q':
                        stream->println("F2");
                        end_sequence(0);
                        return KEY_FN + 2;
                    case 'R':
                        stream->println("F3");
                        end_sequence(0);
                        return KEY_FN + 3;
                    case 'S':
                        stream->println("F4");
                        end_sequence(0);
                        return KEY_FN + 4;
                    default:
                        stream->println("UNKNOWN");
                        return KEY_UNKNOWN;
                }
                break;
            } // CSO mode

            default:
                return KEY_UNKNOWN;
        }
    }

    end_sequence(0);

    switch(key) {
        case KEY_CTRL('A')... KEY_CTRL('Z'): { // Ctrl + CHAR

            switch(key) {
                case KEY_CTRL('A'): // ^A = goto begin
                    caret_pos     = 0;
                    history_index = 0;
                    return KEY_CTRL('A');

                case KEY_CTRL('B'): // ^B = go back
                    caret_pos = strnlen(input_buf, sizeof(input_buf));

                    history_index = 0;
                    return KEY_CTRL('B');

                case KEY_CTRL('C'): // ^C = Break
                    caret_pos     = 0;
                    history_index = 0;
                    return KEY_CTRL('C');

                case KEY_CTRL('E'): // ^E = goto end
                    caret_pos     = strnlen(input_buf, sizeof(input_buf));
                    history_index = 0;
                    return KEY_CTRL('E');

                case KEY_CTRL('F'): // ^F = go forward
                    caret_pos     = strnlen(input_buf, sizeof(input_buf));
                    history_index = 0;
                    return KEY_CTRL('F');

                case 8: // Backspace
                    do_backspace();
                    stream->println("Backspace");
                    return KEY_BACKSPACE;

                case 9: // Delete
                    do_delete();
                    stream->println("Delete");
                    return KEY_DELETE;

                case KEY_LF ... KEY_CR: { // LF, VT, FF, CR
                                          // if(input_buf[0] != 0) {
                    // stream->println();
                    // dispatchTextLine(input_buf);

                    // size_t numchars = 1;
                    // memmove(input_buf + numchars, input_buf,
                    //         sizeof(input_buf) - numchars); // Shift chars right

                    // caret_pos   = 0;
                    // input_buf[0]  = 0;
                    // history_index = 0;
                    // debugShowHistory();
                    return key;
                }

                case 0x1a: // PAUSE
                    return KEY_PAUSE;
            }

            return end_sequence(KEY_BUFFERED);

        } // Ctrl + CHAR

        case 32 ... 126:
        case 128 ... 254:
            insertCharacter(key);
            return key;

        case 0x7f: // DELETE
            do_backspace();
            return KEY_BACKSPACE;
    }

    return key;
}
