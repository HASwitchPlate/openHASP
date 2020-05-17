void printhex(uint8_t val)
{
  if (val < 0x10) tft.print("0");
  tft.print(val, HEX);
  if (val < 0x10) Serial.print("0");
  Serial.print(val, HEX);
}

void reportRegister(uint16_t reg, uint8_t n, const char *msg)
{
  uint8_t n_max = 20;
  n = min(n, n_max);
  uint8_t val8[n_max];
#if 1
  for (uint8_t i = 0; i < n; i++)
  {
    val8[i] = controller.readRegister(reg, i);
  }
#else
  io.startTransaction();
  io.writeCommand(reg);
  for (uint8_t i = 0; i < n; i++)
  {
    val8[i] = io.readData();
  }
  io.endTransaction();
#endif
  tft.print("reg(0x");
  Serial.print("reg(0x");
  printhex(reg >> 8);
  printhex(reg);
  tft.print(")");
  Serial.print(")");
  for (uint8_t i = 0; i < n; i++)
  {
    tft.print(" ");
    Serial.print(" ");
    printhex(val8[i]);
  }
  tft.print("\t");
  tft.println(msg);
  Serial.print("\t");
  Serial.println(msg);
}

void GxReportRegisters()
{
  tft.setTextSize((tft.width() > 320) ? 2 : 1);
  if (controller.ID == 0x1963)
  {
    reportRegister(0x0A, 1, "Get Power Mode");
    reportRegister(0x0B, 1, "Get Address Mode");
    reportRegister(0x0C, 1, "Get Pixel Format");
    reportRegister(0x0D, 1, "Get Display Mode");
    reportRegister(0x0E, 1, "Get Signal Mode");
    reportRegister(0xB1, 7, "Get LCD Mode");
    reportRegister(0xBF, 7, "Get PWM Configuration");
    reportRegister(0xF1, 1, "Get Pixel Data Interface");
  }
  else if (controller.ID == 0x8875)
  {
    reportRegister(0x0, 1, "ID, 75");
    reportRegister(0x14, 1, "RA8875_HDWR, 63");
    reportRegister(0x19, 1, "RA8875_VDHR, DF");
    reportRegister(0x1A, 1, "RA8875_VDHR1, 01");
    reportRegister(0x19, 2, "RA8875_VDHR, DF 01");
    uint32_t rv1 = controller.readRegister(0x19, 0, 1);
    tft.print("readRegister(0x19, 0, 1) : 0x"); tft.println(rv1, HEX);
    uint32_t rv2 = controller.readRegister(0x19, 0, 2);
    tft.print("readRegister(0x19, 0, 2) : 0x"); tft.println(rv2, HEX);
  }
  else if (controller.ID == 0x9806)
  {
    reportRegister(0xD3, 4, "Device Code ILI9806");
    reportRegister(0x04, 4, "Manufacturer ID");
    reportRegister(0x09, 5, "Status Register");
    reportRegister(0x0A, 2, "Power Mode");
    reportRegister(0x0B, 2, "MADCTL");
    reportRegister(0x0C, 2, "Pixel Format");
    reportRegister(0x54, 2, "Display CTRL");
    reportRegister(0x54, 2, "Display CABC");
  }
  else if (controller.ID == 0x8009A)
  {
    reportRegister(0x0A, 1, "Read Power Mode");
    reportRegister(0x0B, 1, "Read MADCTL");
    reportRegister(0x0C, 1, "Read Pixel Format");
    reportRegister(0x0D, 1, "Read Image Mode");
    reportRegister(0x45, 2, "Read Scan Line");
    reportRegister(0x52, 1, "Read Display Brightness Value");
    reportRegister(0x54, 1, "Read Display CTRL");
    reportRegister(0x56, 1, "Read Content Adaptive Brighness Control");
    reportRegister(0xA1, 5, "Read DDB Start");
    reportRegister(0xDB, 1, "Read ID2");
    reportRegister(0xDC, 1, "Read ID3");
  }
  else
  {
    reportRegister(0x00, 2, "ID: ILI9320, ILI9325, ILI9335, ...");
    reportRegister(0x04, 4, "Manufacturer ID");
    reportRegister(0x09, 5, "Status Register");
    reportRegister(0xBF, 6, "ILI9481, HX8357-B");
    reportRegister(0xD3, 4, "ILI9341, ILI9488");
    reportRegister(0xC8, 13, "GAMMA");
    reportRegister(0xE0, 16, "GAMMA-P");
    reportRegister(0xE1, 16, "GAMMA-N");
    reportRegister(0x0A, 2, "Get Power Mode");
    reportRegister(0x0C, 2, "Get Pixel Format");
    reportRegister(0x61, 2, "RDID1 HX8347-G");
    reportRegister(0x62, 2, "RDID2 HX8347-G");
    reportRegister(0x63, 2, "RDID3 HX8347-G");
    reportRegister(0x64, 2, "RDID1 HX8347-A");
    reportRegister(0x65, 2, "RDID2 HX8347-A");
    reportRegister(0x66, 2, "RDID3 HX8347-A");
    reportRegister(0x67, 2, "RDID Himax HX8347-A");
    reportRegister(0x70, 2, "Panel Himax HX8347-A");
    reportRegister(0xA1, 5, "RD_DDB SSD1963");
    reportRegister(0xB0, 2, "RGB Interface Signal Control");
    reportRegister(0xB4, 2, "Inversion Control");
    reportRegister(0xB6, 5, "Display Control");
    reportRegister(0xB7, 2, "Entry Mode Set");
    reportRegister(0xBF, 6, "ILI9481, HX8357-B");
    reportRegister(0xC0, 9, "Panel Control");
    reportRegister(0xC8, 13, "GAMMA");
    reportRegister(0xCC, 2, "Panel Control");
    reportRegister(0xD0, 3, "Power Control");
    reportRegister(0xD2, 5, "NVM Read");
    reportRegister(0xD3, 4, "ILI9341, ILI9488");
    reportRegister(0xDA, 2, "RDID1");
    reportRegister(0xDB, 2, "RDID2");
    reportRegister(0xDC, 2, "RDID3");
    reportRegister(0xE0, 16, "GAMMA-P");
    reportRegister(0xE1, 16, "GAMMA-N");
    reportRegister(0xEF, 6, "ILI9327");
    reportRegister(0xF2, 12, "Adjust Control 2");
    reportRegister(0xF6, 4, "Interface Control");
  }
}

void GxTestReadGRAM()
{
  const uint16_t testrect_x = 40;
  const uint16_t testrect_y = 20;
  const uint16_t testrect_w = 2;
  const uint16_t testrect_h = 2;
  tft.fillScreen(BLACK);
  tft.fillRect(testrect_x, testrect_y, testrect_w, testrect_h, RED);
  tft.fillRect(testrect_x + testrect_w, testrect_y, testrect_w, testrect_h, GREEN);
  tft.fillRect(testrect_x, testrect_y + testrect_h, testrect_w, testrect_h, BLUE);
  tft.fillRect(testrect_x + testrect_w, testrect_y + testrect_h, testrect_w, testrect_h, WHITE);
#if 1
  uint16_t buffer[4 * testrect_w * testrect_h];
  for (uint16_t i = 0; i < 4 * testrect_w * testrect_h; i++) buffer[i] = 0;
  controller.readRect(testrect_x, testrect_y, 2 * testrect_w, 2 * testrect_h, buffer);
  //controller.readRect(testrect_x, testrect_y, testrect_w, testrect_h, buffer);
  //controller.readRect(testrect_x + testrect_w, testrect_y, testrect_w, testrect_h, buffer);
  //controller.readRect(testrect_x, testrect_y, 1, 2, buffer);
  tft.setCursor(0, testrect_y + 20);
  tft.setTextColor(WHITE);
  for (uint16_t y = 0; y < 2 * testrect_h; y++)
  {
    for (uint16_t x = 0; x < 2 * testrect_w; x++)
    {
      Serial.print(" 0x"); Serial.print(buffer[x + y * 2 * testrect_h], HEX);
      tft.print(" 0x"); tft.print(buffer[x + y * 2 * testrect_h], HEX);
    }
    Serial.println();
    tft.println();
  }
#else
  const uint16_t parts = 3;
  uint16_t buffer[4 * testrect_w * testrect_h * parts];
  for (uint16_t i = 0; i < 4 * testrect_w * testrect_h* parts; i++) buffer[i] = 0;
  controller.readRect(testrect_x, testrect_y, 2 * testrect_w, 2 * testrect_h, buffer);
  //controller.readRect(testrect_x, testrect_y, 2 * testrect_w, testrect_h, buffer);
  tft.setCursor(0, testrect_y + 20);
  tft.setTextColor(WHITE);
  for (uint16_t y = 0; y < 2 * testrect_h; y++)
  {
    for (uint16_t x = 0; x < 2 * testrect_w; x++)
    {
      for (uint16_t k = 0; k < parts; k++)
      {
        Serial.print(" 0x"); Serial.print(buffer[(x + y * 2 * testrect_h) * parts + k], HEX);
        tft.print(" 0x"); tft.print(buffer[(x + y * 2 * testrect_h) * parts + k], HEX);
      }
      //tft.print(" ");
      tft.println();
    }
    Serial.println();
    tft.println();
  }
#endif
}

