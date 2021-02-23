/* MIT License - Copyright (c) 2019-2021 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include "device.h"

#if defined(LANBONL8)
#warning Lanbon L8
#elif defined(M5STACK)
#warning M5 Stack
#else
#warning Generic Device
#endif