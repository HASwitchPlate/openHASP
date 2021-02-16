/* MIT License - Copyright (c) 2020 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#ifndef HASP_DEVICE_H
#define HASP_DEVICE_H

namespace dev {

class BaseDevice {
  public:
    virtual void pre_setup()
    {}
    virtual void post_setup()
    {}
    virtual void loop()
    {}
    virtual void loop_5s()
    {}
};

} // namespace dev

using dev::BaseDevice;

#if defined(LANBONL8)
    #warning Lanbon L8
    #include "lanbonl8.h"
#elif defined(M5STACK)
    #warning M5 Stack
    #include "m5stackcore2.h"
#else
extern dev::BaseDevice haspDevice;
#endif
#endif