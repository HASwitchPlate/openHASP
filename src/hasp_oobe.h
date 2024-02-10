/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#if HASP_USE_CONFIG > 0

void oobeSetAutoCalibrate(bool cal);
bool oobeSetup();
void oobeFakeSetup(const char*, const char*, uint8_t source); // for testing purposes only

#endif // HASP_USE_CONFIG
