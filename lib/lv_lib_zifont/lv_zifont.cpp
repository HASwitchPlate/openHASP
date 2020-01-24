/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>

#ifdef ESP32
#include "SPIFFS.h"
#endif
#include <FS.h>

#include "lvgl.h"
#include "lv_zifont.h"
#include "../src/hasp_log.h"

/*********************
 *      DEFINES
 *********************/
#define ColorBlack 0x0f
#define ColorWhite 0x00

/**********************
 *      TYPEDEFS
 **********************/
typedef enum zifont_error_t {
    ZIFONT_NO_ERROR,
    ZIFONT_ERROR_OUT_OF_MEMORY,
    ZIFONT_ERROR_OPENING_FILE,
    ZIFONT_ERROR_READING_DATA,
    ZIFONT_ERROR_UNKNOWN_HEADER
};

typedef enum zifont_codepage_t8_t {
    ASCII = 0x01,
    // GB2312     = 0x02,
    ISO_8859_1 = 0x03,
    /*   ISO_8859_2     = 0x04,
       ISO_8859_3     = 0x05,
       ISO_8859_4     = 0x06,
       ISO_8859_5     = 0x07,
       ISO_8859_6     = 0x08,
       ISO_8859_7     = 0x09,
       ISO_8859_8     = 0x0A,
       ISO_8859_9     = 0x0B,
       ISO_8859_13    = 0x0C,
       ISO_8859_15    = 0x0D,
       ISO_8859_11    = 0x0E,
       KS_C_5601_1987 = 0x0F,
       BIG5           = 0x10,
       WINDOWS_1255   = 0x11,
       WINDOWS_1256   = 0x12,
       WINDOWS_1257   = 0x13,
       WINDOWS_1258   = 0x14,
       WINDOWS_874    = 0x15,
       KOI8_R         = 0x16,
       SHIFT_JIS      = 0x17,*/
    UTF_8 = 0x18
};

/**********************
 *  STATIC PROTOTYPES
 **********************/
static const uint8_t * lv_font_get_bitmap_fmt_zifont(const lv_font_t * font, uint32_t unicode_letter);
static bool lv_font_get_glyph_dsc_fmt_zifont(const lv_font_t * font, lv_font_glyph_dsc_t * dsc_out,
                                             uint32_t unicode_letter, uint32_t unicode_letter_next);

/**********************
 *  STATIC VARIABLES
 **********************/
// static uint8_t * charBitmap_p[32 * 32];
uint32_t charInBuffer = 0;
// uint8_t filecharBitmap_p[20 * 1024];

#if ESP32
// uint8_t charBitmap_p[32 * 32];
static lv_zifont_char_t charCache[256 - 32];
#else
// uint8_t charBitmap_p[32 * 32];
static lv_zifont_char_t charCache[256 - 32];
#endif
static uint8_t * charBitmap_p;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void printBuffer(uint8_t * charBitmap_p, uint8_t w, uint8_t h);
uint16_t colorsAdd(uint8_t * charBitmap_p, uint8_t color1, uint8_t w, uint16_t pos);
uint16_t unicode2codepoint(uint32_t unicode, uint8_t codepage);

int lv_zifont_init(void)
{
    // charBitmap_p = (uint8_t *)lv_mem_alloc(32 * 32);
    return LV_RES_OK; // OK
}

int lv_zifont_font_init(lv_font_t * font, const char * font_path, uint16_t size)
{
    charInBuffer = 0; // invalidate any previous cache

    lv_font_fmt_zifont_dsc_t * dsc = (lv_font_fmt_zifont_dsc_t *)lv_mem_alloc(sizeof(lv_font_fmt_zifont_dsc_t));
    LV_ASSERT_MEM(dsc);
    if(dsc == NULL) return ZIFONT_ERROR_OUT_OF_MEMORY;
    int error = 0;

    /* Open the font for reading */
    File file = SPIFFS.open(font_path, "r");
    if(!file) {
        char msg[64];
        sprintf(msg, PSTR("FONT: Error %d while opening font: %s\n"), error, font_path);
        debugPrintln(msg);
        return ZIFONT_ERROR_OPENING_FILE;
    }

    /* Read file header as dsc */
    size_t readSize = file.readBytes((char *)dsc, sizeof(lv_font_fmt_zifont_dsc_t));

    /* Check that we read the correct size */
    if(readSize != sizeof(lv_font_fmt_zifont_dsc_t)) {
        debugPrintln(PSTR("FONT: Error reading ziFont Header"));
        return ZIFONT_ERROR_READING_DATA;
    }

    /* Check ziFile Header Format */
    if(dsc->Password != 4 || dsc->Version != 5) {
        debugPrintln(PSTR("FONT: Unknown font file format"));
        return ZIFONT_ERROR_UNKNOWN_HEADER;
    }

    /* read charmap into cache */
    file.seek(0 * sizeof(lv_zifont_char_t) + dsc->Descriptionlength + sizeof(lv_font_fmt_zifont_dsc_t), SeekSet);
    //* read and fill charmap cache
    readSize = file.readBytes((char *)charCache, sizeof(charCache));

    //* Check that we read the correct size
    if(readSize != sizeof(charCache)) {
        debugPrintln(PSTR("FONT: Error reading ziFont character map"));
        return ZIFONT_ERROR_READING_DATA;
    }

    file.close();

    char msg[128];
    sprintf_P(msg, PSTR("FONT: Loaded V%d Font File: %s containing %d characters"), dsc->Version, font_path,
              dsc->Maximumnumchars);
    debugPrintln(msg);

    /*
        sprintf_P(msg, PSTR("password: %u - skipL0: %u - skipLH: %u - state: %u\n"), dsc->Password, dsc->SkipL0,
                  dsc->SkipLH, dsc->State);
        Serial.printf(msg);
        sprintf_P(msg, PSTR("Orientation: %u - Encoding: %u - CharWidth: %u - CharHeight: %u\n"), dsc->Orientation,
                  dsc->Codepageid, dsc->CharWidth, dsc->CharHeight);
        Serial.printf(msg);
        sprintf_P(msg, PSTR("CodepageStart0: %u - CodepageEnd0: %u - CodepageStart1: %u - CodepageEnd1: %u\n"),
                  dsc->SecondByteStart, dsc->SecondByteEnd, dsc->FirstByteStart, dsc->FirstByteEnd);
        Serial.printf(msg);
        sprintf_P(msg, PSTR("CharCount: %u - FileVersion: %u - FontnameLength: %u - tbd: %u\n"), dsc->Maximumnumchars, ,
                  dsc->Descriptionlength, dsc->Zimobinbeg);
        Serial.printf(msg);
        sprintf_P(msg, PSTR("FontnameAndCmapLength: %u - FontnameStartAddr: %u - temp0: %u - temp1: %u\n"),
                  dsc->Totaldatalength, dsc->Startdataaddress, dsc->CodeT0, dsc->CodeDec);
        Serial.printf(msg);*/

    font->get_glyph_dsc    = lv_font_get_glyph_dsc_fmt_zifont; /*Function pointer to get glyph's data*/
    font->get_glyph_bitmap = lv_font_get_bitmap_fmt_zifont;    /*Function pointer to get glyph's bitmap*/
    font->line_height      = dsc->CharHeight;                  /*The maximum line height required by the font*/
    font->base_line        = 0;                                /*Baseline measured from the bottom of the line*/
    font->dsc   = dsc; /* header data struct */ /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
    font->subpx = 0;

    font->user_data = (char *)font_path;
    return ZIFONT_NO_ERROR;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Used as `get_glyph_bitmap` callback in LittelvGL's native font format if the font is uncompressed.
 * @param font pointer to font
 * @param unicode_letter an unicode letter which bitmap should be get
 * @return pointer to the bitmap or NULL if not found
 */
static const uint8_t * lv_font_get_bitmap_fmt_zifont(const lv_font_t * font, uint32_t unicode_letter)
{
    // ulong startMillis               = millis();
    lv_font_fmt_zifont_dsc_t * fdsc = (lv_font_fmt_zifont_dsc_t *)font->dsc; /* header data struct */
    int error                       = 0;
    uint32_t charNum                = unicode_letter - 0x20; // simple unicode to ascii - space is charNum=0

    if(charInBuffer == unicode_letter && charBitmap_p) {
        Serial.printf("CacheLetter %c\n", (char)(uint8_t)unicode_letter);
        return charBitmap_p;
    }

    /* Open the font for reading */
    File file = SPIFFS.open((char *)font->user_data, "r");
    if(!file) {
        debugPrintln(PSTR("FONT: [ERROR] while opening font:"));
        debugPrintln((char *)font->user_data);
        return NULL;
    }

    /* Read Character Table */
    lv_zifont_char_t * charInfo = (lv_zifont_char_t *)lv_mem_alloc(sizeof(lv_zifont_char_t));
    file.seek(charNum * 10 + fdsc->Descriptionlength + 0x2C, SeekSet);
    size_t readSize = file.readBytes((char *)charInfo, sizeof(lv_zifont_char_t));

    /* Check that we read the correct size */
    if(readSize != sizeof(lv_zifont_char_t)) {
        file.close();
        lv_mem_free(charInfo);
        debugPrintln(PSTR("FONT: [ERROR] Wrong number of bytes read from flash"));
        return NULL;
    }

    /* Double-check that we got the correct letter */
    if(charInfo->character != unicode_letter) {
        file.close();
        lv_mem_free(charInfo);
        debugPrintln(PSTR("FONT: [ERROR] Incorrect letter read from flash"));
        return NULL;
    }

    long datapos =
        fdsc->Descriptionlength + 0x2C + (charInfo->pos[2] << 16) + (charInfo->pos[1] << 8) + charInfo->pos[0];

    /* Allocate & Initialize Buffer for 4bpp */
    uint32_t size = (charInfo->width * fdsc->CharHeight + 1) / 2; // add 1 for rounding up
    if(charBitmap_p) lv_mem_free(charBitmap_p);
    charBitmap_p = (uint8_t *)lv_mem_alloc(size);
    memset(charBitmap_p, 0, size); // init the bitmap to white

    char ch[10];
    file.seek(datapos, SeekSet);
    // Serial.printf("Data start position :%d = %d\n", datapos, ftell(file));
    file.readBytes(ch, 1); /* check first byte = bpp */
    uint8_t b = ch[0];

    if(b != 3) {
        file.close();
        lv_mem_free(charInfo);
        debugPrintln(PSTR("FONT: [ERROR] Character is not 3bpp encoded"));
        return NULL;
    }

    uint16_t arrindex  = 0;
    uint8_t w          = charInfo->width + charInfo->kerningL + charInfo->kerningR;
    uint16_t fileindex = 0;

    char data[256];
    int len;

    while((fileindex < charInfo->length)) { //} && !feof(file)) {
        if(sizeof(data) < charInfo->length - fileindex) {
            len = file.readBytes(data, sizeof(data));
        } else {
            len = file.readBytes(data, charInfo->length - fileindex);
        }
        fileindex += len;

        for(uint8_t k = 0; k < len; k++) {
            uint8_t b = data[k];
            // Serial.printf("%d - %d > %x = %x  arrindex:%d\n", fileindex, arrindex, b, ch[0], ftell(file));

            uint8_t repeats = b & 0b00011111; /* last 5 bits indicate repetition as the same color */
            switch((uint8_t)b >> 5) {
                case(0b000):
                    // for(int i = 0; i < repeats; i++) {
                    //     arrindex += colorsAdd(charBitmap_p, ColorWhite, w, arrindex);
                    // }
                    arrindex += repeats;
                    break;

                case(0b001):
                    for(int i = 0; i < repeats; i++) {
                        arrindex += colorsAdd(charBitmap_p, ColorBlack, w, arrindex);
                    }
                    break;

                case(0b010):
                    // for(int i = 0; i < repeats; i++) {
                    //     arrindex += colorsAdd(charBitmap_p, ColorWhite, w, arrindex);
                    // }
                    arrindex += repeats;
                    arrindex += colorsAdd(charBitmap_p, ColorBlack, w, arrindex);
                    break;

                case(0b011):
                    // for(int i = 0; i < repeats; i++) {
                    //     arrindex += colorsAdd(charBitmap_p, ColorWhite, w, arrindex);
                    // }
                    arrindex += repeats;
                    arrindex += colorsAdd(charBitmap_p, ColorBlack, w, arrindex);
                    arrindex += colorsAdd(charBitmap_p, ColorBlack, w, arrindex);
                    break;

                case(0b100):
                case(0b101): {
                    repeats       = (uint8_t)((b & (0b111000)) >> 3); /* 3 bits indicate repetition as the same color */
                    uint8_t color = (uint8_t)(b & (0b0111));
                    // for(int i = 0; i < repeats; i++) {
                    //     arrindex += colorsAdd(charBitmap_p, ColorWhite, w, arrindex);
                    // }
                    arrindex += repeats;
                    arrindex += colorsAdd(charBitmap_p, color, w, arrindex);
                    break;
                }

                case(0b110):
                case(0b111): {
                    repeats        = 0;
                    uint8_t color1 = (b & 0b111000) >> 3;
                    uint8_t color2 = b & 0b000111;
                    arrindex += colorsAdd(charBitmap_p, color1, w, arrindex);
                    arrindex += colorsAdd(charBitmap_p, color2, w, arrindex);
                    break;
                }

                default:
                    debugPrintln(PSTR("FONT: [ERROR] Invalid drawing mode encounterd"));
                    file.close();
                    lv_mem_free(charInfo);
                    return NULL;
            }
        }
    }

    // Serial.printf("[OK] Letter %c - %d\n", (char)(uint8_t)unicode_letter, arrindex);
    // printBuffer(charBitmap_p, charInfo->width, fdsc->CharHeight);

    file.close();

    // debugPrintln("FONT: Bitmap " + String((char)charInfo->character) + " took " + String(millis() - startMillis) +
    // "ms");
    lv_mem_free(charInfo);
    charInBuffer = unicode_letter;

    return charBitmap_p;
}

/**
 * Used as `get_glyph_dsc` callback in LittelvGL's native font format if the font is uncompressed.
 * @param font_p pointer to font
 * @param dsc_out store the result descriptor here
 * @param letter an UNICODE letter code
 * @return true: descriptor is successfully loaded into `dsc_out`.
 *         false: the letter was not found, no data is loaded to `dsc_out`
 */
static bool lv_font_get_glyph_dsc_fmt_zifont(const lv_font_t * font, lv_font_glyph_dsc_t * dsc_out,
                                             uint32_t unicode_letter, uint32_t unicode_letter_next)
{
    // ulong startMillis               = millis();
    lv_font_fmt_zifont_dsc_t * fdsc = (lv_font_fmt_zifont_dsc_t *)font->dsc; /* header data struct */
    uint16_t charNum                = unicode2codepoint(unicode_letter, fdsc->Codepageid);
    int error                       = 0;

    /* Only ascii characteres supported for now */
    if(charNum < 0x20 || charNum > 0xff) return false;

    charNum -= 32;
    if(charCache[charNum].width == 0) {

        /* Open the font for reading */
        File file = SPIFFS.open((char *)font->user_data, "r");
        if(!file) {
            Serial.printf("Error %d in opening file: %s\n", error, (char *)font->user_data);
            return false;
        }

        /* read 10 bytes charmap */
        lv_zifont_char_t * myCharIndex = (lv_zifont_char_t *)lv_mem_alloc(sizeof(lv_zifont_char_t));
        file.seek((charNum - 32) * sizeof(lv_zifont_char_t) + fdsc->Descriptionlength +
                      sizeof(lv_font_fmt_zifont_dsc_t),
                  SeekSet);
        size_t readSize = file.readBytes((char *)myCharIndex, sizeof(lv_zifont_char_t));
        file.close();

        /* Check that we read the correct size */
        if(readSize != sizeof(lv_zifont_char_t)) {
            lv_mem_free(myCharIndex);
            return false;
        }

        /* Double-check that we got the correct letter */
        if(myCharIndex->character != unicode_letter) {
            lv_mem_free(myCharIndex);
            return false;
        }

        charCache[charNum] = *myCharIndex;
        lv_mem_free(myCharIndex);
    } else {
    }

    dsc_out->adv_w = charCache[charNum].width; //-myCharIndex->righroverlap)*16; /* 8 bit integer 4 bit fractional*/
    dsc_out->box_w = charCache[charNum].width + charCache[charNum].kerningL + charCache[charNum].kerningR;
    dsc_out->box_h = fdsc->CharHeight;
    dsc_out->ofs_x = -charCache[charNum].kerningL;
    dsc_out->ofs_y = 0;
    dsc_out->bpp   = 4; /**< Bit-per-pixel: 1, 2, 4, 8*/

    // Serial.printf("Letter %c\n", (char)(uint8_t)unicode_letter);

    // Serial.printf("adv_w %u (%u) - bpp %u  -  ", dsc_out->adv_w, myCharIndex->width, dsc_out->bpp);
    // Serial.printf("box_w %u  -  box_h %u   -  ", dsc_out->box_w, dsc_out->box_h);
    // Serial.printf("kernL %u  -  kernR %u   \n", myCharIndex->kerningL, myCharIndex->kerningR);
    // Serial.printf("ofs_x %u - ofs_y %u \n\n", dsc_out->ofs_x, dsc_out->ofs_x);

    //    debugPrintln("FONT: Char " + String((char)myCharIndex->character) + " lookup took " + String(millis() -
    //    startMillis) + "ms");
    return true;
}

uint16_t colorsAdd(uint8_t * charBitmap_p, uint8_t color1, uint8_t w, uint16_t pos)
{
    uint16_t map_p = pos >> 1;
    uint8_t col    = pos % 2;
    color1         = color1 & 0b1111;

    if(color1 != ColorBlack) {
        //  && color1 != ColorWhite) { // Don't check white, as the function is only used for colors
        color1 <<= 1; // 3bpp to 4bpp
    }

    // Serial.printf("%u color %u\n", pos, color1);
    if(col == 0) {
        charBitmap_p[map_p] = color1 << 4;
    } else {
        charBitmap_p[map_p] |= color1;
    }

    return 1; // shift 1 position
}

/*
void printPixel(uint8_t pixel)
{
    switch(pixel >> 1) {
        case 0:
            Serial.printf(".");
            break;
        case 1:
            Serial.printf("-");
            break;
        case 2:
            Serial.printf("+");
            break;
        case 3:
            Serial.printf("*");
            break;
        case 4:
            Serial.printf("à");
            break;
        case 5:
            Serial.printf("$");
            break;
        case 6:
            Serial.printf("#");
            break;
        case 7:
        case 15:
            Serial.printf("@");
            break;
        default:
            Serial.printf(" ?? ");
    }
}

void printBuffer(uint8_t * charBitmap_p, uint8_t w, uint8_t h)
{
    uint8_t cols = w + w % 2;
    cols /= 2;

    for(uint8_t i = 0; i < h; i++) {
        for(uint8_t j = 0; j < cols; j++) {
            uint8_t b = charBitmap_p[i * cols + j];
            printPixel(b >> 4);
            printPixel(b & 0b1111);
        }
        Serial.println("");
    }
    Serial.println("/end");
}
*/

uint16_t unicode2codepoint(uint32_t unicode, uint8_t codepage)
{
#ifdef ESP8266
    // ESP8266 needs the memory
    if(unicode < 128) return unicode;
    return 0;
    return (uint16_t)unicode;
#else
    if(unicode < 128) return unicode;
    if(unicode > 65535) return 0;

    switch(codepage) {
        case ASCII:
        case ISO_8859_1:
        case UTF_8:
            return unicode;
            /*
                    case ISO_8859_2:
                        switch(unicode) {
                            case 0x0104:
                                return 0xA1;
                            case 0x0141:
                                return 0xA3;
                            case 0x013D:
                                return 0xA5;
                            case 0x015A:
                                return 0xA6;
                            case 0x0160:
                                return 0xA9;
                            case 0x015E:
                                return 0xAA;
                            case 0x0164:
                                return 0xAB;
                            case 0x0179:
                                return 0xAC;
                            case 0x017D:
                                return 0xAE;
                            case 0x017B:
                                return 0xAF;
                            case 0x0105:
                                return 0xB1;
                            case 0x02DB:
                                return 0xB2;
                            case 0x0142:
                                return 0xB3;
                            case 0x013E:
                                return 0xB5;
                            case 0x015B:
                                return 0xB6;
                            case 0x02C7:
                                return 0xB7;
                            case 0x0161:
                                return 0xB9;
                            case 0x015F:
                                return 0xBA;
                            case 0x0165:
                                return 0xBB;
                            case 0x017A:
                                return 0xBC;
                            case 0x02DD:
                                return 0xBD;
                            case 0x017E:
                                return 0xBE;
                            case 0x017C:
                                return 0xBF;
                            case 0x0154:
                                return 0xC0;
                            case 0x0102:
                                return 0xC3;
                            case 0x0139:
                                return 0xC5;
                            case 0x0106:
                                return 0xC6;
                            case 0x010C:
                                return 0xC8;
                            case 0x0118:
                                return 0xCA;
                            case 0x011A:
                                return 0xCC;
                            case 0x010E:
                                return 0xCF;
                            case 0x0110:
                                return 0xD0;
                            case 0x0143:
                                return 0xD1;
                            case 0x0147:
                                return 0xD2;
                            case 0x0150:
                                return 0xD5;
                            case 0x0158:
                                return 0xD8;
                            case 0x016E:
                                return 0xD9;
                            case 0x0170:
                                return 0xDB;
                            case 0x0162:
                                return 0xDE;
                            case 0x0155:
                                return 0xE0;
                            case 0x0103:
                                return 0xE3;
                            case 0x013A:
                                return 0xE5;
                            case 0x0107:
                                return 0xE6;
                            case 0x010D:
                                return 0xE8;
                            case 0x0119:
                                return 0xEA;
                            case 0x011B:
                                return 0xEC;
                            case 0x010F:
                                return 0xEF;
                            case 0x0111:
                                return 0xF0;
                            case 0x0144:
                                return 0xF1;
                            case 0x0148:
                                return 0xF2;
                            case 0x0151:
                                return 0xF5;
                            case 0x0159:
                                return 0xF8;
                            case 0x016F:
                                return 0xF9;
                            case 0x0171:
                                return 0xFB;
                            case 0x0163:
                                return 0xFE;
                            case 0x02D9:
                                return 0xFF;
                        }

                    case ISO_8859_3:
                        switch(unicode) {
                            case 0x0126:
                                return 0xA1;
                            case 0x02D8:
                                return 0xA2;
                            case 0x0124:
                                return 0xA6;
                            case 0x0130:
                                return 0xA9;
                            case 0x015E:
                                return 0xAA;
                            case 0x011E:
                                return 0xAB;
                            case 0x0134:
                                return 0xAC;
                            case 0x017B:
                                return 0xAF;
                            case 0x0127:
                                return 0xB1;
                            case 0x0125:
                                return 0xB6;
                            case 0x0131:
                                return 0xB9;
                            case 0x015F:
                                return 0xBA;
                            case 0x011F:
                                return 0xBB;
                            case 0x0135:
                                return 0xBC;
                            case 0x017C:
                                return 0xBF;
                            case 0x010A:
                                return 0xC5;
                            case 0x0108:
                                return 0xC6;
                            case 0x011C:
                                return 0xD8;
                            case 0x016C:
                                return 0xDD;
                            case 0x015C:
                                return 0xDE;
                            case 0x010B:
                                return 0xE5;
                            case 0x0109:
                                return 0xE6;
                            case 0x0121:
                                return 0xF5;
                            case 0x011D:
                                return 0xF8;
                            case 0x016D:
                                return 0xFD;
                            case 0x015D:
                                return 0xFE;
                            case 0x02D9:
                                return 0xFF;
                        }

                   








            */
        default:
            return 0;
    }

    if(unicode < 256) return unicode;
    return 0;
#endif
}