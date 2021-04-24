/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "device.h"

#define Q(x) #x
#define QUOTE(x) Q(x)

namespace dev {

const char* BaseDevice::get_model()
{
#ifdef HASP_MODEL
    return QUOTE(HASP_MODEL);
#else
    return PIOENV;
#endif
}
} // namespace dev