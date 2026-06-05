#include "twatch.h"

#if defined(TWATCH)

#include "Wire.h"
#include "dev/esp32/esp32.h"

/*
 * This covers the TWatch 2019 and 2020 PMIC. Only the init function is implemented
 * because the TWatch 2020 v1 variant has not just the backlight connected to LDO2, 
 * but also the power for the display controller. This shouldn't get regulated down
 * or be switched off (since the latter would require display being re-inited) so we
 * don't let the user switch it off. 
 * 
 * This *could* get fixed, by detecting hardware type and changing behaviour, but
 * this device isn't particularly common and OpenHASP probably isn't the proper tool
 * for this hardware. It's nice to have some support though.
*/
namespace dev {
    void TWatch::init(void) {
        Wire.begin();
        
        Wire.beginTransmission(53);
        Wire.write(0x12);
        Wire.write(0b00000110); // Enable LDO2, DC-DC3, disable others
        Wire.endTransmission();

        // Section set LDO2 to 3.3V
        uint8_t ldo_ctrl = ~0;
        Wire.beginTransmission(53);
        Wire.write(0x28);
        Wire.write(ldo_ctrl);
        Wire.endTransmission();
    }
}

dev::TWatch haspDevice;

#endif