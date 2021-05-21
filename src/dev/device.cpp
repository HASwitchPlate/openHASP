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

const char* BaseDevice::get_version()
{
    return (QUOTE(HASP_VER_MAJ) "." QUOTE(HASP_VER_MIN) "." QUOTE(HASP_VER_REV));
}

const char* BaseDevice::get_hostname()
{
    return _hostname; //.c_str();
}

void BaseDevice::set_hostname(const char* hostname)
{
    strncpy(_hostname, hostname, STR_LEN_HOSTNAME);
}

} // namespace dev