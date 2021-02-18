/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DEVICE_LANBONL8_H
#define HASP_DEVICE_LANBONL8_H

#include "esp32.h"

#if defined(LANBONL8)

namespace dev {

class LanbonL8 : public Esp32Device {
  public:
    void init();
};

} // namespace dev

using dev::LanbonL8;
extern dev::LanbonL8 haspDevice;

#endif
#endif