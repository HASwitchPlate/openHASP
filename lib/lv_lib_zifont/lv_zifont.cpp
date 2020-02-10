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
uint32_t charInBuffer = 0; // Last Character ID in the Bitmap Buffer
// uint8_t filecharBitmap_p[20 * 1024];
static lv_zifont_char_t lastCharInfo; // Holds the last Glyph DSC

#if ESP32
// static lv_zifont_char_t charCache[256 - 32]; // glyphID DSC cache
#define CHAR_CACHE_SIZE 224
#else
#define CHAR_CACHE_SIZE 1
// static lv_zifont_char_t charCache[256 - 32]; // glyphID DSC cache
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

bool openFont(File & file, const char * filename)
{
    file = SPIFFS.open(filename, "r");
    if(!file) {
        errorPrintln(String(F("FONT: %sOpening font: ")) + String(filename));
        return false;
    }
    return true;
}

int lv_zifont_font_init(lv_font_t * font, const char * font_path, uint16_t size)
{
    charInBuffer = 0; // invalidate any previous cache

    lv_font_fmt_zifont_dsc_t * dsc;
    if(!font->dsc) {
        dsc = (lv_font_fmt_zifont_dsc_t *)lv_mem_alloc(sizeof(lv_font_fmt_zifont_dsc_t));
        LV_ASSERT_MEM(dsc);
    } else {
        dsc = (lv_font_fmt_zifont_dsc_t *)font->dsc;
    }
    if(dsc == NULL) return ZIFONT_ERROR_OUT_OF_MEMORY;
    int error = 0;

    /* Initialize Last Glyph DSC */
    dsc->last_glyph_dsc = (lv_zifont_char_t *)lv_mem_alloc(sizeof(lv_zifont_char_t));
    if(dsc->last_glyph_dsc == NULL) return ZIFONT_ERROR_OUT_OF_MEMORY;
    dsc->last_glyph_dsc->width = 0;
    dsc->last_glyph_id         = 0;

    /* Open the font for reading */
    File file;
    if(!openFont(file, font_path)) return ZIFONT_ERROR_OPENING_FILE;

    /* Read file header as dsc */
    zi_font_header_t header;
    size_t readSize = file.readBytes((char *)&header, sizeof(zi_font_header_t));

    /* Check that we read the correct size */
    if(readSize != sizeof(zi_font_header_t)) {
        debugPrintln(PSTR("FONT: Error reading ziFont Header"));
        file.close();
        return ZIFONT_ERROR_READING_DATA;
    }

    /* Check ziFile Header Format */
    if(header.Password != 4 || header.Version != 5) {
        debugPrintln(PSTR("FONT: Unknown font file format"));
        file.close();
        return ZIFONT_ERROR_UNKNOWN_HEADER;
    }

    dsc->CharHeight       = header.CharHeight;
    dsc->CharWidth        = header.CharWidth;
    dsc->Maximumnumchars  = header.Maximumnumchars;
    dsc->Actualnumchars   = header.Actualnumchars;
    dsc->Totaldatalength  = header.Totaldatalength;
    dsc->Startdataaddress = header.Startdataaddress + header.Descriptionlength;
    dsc->Fontdataadd8byte = header.Fontdataadd8byte;

    if(!dsc->ascii_glyph_dsc) {
        dsc->ascii_glyph_dsc = (lv_zifont_char_t *)lv_mem_alloc(sizeof(lv_zifont_char_t) * CHAR_CACHE_SIZE);
        LV_ASSERT_MEM(dsc->ascii_glyph_dsc);
    }
    if(dsc->ascii_glyph_dsc == NULL) {
        file.close();
        return ZIFONT_ERROR_OUT_OF_MEMORY;
    }

    /* read charmap into cache */
    file.seek(0 * sizeof(zi_font_header_t) + dsc->Startdataaddress, SeekSet);
    //* read and fill charmap cache
    readSize = file.readBytes((char *)dsc->ascii_glyph_dsc, sizeof(lv_zifont_char_t) * CHAR_CACHE_SIZE);

    //* Check that we read the correct size
    if(readSize != sizeof(lv_zifont_char_t) * CHAR_CACHE_SIZE) {
        debugPrintln(PSTR("FONT: Error reading ziFont character map"));
        file.close();
        return ZIFONT_ERROR_READING_DATA;
    }

    char msg[128];
    sprintf_P(msg, PSTR("FONT: Loaded V%d Font File: %s containing %d characters"), header.Version, font_path,
              header.Maximumnumchars);
    debugPrintln(msg);

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

    font->get_glyph_dsc    = lv_font_get_glyph_dsc_fmt_zifont; /*Function pointer to get glyph's data*/
    font->get_glyph_bitmap = lv_font_get_bitmap_fmt_zifont;    /*Function pointer to get glyph's bitmap*/
    font->line_height      = dsc->CharHeight;                  /*The maximum line height required by the font*/
    font->base_line        = 0;                                /*Baseline measured from the bottom of the line*/
    font->dsc   = dsc; /* header data struct */ /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
    font->subpx = 0;

    if(font->user_data != (char *)font_path) {
        if(font->user_data) free(font->user_data);
        font->user_data = (char *)font_path;
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
static const uint8_t * lv_font_get_bitmap_fmt_zifont(const lv_font_t * font, uint32_t unicode_letter)
{
    lv_font_fmt_zifont_dsc_t * fdsc = (lv_font_fmt_zifont_dsc_t *)font->dsc; /* header data struct */
    int error                       = 0;
    uint32_t glyphID;
    char filename[32];

    /* Bitmap still in buffer */
    if(charInBuffer == unicode_letter && charBitmap_p) {
        // Serial.printf("CacheLetter %c\n", (char)(uint8_t)unicode_letter);
        // Serial.printf("#%c", (char)(uint8_t)unicode_letter);
        return charBitmap_p;
    }

    File file;
    uint16_t charmap_position;
    if(unicode_letter >= 0xF000) {
        sprintf_P(filename, PSTR("/fontawesome%u.zi"), fdsc->CharHeight);
        charmap_position = 25 + sizeof(zi_font_header_t);
        glyphID          = unicode_letter - 0xf000; // start of fontawesome
    } else {
        strcpy(filename, (char *)font->user_data);
        charmap_position = fdsc->Startdataaddress;
        glyphID          = unicode_letter - 0x20; // simple unicode to ascii - space is charNum=0
    }

    if(!openFont(file, filename)) return NULL;

    lv_zifont_char_t * charInfo;
    /* Check Last Glyph in chache is valid and Matches currentGlyphID */
    if(fdsc->last_glyph_id == glyphID && fdsc->last_glyph_dsc && fdsc->last_glyph_dsc->width > 0) {
        // Serial.print("@");
        charInfo = fdsc->last_glyph_dsc;
    } else {
        Serial.print("%");
        /* Read Character Table */
        charInfo               = (lv_zifont_char_t *)lv_mem_alloc(sizeof(lv_zifont_char_t));
        uint32_t char_position = glyphID * sizeof(lv_zifont_char_t) + charmap_position;
        file.seek(char_position, SeekSet);
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
            // file.close();
            // lv_mem_free(charInfo);
            // debugPrintln(PSTR("FONT: [ERROR] Incorrect letter read from flash"));
            // return NULL;
        }
    }

    long datapos = charmap_position + (charInfo->pos[2] << 16) + (charInfo->pos[1] << 8) + charInfo->pos[0];

    /* Allocate & Initialize Buffer for 4bpp */
    uint32_t size = (charInfo->width * fdsc->CharHeight + 1) / 2; // add 1 for rounding up
    if(charBitmap_p) lv_mem_free(charBitmap_p);
    charBitmap_p = (uint8_t *)lv_mem_alloc(size);
    memset(charBitmap_p, 0, size); // init the bitmap to white

    char ch[1];
    file.seek(datapos, SeekSet);
    file.readBytes(ch, 1); /* check first byte = bpp */

    if(ch[0] != 3) {
        file.close();
        lv_mem_free(charInfo);
        debugPrintln(PSTR("FONT: [ERROR] Character is not 3bpp encoded"));

        // Serial.printf("  adv_w %u (%u) - bpp %u  -  ", dsc_out->adv_w, charInfo->width, dsc_out->bpp);
        // Serial.printf("  box_w %u  -  box_h %u   -  ", dsc_out->box_w, dsc_out->box_h);
        // Serial.printf("  kernL %u  -  kernR %u   \n", charInfo->kerningL, charInfo->kerningR);
        // Serial.printf("  ofs_x %u - ofs_y %u \n\n", dsc_out->ofs_x, dsc_out->ofs_x);

        return NULL;
    }

    uint16_t arrindex  = 0;
    uint8_t w          = charInfo->width + charInfo->kerningL + charInfo->kerningR;
    uint16_t fileindex = 0;

    char data[256];
    int len = 1;

    // while((fileindex < charInfo->length) && len > 0) { //} && !feof(file)) {
    while(arrindex < size && len > 0) { // read untill the bitmap is full, no need for datalength
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
                    arrindex += repeats;
                    break;

                case(0b001):
                    for(int i = 0; i < repeats; i++) {
                        arrindex += colorsAdd(charBitmap_p, ColorBlack, w, arrindex);
                    }
                    break;

                case(0b010):
                    arrindex += repeats;
                    arrindex += colorsAdd(charBitmap_p, ColorBlack, w, arrindex);
                    break;

                case(0b011):
                    arrindex += repeats;
                    arrindex += colorsAdd(charBitmap_p, ColorBlack, w, arrindex);
                    arrindex += colorsAdd(charBitmap_p, ColorBlack, w, arrindex);
                    break;

                case(0b100):
                case(0b101): {
                    repeats       = (uint8_t)((b & (0b111000)) >> 3); /* 3 bits indicate repetition as the same color */
                    uint8_t color = (uint8_t)(b & (0b0111));
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
                    errorPrintln(PSTR("FONT: %sInvalid drawing mode"));
                    file.close();
                    lv_mem_free(charInfo);
                    return NULL;
            }
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
static bool lv_font_get_glyph_dsc_fmt_zifont(const lv_font_t * font, lv_font_glyph_dsc_t * dsc_out,
                                             uint32_t unicode_letter, uint32_t unicode_letter_next)
{
    // ulong startMillis               = millis();
    lv_font_fmt_zifont_dsc_t * fdsc = (lv_font_fmt_zifont_dsc_t *)font->dsc; /* header data struct */
    uint16_t glyphID;
    int error = 0;

    /* Only ascii characteres supported for now */
    if(unicode_letter < 0x20) return false;
    if(unicode_letter > 0xff && unicode_letter < 0xf000) return false;
    // if(unicode_letter > 0xff) Serial.printf("Char# %u\n", unicode_letter);

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
            char filename[32];
            sprintf_P(filename, PSTR("/fontawesome%u.zi"), fdsc->CharHeight);
            if(!openFont(file, filename)) return false;
        } else {
            if(!openFont(file, (char *)font->user_data)) return false;
        }

        /* read 10 bytes charmap */
        // lv_zifont_char_t * myCharIndex = (lv_zifont_char_t *)lv_mem_alloc(sizeof(lv_zifont_char_t));
        lv_zifont_char_t myCharIndex;
        uint32_t char_position = glyphID * sizeof(lv_zifont_char_t) + charmap_position;
        file.seek(char_position, SeekSet);
        size_t readSize = file.readBytes((char *)&myCharIndex, sizeof(lv_zifont_char_t));
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

    uint8_t w = lastCharInfo.width + lastCharInfo.kerningL + lastCharInfo.kerningR;

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

    //    debugPrintln("FONT: Char " + String((char)myCharIndex->character) + " lookup took " + String(millis() -
    //    startMillis) + "ms");
    return true;
}

uint16_t colorsAdd(uint8_t * charBitmap_p, uint8_t color1, uint8_t w, uint16_t pos)
{
    uint16_t map_p = pos >> 1; // devide by 2
    uint8_t col    = pos % 2;  // remainder
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
