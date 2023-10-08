#include "FT6336U.h"
#include <stdio.h>

#define I2C_SDA 22
#define I2C_SCL 23
#define RST_N_PIN 21
#define INT_N_PIN 34

FT6336U ft6336u(I2C_SDA, I2C_SCL, RST_N_PIN, INT_N_PIN); 

void setup() {
    Serial.begin(115200); 

    ft6336u.begin(); 
    
//    ft6336u.write_device_mode(factory_mode); 
    Serial.print("FT6336U Device Mode: "); 
    Serial.println(ft6336u.read_device_mode());  
    Serial.print("FT6336U Threshold: 0x"); 
    Serial.println(ft6336u.read_touch_threshold(), HEX); 
    Serial.print("FT6336U Filter Coefficient: 0x"); 
    Serial.println(ft6336u.read_filter_coefficient(), HEX); 
    Serial.print("FT6336U Control Mode: 0x"); 
    Serial.println(ft6336u.read_ctrl_mode(), HEX); 
    Serial.print("FT6336U Time Period for enter to Monitor Mode: 0x"); 
    Serial.println(ft6336u.read_time_period_enter_monitor(), HEX); 
    Serial.print("FT6336U Active Rate: 0x"); 
    Serial.println(ft6336u.read_active_rate(), HEX); 
    Serial.print("FT6336U Monitor Rate: 0x"); 
    Serial.println(ft6336u.read_monitor_rate(), HEX); 
    
    Serial.print("FT6336U LIB Ver: 0x"); 
    Serial.println(ft6336u.read_library_version(), HEX); 
    Serial.print("FT6336U Chip ID: 0x"); 
    Serial.println(ft6336u.read_chip_id(), HEX); 
    Serial.print("FT6336U G Mode: 0x"); 
    Serial.println(ft6336u.read_g_mode(), HEX); 
    Serial.print("FT6336U POWER Mode: 0x"); 
    Serial.println(ft6336u.read_pwrmode(), HEX); 
    Serial.print("FT6336U Firm ID: 0x"); 
    Serial.println(ft6336u.read_firmware_id(), HEX); 
    Serial.print("FT6336U Focal Tehc ID: 0x"); 
    Serial.println(ft6336u.read_focaltech_id(), HEX); 
    Serial.print("FT6336U Release Code ID: 0x"); 
    Serial.println(ft6336u.read_release_code_id(), HEX); 
    Serial.print("FT6336U State: 0x"); 
    Serial.println(ft6336u.read_state(), HEX); 

}

FT6336U_TouchPointType tp; 
void loop() {
    tp = ft6336u.scan(); 
    char tempString[128]; 
    sprintf(tempString, "FT6336U TD Count %d / TD1 (%d, %4d, %4d) / TD2 (%d, %4d, %4d)\r", tp.touch_count, tp.tp[0].status, tp.tp[0].x, tp.tp[0].y, tp.tp[1].status, tp.tp[1].x, tp.tp[1].y); 
    Serial.print(tempString); 
}
