#include "FT6336U.h"

#define I2C_SDA 22
#define I2C_SCL 23
#define RST_N_PIN 21
#define INT_N_PIN 34

FT6336U ft6336u(I2C_SDA, I2C_SCL, RST_N_PIN, INT_N_PIN); 

void setup() {
    Serial.begin(115200); 

    ft6336u.begin(); 

    Serial.print("FT6336U Firmware Version: "); 
    Serial.println(ft6336u.read_firmware_id());  
    Serial.print("FT6336U Device Mode: "); 
    Serial.println(ft6336u.read_device_mode());  
}

void loop() {
    if(digitalRead(INT_N_PIN) != -1) {
        Serial.print("FT6336U TD Status: "); 
        Serial.println(ft6336u.read_td_status());  
        Serial.print("FT6336U Touch Event/ID 1: ("); 
        Serial.print(ft6336u.read_touch1_event()); Serial.print(" / "); Serial.print(ft6336u.read_touch1_id()); Serial.println(")"); 
        Serial.print("FT6336U Touch Position 1: ("); 
        Serial.print(ft6336u.read_touch1_x()); Serial.print(" , "); Serial.print(ft6336u.read_touch1_y()); Serial.println(")"); 
        Serial.print("FT6336U Touch Weight/MISC 1: ("); 
        Serial.print(ft6336u.read_touch1_weight()); Serial.print(" / "); Serial.print(ft6336u.read_touch1_misc()); Serial.println(")"); 
        Serial.print("FT6336U Touch Event/ID 2: ("); 
        Serial.print(ft6336u.read_touch2_event()); Serial.print(" / "); Serial.print(ft6336u.read_touch2_id()); Serial.println(")"); 
        Serial.print("FT6336U Touch Position 2: ("); 
        Serial.print(ft6336u.read_touch2_x()); Serial.print(" , "); Serial.print(ft6336u.read_touch2_y()); Serial.println(")"); 
        Serial.print("FT6336U Touch Weight/MISC 2: ("); 
        Serial.print(ft6336u.read_touch2_weight()); Serial.print(" / "); Serial.print(ft6336u.read_touch2_misc()); Serial.println(")"); 
    }

}

