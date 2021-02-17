/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DEVICE_M5STACKCORE2_H
#define HASP_DEVICE_M5STACKCORE2_H

#if defined(M5STACK)

#include "esp32.h"

namespace dev {

class M5StackCore2 : public Esp32Device {
  public:
    void init() override;
};

} // namespace dev

using dev::M5StackCore2;
extern dev::M5StackCore2 haspDevice;

#endif
#endif