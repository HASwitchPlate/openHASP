/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)

Contributors:
 [ciniml](https://github.com/ciniml)
 [mongonta0716](https://github.com/mongonta0716)
 [tobozo](https://github.com/tobozo)
/----------------------------------------------------------------------------*/
#pragma once

#if defined(ARDUINO) && defined(LGFX_USE_V1)

#include "LovyanGFX.hpp"

namespace lgfx {
inline namespace v1 {
struct Panel_ILI9481_b : public Panel_ILI948x
{
  protected:
    static constexpr uint8_t CMD_PNLDRV = 0xC0;
    static constexpr uint8_t CMD_FRMCTR = 0xC5;
    static constexpr uint8_t CMD_IFCTR  = 0xC6;
    static constexpr uint8_t CMD_PWSET  = 0xD0;
    static constexpr uint8_t CMD_VMCTR  = 0xD1;
    static constexpr uint8_t CMD_PWSETN = 0xD2;
    static constexpr uint8_t CMD_GMCTR  = 0xC8;

    const uint8_t* getInitCommands(uint8_t listno) const override
    {
        static constexpr uint8_t list0[] = {
            CMD_SLPOUT, 0 + CMD_INIT_DELAY, 120, // Exit sleep mode
            CMD_PWSET, 3, 0x07, 0x42, 0x18, CMD_VMCTR, 3, 0x00, 0x07, 0x10, CMD_PWSETN, 2, 0x01, 0x02, CMD_PNLDRV, 5,
            0x10, 0x3B, 0x00, 0x02, 0x11, CMD_FRMCTR, 1, 0x03,
            // CMD_GMCTR  ,12, 0x00, 0x44, 0x06, 0x44, 0x0A, 0x08,
            //                 0x17, 0x33, 0x77, 0x44, 0x08, 0x0C,
            CMD_GMCTR, 12, 0x00, 0x32, 0x36, 0x45, 0x06, 0x16, 0x37, 0x75, 0x77, 0x54, 0x0C, 0x00, CMD_SLPOUT,
            0 + CMD_INIT_DELAY, 120,                                        // Exit sleep mode
            CMD_IDMOFF, 0, CMD_DISPON, 0 + CMD_INIT_DELAY, 100, 0xFF, 0xFF, // end
        };
        switch(listno) {
            case 0:
                return list0;
            default:
                return nullptr;
        }
    }

    uint8_t getMadCtl(uint8_t r) const override
    {
        static constexpr uint8_t madctl_table[] = {
            MAD_HF, MAD_MV, MAD_VF, MAD_MV | MAD_HF | MAD_VF, MAD_HF | MAD_VF, MAD_MV | MAD_HF, 0, MAD_MV | MAD_VF,
        };
        return madctl_table[r];
    }
};

} // namespace v1
} // namespace lgfx

#endif