#include "device.h"

#if defined(LANBONL8)
    #warning Lanbon L8
#elif defined(M5STACK)
    #warning M5 Stack
#else
dev::BaseDevice haspDevice;
#endif