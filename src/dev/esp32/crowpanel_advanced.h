/* CrowPanel Advance device support. See the project LICENSE for terms. */

#ifndef HASP_DEVICE_CROWPANEL_ADVANCED_H
#define HASP_DEVICE_CROWPANEL_ADVANCED_H

#include "esp32.h"

#if defined(HASP_CROWPANEL_ADVANCED)

namespace dev {

class CrowPanelAdvanced : public Esp32Device {
  public:
    void set_backlight_pin(uint8_t pin) override;

    void set_backlight_level(uint8_t level) override;
    uint8_t get_backlight_level() override;
    void set_backlight_power(bool power) override;
    bool get_backlight_power() override;

    bool is_system_pin(uint8_t pin) override;

  private:
    bool probe(uint8_t address);
    bool command(uint8_t value);
    void update_backlight();

    bool _initialized        = false;
    bool _controller_present = false;
    uint8_t _backlight_level = 255;
    bool _backlight_power    = true;
};

} // namespace dev

using dev::CrowPanelAdvanced;
extern dev::CrowPanelAdvanced haspDevice;

#endif
#endif
