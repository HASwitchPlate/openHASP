/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DEVICE_LANBONL8_H
#define HASP_DEVICE_LANBONL8_H

#include "device.h"

#if defined(LANBONL8)

namespace dev {

class LanbonL8 : public BaseDevice {
  public:
    void pre_setup() override;

    void post_setup() override;

    void loop() override;

    void loop_5s() override;
};
} // namespace dev

extern dev::LanbonL8 haspDevice;

#endif
#endif