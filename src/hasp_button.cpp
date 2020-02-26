#include "Button.h"

#include "hasp_conf.h"

#include "hasp_button.h"
#include "hasp_dispatch.h"

Button * button[HASP_NUM_INPUTS]; // Connect your button between pin 2 and GND

void buttonSetup(void)
{
    // button[0] = new Button(2);
    button[1] = new Button(3);
    button[2] = new Button(4);

    // button[0]->begin();
    button[1]->begin();
    button[2]->begin();
}

void buttonLoop(void)
{
    for(uint8_t i = 0; i < (sizeof button / sizeof *button); i++) {
        if(button[i] && button[i]->toggled()) {
            dispatchButton(i, button[i]->read() == Button::PRESSED);
        }
    }
}
