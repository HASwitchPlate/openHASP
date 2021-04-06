#ifndef HASP_LANG_H
#define HASP_LANG_H

#ifndef HASP_LANGUAGE
  #include "en_US.h"
#else
  #define QUOTEME(x) QUOTEME_1(x)
  #define QUOTEME_1(x) #x
  #define INCLUDE_FILE(x) QUOTEME(x.h)
  #include INCLUDE_FILE(HASP_LANGUAGE)
#endif

//  language independent defines
#define D_PASSWORD_MASK "********"
#define D_BULLET "    * "
#define D_MANUFACTURER "openHASP"

#endif