/* CrowPanel Advance board support. See the project LICENSE for terms. */

#pragma once

#include <stdint.h>

bool crowpanelAdvanceBegin();
void crowpanelAdvanceBacklight(uint8_t level, bool power);
bool crowpanelAdvancePinInUse(uint8_t pin);
