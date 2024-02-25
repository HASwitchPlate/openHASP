/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

/*********************
 *      INCLUDES
 *********************/

#if !(HASP_TARGET_PC || defined(STM32F7xx))

#include <Arduino.h>
#include <stdio.h>

#if defined(ARDUINO_ARCH_ESP32)
#if HASP_USE_SPIFFS > 0
#include "SPIFFS.h"
#define FS SPIFFS
#elif HASP_USE_LITTLEFS > 0

#ifndef ESP_ARDUINO_VERSION_VAL
#define ESP_ARDUINO_VERSION_VAL(major, minor, patch) ((major << 16) | (minor << 8) | (patch))
#endif

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(2, 0, 0)
#include <LittleFS.h>
#define FS LittleFS
#else
#include "LITTLEFS.h"
#include "esp_littlefs.h"
#define FS LITTLEFS
#endif // ESP_ARDUINO_VERSION

#endif
#elif defined(ARDUINO_ARCH_ESP8266)
#include "LittleFS.h"
#define FS LittleFS
#endif // ARDUINO_ARCH

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
#include <FS.h>
#include <Esp.h>
#endif // ARDUINO_ARCH

#include "lvgl.h"
#include "lv_misc/lv_debug.h"
#include "lv_zifont.h"
#include "ArduinoLog.h"
#include "hasp_macro.h"

/*********************
 *      DEFINES
 *********************/
#define ColorBlack 0x0f
#define ColorWhite 0x00

#define TAG_FONT 92

/**********************
 *      TYPEDEFS
 **********************/
enum zifont_error_t {
    ZIFONT_NO_ERROR,
    ZIFONT_ERROR_OUT_OF_MEMORY,
    ZIFONT_ERROR_OPENING_FILE,
    ZIFONT_ERROR_READING_DATA,
    ZIFONT_ERROR_UNKNOWN_HEADER
};

enum zifont_codepage_t8_t { ASCII = 0x01, ISO_8859_1 = 0x03, UTF_8 = 0x18 };

/**********************
 *  STATIC PROTOTYPES
 **********************/
HASP_ATTRIBUTE_FAST_MEM const uint8_t* lv_font_get_bitmap_fmt_zifont(const lv_font_t* font, uint32_t unicode_letter);
HASP_ATTRIBUTE_FAST_MEM bool lv_font_get_glyph_dsc_fmt_zifont(const lv_font_t* font, lv_font_glyph_dsc_t* dsc_out,
                                                              uint32_t unicode_letter, uint32_t unicode_letter_next);

/**********************
 *  STATIC VARIABLES
 **********************/
uint32_t charInBuffer = 0; // Last Character ID in the Bitmap Buffer
// uint8_t filecharBitmap_p[20 * 1024];
lv_zifont_char_t lastCharInfo; // Holds the last Glyph DSC

#if ESP32
// static lv_zifont_char_t charCache[256 - 32]; // glyphID DSC cache
#define CHAR_CACHE_SIZE 224
#else
#define CHAR_CACHE_SIZE 95
// static lv_zifont_char_t charCache[256 - 32]; // glyphID DSC cache
#endif
static uint8_t* charBitmap_p;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

HASP_ATTRIBUTE_FAST_MEM static void blackAdd(uint8_t* charBitmap_p, uint16_t pos);
HASP_ATTRIBUTE_FAST_MEM static void colorsAdd(uint8_t* charBitmap_p, uint8_t color1, uint16_t pos);
// static uint16_t unicode2codepoint(uint32_t unicode, uint8_t codepage);
// static void printBuffer(uint8_t * charBitmap_p, uint8_t w, uint8_t h);

int lv_zifont_init(void)
{
    // FS.begin(true);
    // charBitmap_p = (uint8_t *)lv_mem_alloc(32 * 32);
    return LV_RES_OK; // OK
}

static inline bool openFont(File& file, const char* filename)
{
    if(*filename != '/') return false;

    file = FS.open(filename, "r");
    if(!file) {
        LOG_ERROR(TAG_FONT, F("Opening font: %s"), filename);
        return false;
    }
    // LOG_TRACE(TAG_FONT, F("Opening font: %s"), filename);
    return file;
}

static inline bool initCharacterFrame(size_t size)
{
    if(size > _lv_mem_get_size(charBitmap_p)) {
        lv_mem_free(charBitmap_p);
        charBitmap_p = (uint8_t*)lv_mem_alloc(size);
        LOG_WARNING(TAG_FONT, F("Pixel buffer is %d bytes"), _lv_mem_get_size(charBitmap_p));
    }

    if(charBitmap_p != NULL) {
        _lv_memset_00(charBitmap_p, size); // init the bitmap to white
        return true;
    } else {
        LOG_ERROR(TAG_FONT, F("Failed to allocate pixel buffer"));
        return false;
    }
}

int lv_zifont_font_init(lv_font_t** font, const char* font_path, uint16_t size)
{
    charInBuffer = 0; // invalidate any previous cache
    LOG_TRACE(TAG_FONT, F("File %s - Line %d - lv_zifont_font_init"), __FILE__, __LINE__);

    if(!*font) {
        LOG_TRACE(TAG_FONT, F("File %s - Line %d - init font"), __FILE__, __LINE__);
        *font = (lv_font_t*)lv_mem_alloc(sizeof(lv_font_t));
        LV_ASSERT_MEM(*font);
        _lv_memset_00(*font, sizeof(lv_font_t)); // lv_mem_alloc might be dirty
    }

    lv_font_fmt_zifont_dsc_t* dsc;
    if(!(*font)->dsc) {
        LOG_TRACE(TAG_FONT, F("File %s - Line %d - init font dsc"), __FILE__, __LINE__);
        dsc = (lv_font_fmt_zifont_dsc_t*)lv_mem_alloc(sizeof(lv_font_fmt_zifont_dsc_t));
        LV_ASSERT_MEM(dsc);
        _lv_memset_00(dsc, sizeof(lv_font_fmt_zifont_dsc_t)); // lv_mem_alloc might be dirty
        dsc->ascii_glyph_dsc = NULL;
    } else {
        LOG_TRACE(TAG_FONT, F("File %s - Line %d - reuse font dsc"), __FILE__, __LINE__);
        dsc = (lv_font_fmt_zifont_dsc_t*)(*font)->dsc;
    }
    LV_ASSERT_MEM(dsc);
    if(!dsc) return ZIFONT_ERROR_OUT_OF_MEMORY;

    /* Initialize Last Glyph DSC */
    dsc->last_glyph_dsc = (lv_zifont_char_t*)lv_mem_alloc(sizeof(lv_zifont_char_t));
    _lv_memset_00(dsc->last_glyph_dsc, sizeof(lv_zifont_char_t)); // lv_mem_alloc might be dirty

    if(dsc->last_glyph_dsc == NULL) return ZIFONT_ERROR_OUT_OF_MEMORY;
    dsc->last_glyph_dsc->width = 0;
    dsc->last_glyph_id         = 0;

    /* Open the font for reading */
    File file;
    if(!openFont(file, font_path)) return ZIFONT_ERROR_OPENING_FILE;

    /* Read file header as dsc */
    zi_font_header_t header;
    size_t readSize = file.readBytes((char*)&header, sizeof(zi_font_header_t));

    LOG_TRACE(TAG_FONT, F("File %s - Line %d"), __FILE__, __LINE__);

    /* Check that we read the correct size */
    if(readSize != sizeof(zi_font_header_t)) {
        LOG_ERROR(TAG_FONT, F("Error reading ziFont Header"));
        file.close();
        return ZIFONT_ERROR_READING_DATA;
    }

    LOG_TRACE(TAG_FONT, F("File %s - Line %d"), __FILE__, __LINE__);

    /* Check ziFile Header Format */
    if(header.Password != 4 || header.Version != 5) {
        LOG_ERROR(TAG_FONT, F("Unknown font file format"));
        file.close();
        return ZIFONT_ERROR_UNKNOWN_HEADER;
    }

    LOG_TRACE(TAG_FONT, F("File %s - Line %d"), __FILE__, __LINE__);

    dsc->CharHeight       = header.CharHeight;
    dsc->CharWidth        = header.CharWidth;
    dsc->Maximumnumchars  = header.Maximumnumchars;
    dsc->Actualnumchars   = header.Actualnumchars;
    dsc->Totaldatalength  = header.Totaldatalength;
    dsc->Startdataaddress = header.Startdataaddress + header.Descriptionlength;
    dsc->Fontdataadd8byte = header.Fontdataadd8byte;

    LOG_TRACE(TAG_FONT, F("File %s - Line %d - %d"), __FILE__, __LINE__, dsc->ascii_glyph_dsc);

    if(dsc->ascii_glyph_dsc == NULL) {
        LOG_TRACE(TAG_FONT, F("File %s - Line %d - ascii_glyph_dsc init"), __FILE__, __LINE__);
        dsc->ascii_glyph_dsc = (lv_zifont_char_t*)lv_mem_alloc(sizeof(lv_zifont_char_t) * CHAR_CACHE_SIZE);
        LV_ASSERT_MEM(dsc->ascii_glyph_dsc);
        _lv_memset_00(dsc->ascii_glyph_dsc, sizeof(lv_zifont_char_t) * CHAR_CACHE_SIZE); // lv_mem_alloc might be dirty
    }

    LOG_TRACE(TAG_FONT, F("File %s - Line %d"), __FILE__, __LINE__);

    if(dsc->ascii_glyph_dsc == NULL) {
        file.close();
        return ZIFONT_ERROR_OUT_OF_MEMORY;
    }

    LOG_TRACE(TAG_FONT, F("File %s - Line %d - Seerkset: %d"), __FILE__, __LINE__, dsc->Startdataaddress);

    /* read charmap into cache */
    file.seek(0 * sizeof(zi_font_header_t) + dsc->Startdataaddress, SeekSet);

    LOG_TRACE(TAG_FONT, F("File %s - Line %d"), __FILE__, __LINE__);

    //* read and fill charmap cache
    readSize = file.readBytes((char*)dsc->ascii_glyph_dsc, sizeof(lv_zifont_char_t) * CHAR_CACHE_SIZE);

    LOG_TRACE(TAG_FONT, F("File %s - Line %d"), __FILE__, __LINE__);

    //* Check that we read the correct size
    if(readSize != sizeof(lv_zifont_char_t) * CHAR_CACHE_SIZE) {
        LOG_ERROR(TAG_FONT, F("Error reading ziFont character map"));
        file.close();
        return ZIFONT_ERROR_READING_DATA;
    }

    LOG_VERBOSE(TAG_FONT, F("Loaded V%d Font File: %s containing %d characters"), header.Version, font_path,
                header.Maximumnumchars);

    file.close();

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
        sprintf_P(msg, PSTR("CharCount: %u - FileVersion: %u - FontnameLength: %u - tbd: %u\n"),
       dsc->Maximumnumchars, , dsc->Descriptionlength, dsc->Zimobinbeg); Serial.printf(msg); sprintf_P(msg,
       PSTR("FontnameAndCmapLength: %u - FontnameStartAddr: %u - temp0: %u - temp1: %u\n"), dsc->Totaldatalength,
       dsc->Startdataaddress, dsc->CodeT0, dsc->CodeDec); Serial.printf(msg);*/

    /* Init Glyph Cache */
    dsc->last_glyph_dsc = NULL;
    dsc->last_glyph_id  = 0;

    (*font)->get_glyph_dsc    = lv_font_get_glyph_dsc_fmt_zifont; /*Function pointer to get glyph's data*/
    (*font)->get_glyph_bitmap = lv_font_get_bitmap_fmt_zifont;    /*Function pointer to get glyph's bitmap*/
    (*font)->line_height      = dsc->CharHeight;                  /*The maximum line height required by the font*/
    (*font)->base_line        = 0;                                /*Baseline measured from the bottom of the line*/
    (*font)->dsc              = dsc;
    /* header data struct */ /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
    (*font)->subpx = 0;

    LOG_TRACE(TAG_FONT, F("File %s - Line %d"), __FILE__, __LINE__);

    if((*font)->user_data != (char*)font_path) {
        if((*font)->user_data) free((*font)->user_data);
        (*font)->user_data = (char*)font_path;
    }

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
HASP_ATTRIBUTE_FAST_MEM const uint8_t* lv_font_get_bitmap_fmt_zifont(const lv_font_t* font, uint32_t unicode_letter)
{
    /* Bitmap still in buffer */
    if(charInBuffer == unicode_letter && charBitmap_p) {
        // Serial.printf("CacheLetter %c\n", (char)(uint8_t)unicode_letter);
        // Serial.printf("#%c", (char)(uint8_t)unicode_letter);
        return charBitmap_p;
    }

    lv_font_fmt_zifont_dsc_t* fdsc = (lv_font_fmt_zifont_dsc_t*)font->dsc; /* header data struct */
    lv_zifont_char_t* charInfo;

    /* Space */
    if(unicode_letter == 0x20) {
        charInfo    = &fdsc->ascii_glyph_dsc[0];
        size_t size = (charInfo->width * fdsc->CharHeight + 1) / 2; // add 1 for rounding up
        if(initCharacterFrame(size)) {
            charInBuffer = unicode_letter;
        } else {
            charInBuffer = 0;
        }
        return charBitmap_p;
    }

    File file;
    char filename[32];
    uint32_t glyphID;
    uint16_t charmap_position;

    if(unicode_letter >= 0xF000) {
        // return NULL;
        snprintf_P(filename, sizeof(filename), PSTR("/fontawesome%u.zi"), fdsc->CharHeight);
        charmap_position = 25 + sizeof(zi_font_header_t);
        glyphID          = unicode_letter - 0xf000; // start of fontawesome
    } else {
        strcpy(filename, (char*)font->user_data);
        charmap_position = fdsc->Startdataaddress;
        glyphID          = unicode_letter - 0x20; // simple unicode to ascii - space is charNum=0
    }

    if(!openFont(file, filename)) return NULL;

    /* Check Last Glyph in chache is valid and Matches currentGlyphID */
    if(fdsc->last_glyph_id == glyphID && fdsc->last_glyph_dsc && fdsc->last_glyph_dsc->width > 0) {
        // Serial.print("@");
        charInfo = fdsc->last_glyph_dsc;
    } else {
        Serial.print("%");
        /* Read Character Table */
        charInfo = (lv_zifont_char_t*)lv_mem_alloc(sizeof(lv_zifont_char_t));
        // lv_memset(charInfo, 0x00, sizeof(lv_zifont_char_t)); // lv_mem_alloc might be dirty
        uint32_t char_position = glyphID * sizeof(lv_zifont_char_t) + charmap_position;
        file.seek(char_position, SeekSet);
        size_t readSize = file.readBytes((char*)charInfo, sizeof(lv_zifont_char_t));

        /* Check that we read the correct size */
        if(readSize != sizeof(lv_zifont_char_t)) {
            file.close();
            lv_mem_free(charInfo);
            LOG_ERROR(TAG_FONT, F("Wrong number of bytes read from flash"));
            return NULL;
        }

        /* Double-check that we got the correct letter */
        if(charInfo->character != unicode_letter) {
            file.close();
            lv_mem_free(charInfo);
            LOG_ERROR(TAG_FONT, F("Incorrect letter read from flash"));
            return NULL;
        }
    }

    long datapos = charmap_position + (charInfo->pos[2] << 16) + (charInfo->pos[1] << 8) + charInfo->pos[0];

    /* Allocate & Initialize Buffer for 4bpp */
    uint32_t size = (charInfo->width * fdsc->CharHeight + 1) / 2; // add 1 for rounding up
    if(!initCharacterFrame(size)) {
        return NULL;
    }

    char data[256];
    file.seek(datapos + 1, SeekSet); // +1 for skipping bpp byte

    /* Speed optimization, skip BPP check
    char b[1];
    file.seek(datapos, SeekSet);
    file.readBytes(b, 1); // check first byte = bpp

    if((uint8_t)b[0] != 3) {
        file.close();
        lv_mem_free(charInfo);
        snprintf_P(data, sizeof(data), PSTR("[ERROR] Character %u at %u is not 3bpp encoded but %u"), glyphID,
                   datapos, b[0]);
        debugPrintln(data);

        // Serial.printf("  adv_w %u (%u) - bpp %u  -  ", dsc_out->adv_w, charInfo->width, dsc_out->bpp);
        // Serial.printf("  box_w %u  -  box_h %u   -  ", dsc_out->box_w, dsc_out->box_h);
        // Serial.printf("  kernL %u  -  kernR %u   \n", charInfo->kerningL, charInfo->kerningR);
        // Serial.printf("  ofs_x %u - ofs_y %u \n\n", dsc_out->ofs_x, dsc_out->ofs_x);

        return NULL;
    }
    */

    // uint8_t w          = charInfo->width + charInfo->kerningL + charInfo->kerningR;
    // char data[256];
    uint16_t fileindex = 0;
    uint16_t arrindex  = 0;
    int k, len = 1; // enter while loop
    uint8_t color1, color2;

    // while((fileindex < charInfo->length) && len > 0) { //} && !feof(file)) {
    while((arrindex < size * 2) && (len > 0)) { // read untill the bitmap is full, no need for datalength
        if((int32_t)sizeof(data) < (charInfo->length - fileindex)) {
            len = file.readBytes(data, sizeof(data));
        } else {
            len = file.readBytes(data, (charInfo->length - fileindex));
        }
        fileindex += len;

        for(k = 0; k < len; k++) {
            uint8_t b = data[k];
            // Serial.printf("%d - %d > %x = %x  arrindex:%d\n", fileindex, arrindex, b, ch[0], ftell(file));

            uint8_t repeats = b & 0b00011111; /* last 5 bits indicate repetition as the same color */
            switch(b >> 5) {
                case(0b000):
                    arrindex += repeats; // repeats are white
                    break;

                case(0b001):
                    for(int i = 0; i < repeats; i++) { // repeats are black
                        blackAdd(charBitmap_p, arrindex++);
                    }
                    break;

                case(0b010):
                    arrindex += repeats; // repeats are white
                    blackAdd(charBitmap_p, arrindex++);
                    break;

                case(0b011):
                    arrindex += repeats; // repeats are white
                    blackAdd(charBitmap_p, arrindex++);
                    blackAdd(charBitmap_p, arrindex++);
                    break;

                case(0b100):
                case(0b101):
                    repeats = (uint8_t)((b & (0b111000)) >> 3); /* 3 bits indicate repetition as the same color */
                    color1  = (uint8_t)(b & (0b0111));
                    arrindex += repeats;
                    colorsAdd(charBitmap_p, color1, arrindex++);
                    break;

                default:
                    color1 = (b & 0b111000) >> 3;
                    color2 = b & 0b000111;
                    colorsAdd(charBitmap_p, color1, arrindex++);
                    colorsAdd(charBitmap_p, color2, arrindex++);
            }
            // if(unicode_letter == 0xf015)
            //     LOG_VERBOSE(TAG_FONT, F("read %d => %d / %d (%d / %d) %d"), len, fileindex, charInfo->length,
            //     arrindex,
            //                 size, k);
        }
    }

    // Serial.printf("[OK] Letter %c - %d\n", (char)(uint8_t)unicode_letter, arrindex);
    // printBuffer(charBitmap_p, charInfo->width, fdsc->CharHeight);

    file.close();

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
HASP_ATTRIBUTE_FAST_MEM bool lv_font_get_glyph_dsc_fmt_zifont(const lv_font_t* font, lv_font_glyph_dsc_t* dsc_out,
                                                              uint32_t unicode_letter, uint32_t unicode_letter_next)
{
    /* Only ascii characteres supported for now */
    // returning true with a box_h of 0 does not display an error
    dsc_out->box_w = dsc_out->box_h = 0; // Prevents glyph not found error messages when true is returned
    if(unicode_letter < 0x20) return true;
    if(unicode_letter > 0xff && unicode_letter < 0xf000) return true;
    // if(unicode_letter > 0xff) Serial.printf("Char# %u\n", unicode_letter);

    // ulong startMillis               = millis();
    lv_font_fmt_zifont_dsc_t* fdsc = (lv_font_fmt_zifont_dsc_t*)font->dsc; /* header data struct */

    uint16_t glyphID;
    File file;
    uint8_t charmap_position;
    uint8_t charwidth;
    if(unicode_letter >= 0xF000) {
        charmap_position = 25 + sizeof(zi_font_header_t);
        glyphID          = unicode_letter - 0xf000; // start of fontawesome
        charwidth        = 0;
    } else {
        charmap_position = fdsc->Startdataaddress; // Descriptionlength + sizeof(lv_font_fmt_zifont_dsc_t);
        glyphID          = unicode_letter - 0x20;  // simple unicode to ascii - space is charNum=0
        // if(glyphID < sizeof(fdsc->ascii_glyph_dsc) / sizeof(lv_zifont_char_t))
        if(glyphID < CHAR_CACHE_SIZE)
            charwidth = fdsc->ascii_glyph_dsc[glyphID].width;
        else
            charwidth = 0;
    }

    // if(charwidth == 0 || glyphID >= sizeof(fdsc->ascii_glyph_dsc) / sizeof(lv_zifont_char_t)) {
    if(charwidth == 0 || glyphID >= CHAR_CACHE_SIZE) {

        /* Open the font for reading */
        if(unicode_letter >= 0xF000) {
            // return false;
            char filename[32];
            snprintf_P(filename, sizeof(filename), PSTR("/fontawesome%u.zi"), fdsc->CharHeight);
            if(!openFont(file, filename)) return false;
        } else {
            if(!openFont(file, (char*)font->user_data)) return false;
        }

        /* read 10 bytes charmap */
        // lv_zifont_char_t * myCharIndex = (lv_zifont_char_t *)lv_mem_alloc(sizeof(lv_zifont_char_t));
        lv_zifont_char_t myCharIndex;
        uint32_t char_position = glyphID * sizeof(lv_zifont_char_t) + charmap_position;
        file.seek(char_position, SeekSet);
        size_t readSize = file.readBytes((char*)&myCharIndex, sizeof(lv_zifont_char_t));
        file.close();

        /* Check that we read the correct size */
        if(readSize != sizeof(lv_zifont_char_t)) {
            // lv_mem_free(myCharIndex);
            return false;
        }

        /* Double-check that we got the correct letter */
        if(fdsc->last_glyph_dsc->character != unicode_letter) {
            // lv_mem_free(myCharIndex);
            // return false;
        }

        if(unicode_letter <= 0xff && glyphID < CHAR_CACHE_SIZE) fdsc->ascii_glyph_dsc[glyphID] = myCharIndex;
        lastCharInfo = myCharIndex;
        // lv_mem_free(myCharIndex);

    } else {
        lastCharInfo = fdsc->ascii_glyph_dsc[glyphID];
    }

    /*cache glyph data*/
    fdsc->last_glyph_id  = glyphID;
    fdsc->last_glyph_dsc = &lastCharInfo;

    dsc_out->adv_w = lastCharInfo.width; //-myCharIndex->righroverlap)*16; /* 8 bit integer 4 bit fractional*/
    dsc_out->box_w = lastCharInfo.width + lastCharInfo.kerningL + lastCharInfo.kerningR;
    dsc_out->box_h = fdsc->CharHeight;
    dsc_out->ofs_x = -lastCharInfo.kerningL;
    dsc_out->ofs_y = 0;
    dsc_out->bpp   = 4; /**< Bit-per-pixel: 1, 2, 4, 8*/

    // Serial.printf("Letter %c\n", (char)(uint8_t)unicode_letter);

    // Serial.printf("adv_w %u (%u) - bpp %u  -  ", dsc_out->adv_w, myCharIndex->width, dsc_out->bpp);
    // Serial.printf("box_w %u  -  box_h %u   -  ", dsc_out->box_w, dsc_out->box_h);
    // Serial.printf("kernL %u  -  kernR %u   \n", myCharIndex->kerningL, myCharIndex->kerningR);
    // Serial.printf("ofs_x %u - ofs_y %u \n\n", dsc_out->ofs_x, dsc_out->ofs_x);

    //    debugPrintln("Char " + String((char)myCharIndex->character) + " lookup took " + String(millis() -
    //    startMillis) + "ms");
    return true;
}

HASP_ATTRIBUTE_FAST_MEM static void blackAdd(uint8_t* charBitmap_p, uint16_t pos)
{
    uint8_t col    = pos & 0x0001; // remainder
    uint16_t map_p = pos >> 1;     // devide by 2

    if(col == 0) {
        charBitmap_p[map_p] = 0xf0;
    } else {
        charBitmap_p[map_p] |= ColorBlack;
    }
}

HASP_ATTRIBUTE_FAST_MEM static inline void colorsAdd(uint8_t* charBitmap_p, uint8_t color1, uint16_t pos)
{
    uint32_t col   = pos & 0x0001; // remainder
    uint32_t map_p = pos >> 1;     // devide by 2

    if(col == 0) {
        charBitmap_p[map_p] = color1 << 5;
    } else {
        charBitmap_p[map_p] |= color1 << 1;
    }
}

#endif

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

    for(int i = 0; i < h; i++) {
        for(int j = 0; j < cols; j++) {
            uint8_t b = charBitmap_p[i * cols + j];
            printPixel(b >> 4);
            printPixel(b & 0b1111);
        }
        Serial.println("");
    }
    Serial.println("/end");
}
*/
