/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DEVICE_M5STACKCORE2_H
#define HASP_DEVICE_M5STACKCORE2_H

#if defined(M5STACK)

    #include "../device.h"

namespace dev {

class M5StackCore2 : public BaseDevice {
  public:
    void pre_setup() override;
    void post_setup() override;
    void loop() override;
    void loop_5s() override;
};

} // namespace dev

using dev::M5StackCore2;
extern dev::M5StackCore2 haspDevice;

#endif
#endif