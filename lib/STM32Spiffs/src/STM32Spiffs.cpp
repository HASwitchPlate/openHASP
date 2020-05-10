/*
 FS.cpp - file system wrapper
 Copyright (c) 2015 Ivan Grokhotkov. All rights reserved.
 This file is part of the esp8266 core for Arduino environment.
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <Arduino.h>
#include "spiffs.h"
#include "STM32Spiffs.h"

namespace SpiffsLib
{

File::File(spiffs_file f, spiffs *fs)
{
    _file = f;
    _fs = fs;
}

File::File(void)
{
    _file = 0;
    _fs = nullptr;
}

int File::read()
{
    u8_t ch;
    SPIFFS_read(_fs, _file, (u8_t *)&ch, 1);
    return ch;
}
int File::peek() { return 0; }
int File::availableForWrite() { return 0; }
int File::available() { return 0; }
int File::read(void *buf, uint16_t nbyte)
{
    return SPIFFS_read(_fs, _file, (u8_t *)buf, nbyte);
}

bool File::seek(uint32_t pos)
{
    return SPIFFS_lseek(_fs, _file, pos, SPIFFS_SEEK_SET) >= 0;
}

uint32_t File::position()
{
    return SPIFFS_tell(_fs, _file);
}

void File::flush()
{
    SPIFFS_fflush(_fs, _file);
}

size_t File::write(uint8_t ch)
{
    int res = SPIFFS_write(_fs, _file, (u8_t *)&ch, 1);
    return res;
}
size_t File::write(const uint8_t *buf, size_t size)
{
    int res = SPIFFS_write(_fs, _file, (u8_t *)buf, size);
    return res;
}

void File::close()
{
    SPIFFS_close(_fs, _file);
}

File::~File(void)
{
    Serial.print("File destructor called!");
}

SPIFlash *SpiffsClass::flash;
spiffs SpiffsClass::my_fs;
u8_t SpiffsClass::spiffs_work_buf[LOG_PAGE_SIZE * 2];
u8_t SpiffsClass::spiffs_fds[32 * 4];
u8_t SpiffsClass::spiffs_cache_buf[(LOG_PAGE_SIZE + 32) * 4];

s32_t SpiffsClass::my_spiffs_read(u32_t addr, u32_t size, u8_t *dst)
{
    flash->readByteArray(addr, dst, size);
    // Serial.print("Reading ");
    // Serial.println(*dst);
    return SPIFFS_OK;
}

s32_t SpiffsClass::my_spiffs_write(u32_t addr, u32_t size, u8_t *src)
{
    flash->writeByteArray(addr, src, size);
    Serial.print("Writing ");
    Serial.println(size);
    return SPIFFS_OK;
}

s32_t SpiffsClass::my_spiffs_erase(u32_t addr, u32_t size)
{
    flash->eraseSection(addr, size);
    Serial.print("Erasing ");
    Serial.print(addr, HEX);
    Serial.print(" - ");
    Serial.println(size);
    return SPIFFS_OK;
}

int SpiffsClass::mount()
{
    spiffs_config cfg;
#if SPIFFS_SINGLETON == 0
    cfg.phys_size = flash->GetCapacity(); // use all spi flash
    cfg.phys_addr = 0;                    // start spiffs at start of spi flash
    cfg.phys_erase_block = 65536;         // according to datasheet
    cfg.log_block_size = 65536;           // let us not complicate things
    cfg.log_page_size = LOG_PAGE_SIZE;    // as we said
#endif

    cfg.hal_read_f = my_spiffs_read;
    cfg.hal_write_f = my_spiffs_write;
    cfg.hal_erase_f = my_spiffs_erase;

    int res = SPIFFS_mount(&my_fs,
                           &cfg,
                           spiffs_work_buf,
                           spiffs_fds,
                           sizeof(spiffs_fds),
                           spiffs_cache_buf,
                           sizeof(spiffs_cache_buf),
                           0);
    Serial.print("mount result: ");
    Serial.println(res);

    return res;
}

bool SpiffsClass::begin(uint8_t cs)
{
    if (!flash)
    {
        flash = new SPIFlash(cs);
    }
    flash->begin();

    Serial.println(F("Mount Spiffs"));
    int res = mount();
    Serial.println(res);

    if (res == SPIFFS_ERR_NOT_A_FS)
    {
        Serial.println(F("**************** Spiffs Format ******************"));
        res = SPIFFS_format(&my_fs);
        Serial.println(res);

        Serial.println(F("Mount Spiffs again"));
        res = mount();
        Serial.println(res);
    }
    return res == SPIFFS_OK;
}

File SpiffsClass::open(const char *filename, uint8_t mode)
{
    spiffs_file fh = SPIFFS_open(&my_fs, filename, mode, 0);
    File f(fh, &my_fs);
    return f;
};

bool SpiffsClass::exists(const char *filepath)
{
    spiffs_DIR d;
    struct spiffs_dirent e;
    struct spiffs_dirent *pe = &e;
    int res;

    spiffs_file fd = -1;

    SPIFFS_opendir(&my_fs, "/", &d);
    while ((pe = SPIFFS_readdir(&d, pe)))
    {
        if (0 == strncmp(filepath, (char *)pe->name, strlen(filepath)))
        {
            return true;
        }
    }
    SPIFFS_closedir(&d);

    return false;
}

bool SpiffsClass::rename(const char *oldPath, const char *newPath)
{
    return SPIFFS_rename(&my_fs, oldPath, newPath) == SPIFFS_OK;
}

} // namespace SpiffsLib