/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

/* Include the normal default core configuration */
#include "stm32f4xx_hal_conf_default.h"

#if USE_BUILTIN_ETHERNET
/* Remove the default PHY address */
#ifdef LAN8742A_PHY_ADDRESS
#undef LAN8742A_PHY_ADDRESS
#endif

/* Section 2: PHY configuration section */
/* Redefine LAN8742A PHY Address*/
#ifndef LAN8742A_PHY_ADDRESS
#define LAN8742A_PHY_ADDRESS            0x01U //HASP_PHY_ADDRESS
#endif

#endif