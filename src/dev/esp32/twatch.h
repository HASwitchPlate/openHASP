#ifndef HASP_DEVICE_TWATCH
#define HASP_DEVICE_TWATCH

#if defined(TWATCH)

#include "esp32.h"

namespace dev {
    class TWatch : public Esp32Device {
      public:
        void init() override;
    };
    
}

using dev::TWatch;
extern dev::TWatch haspDevice;

#endif
#endif