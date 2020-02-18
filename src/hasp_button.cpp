#include "Button.h"

#include "hasp_button.h"
#include "hasp_dispatch.h"

Button * button[3]; // Connect your button between pin 2 and GND

void buttonSetup(void)
{
    button[0] = new Button(2);
    button[1] = new Button(3);
    button[2] = new Button(4);

    button[0]->begin();
    button[1]->begin();
    button[2]->begin();
}

void buttonLoop(void)
{
    if(button[2]->toggled()) {
        if(button[2]->read() == Button::PRESSED)
            Serial.println("Button 3 has been pressed");
        else
            Serial.println("Button 3 has been released");
    }
}
