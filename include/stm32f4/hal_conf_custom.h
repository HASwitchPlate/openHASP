/* Include the normal default core configuration */
#include "stm32f4xx_hal_conf_default.h"

/* Remove the default PHY address */
#ifdef LAN8742A_PHY_ADDRESS
#undef LAN8742A_PHY_ADDRESS
#endif

/* Section 2: PHY configuration section */
/* Redefine LAN8742A PHY Address*/
#ifndef LAN8742A_PHY_ADDRESS
#define LAN8742A_PHY_ADDRESS            0x01U //HASP_PHY_ADDRESS
#endif