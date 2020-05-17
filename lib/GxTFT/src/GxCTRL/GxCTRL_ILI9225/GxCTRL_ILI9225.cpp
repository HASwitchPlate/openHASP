// created by Jean-Marc Zingg to be the GxCTRL_ILI9225 class for the GxTFT library
// code extracts taken from TFT_22_ILI9225: https://github.com/Nkawu/TFT_22_ILI9225
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE

#include "GxCTRL_ILI9225.h"

/* ILI9225 LCD Registers */
#define ILI9225_DRIVER_OUTPUT_CTRL      (0x01u)  // Driver Output Control
#define ILI9225_LCD_AC_DRIVING_CTRL     (0x02u)  // LCD AC Driving Control
#define ILI9225_ENTRY_MODE              (0x03u)  // Entry Mode
#define ILI9225_DISP_CTRL1              (0x07u)  // Display Control 1
#define ILI9225_BLANK_PERIOD_CTRL1      (0x08u)  // Blank Period Control
#define ILI9225_FRAME_CYCLE_CTRL        (0x0Bu)  // Frame Cycle Control
#define ILI9225_INTERFACE_CTRL          (0x0Cu)  // Interface Control
#define ILI9225_OSC_CTRL                (0x0Fu)  // Osc Control
#define ILI9225_POWER_CTRL1             (0x10u)  // Power Control 1
#define ILI9225_POWER_CTRL2             (0x11u)  // Power Control 2
#define ILI9225_POWER_CTRL3             (0x12u)  // Power Control 3
#define ILI9225_POWER_CTRL4             (0x13u)  // Power Control 4
#define ILI9225_POWER_CTRL5             (0x14u)  // Power Control 5
#define ILI9225_VCI_RECYCLING           (0x15u)  // VCI Recycling
#define ILI9225_RAM_ADDR_SET1           (0x20u)  // Horizontal GRAM Address Set
#define ILI9225_RAM_ADDR_SET2           (0x21u)  // Vertical GRAM Address Set
#define ILI9225_GRAM_DATA_REG           (0x22u)  // GRAM Data Register
#define ILI9225_GATE_SCAN_CTRL          (0x30u)  // Gate Scan Control Register
#define ILI9225_VERTICAL_SCROLL_CTRL1   (0x31u)  // Vertical Scroll Control 1 Register
#define ILI9225_VERTICAL_SCROLL_CTRL2   (0x32u)  // Vertical Scroll Control 2 Register
#define ILI9225_VERTICAL_SCROLL_CTRL3   (0x33u)  // Vertical Scroll Control 3 Register
#define ILI9225_PARTIAL_DRIVING_POS1    (0x34u)  // Partial Driving Position 1 Register
#define ILI9225_PARTIAL_DRIVING_POS2    (0x35u)  // Partial Driving Position 2 Register
#define ILI9225_HORIZONTAL_WINDOW_ADDR1 (0x36u)  // Horizontal Address Start Position
#define ILI9225_HORIZONTAL_WINDOW_ADDR2 (0x37u)  // Horizontal Address End Position
#define ILI9225_VERTICAL_WINDOW_ADDR1   (0x38u)  // Vertical Address Start Position
#define ILI9225_VERTICAL_WINDOW_ADDR2   (0x39u)  // Vertical Address End Position
#define ILI9225_GAMMA_CTRL1             (0x50u)  // Gamma Control 1
#define ILI9225_GAMMA_CTRL2             (0x51u)  // Gamma Control 2
#define ILI9225_GAMMA_CTRL3             (0x52u)  // Gamma Control 3
#define ILI9225_GAMMA_CTRL4             (0x53u)  // Gamma Control 4
#define ILI9225_GAMMA_CTRL5             (0x54u)  // Gamma Control 5
#define ILI9225_GAMMA_CTRL6             (0x55u)  // Gamma Control 6
#define ILI9225_GAMMA_CTRL7             (0x56u)  // Gamma Control 7
#define ILI9225_GAMMA_CTRL8             (0x57u)  // Gamma Control 8
#define ILI9225_GAMMA_CTRL9             (0x58u)  // Gamma Control 9
#define ILI9225_GAMMA_CTRL10            (0x59u)  // Gamma Control 10

#define ILI9225C_INVOFF  0x20
#define ILI9225C_INVON   0x21

uint32_t GxCTRL_ILI9225::readID()
{
  return readRegister(0x0, 0, 2);
}

uint32_t GxCTRL_ILI9225::readRegister(uint8_t nr, uint8_t index, uint8_t bytes)
{
  uint32_t rv = 0;
  bytes = min(bytes, 4);
  IO.startTransaction();
  IO.writeCommand(nr);
  for (uint8_t i = 0; i < index; i++)
  {
    IO.readData(); // skip
  }
  for (; bytes > 0; bytes--)
  {
    rv <<= 8;
    rv |= IO.readData();
  }
  IO.endTransaction();
  return rv;
}

uint16_t GxCTRL_ILI9225::readPixel(uint16_t x, uint16_t y)
{
  uint16_t rv;
  readRect(x, y, 1, 1, &rv);
  return rv;
}

void GxCTRL_ILI9225::readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data)
{
  uint16_t xe = x + w - 1;
  uint16_t ye = y + h - 1;
  uint32_t num = uint32_t(w) * uint32_t(h);
  for (uint16_t yy = y; yy <= ye; yy++)
  {
    for (uint16_t xx = x; xx <= xe; xx++)
    {
      IO.startTransaction();
      setWindowAddress(xx, yy, xe, ye);
      IO.writeCommand(0x22);
      IO.readData16(); // dummy
      *data++ = IO.readData16();
      IO.endTransaction();
    }
  }
}

void GxCTRL_ILI9225::init()
{
  _rotation = 0; // portrait is default
  /* Set SS bit and direction output from S528 to S1 */
  _write_cmd_data(ILI9225_POWER_CTRL1, 0x0000); // Set SAP,DSTB,STB
  _write_cmd_data(ILI9225_POWER_CTRL2, 0x0000); // Set APON,PON,AON,VCI1EN,VC
  _write_cmd_data(ILI9225_POWER_CTRL3, 0x0000); // Set BT,DC1,DC2,DC3
  _write_cmd_data(ILI9225_POWER_CTRL4, 0x0000); // Set GVDD
  _write_cmd_data(ILI9225_POWER_CTRL5, 0x0000); // Set VCOMH/VCOML voltage
  delay(40);

  // Power-on sequence
  _write_cmd_data(ILI9225_POWER_CTRL2, 0x0018); // Set APON,PON,AON,VCI1EN,VC
  _write_cmd_data(ILI9225_POWER_CTRL3, 0x6121); // Set BT,DC1,DC2,DC3
  _write_cmd_data(ILI9225_POWER_CTRL4, 0x006F); // Set GVDD   /*007F 0088 */
  _write_cmd_data(ILI9225_POWER_CTRL5, 0x495F); // Set VCOMH/VCOML voltage
  _write_cmd_data(ILI9225_POWER_CTRL1, 0x0800); // Set SAP,DSTB,STB
  delay(10);
  _write_cmd_data(ILI9225_POWER_CTRL2, 0x103B); // Set APON,PON,AON,VCI1EN,VC
  delay(50);

  _write_cmd_data(ILI9225_DRIVER_OUTPUT_CTRL, 0x011C); // set the display line number and display direction
  _write_cmd_data(ILI9225_LCD_AC_DRIVING_CTRL, 0x0100); // set 1 line inversion
  _write_cmd_data(ILI9225_ENTRY_MODE, 0x1030); // set GRAM write direction and BGR=1.
  _write_cmd_data(ILI9225_DISP_CTRL1, 0x0000); // Display off
  _write_cmd_data(ILI9225_BLANK_PERIOD_CTRL1, 0x0808); // set the back porch and front porch
  _write_cmd_data(ILI9225_FRAME_CYCLE_CTRL, 0x1100); // set the clocks number per line
  _write_cmd_data(ILI9225_INTERFACE_CTRL, 0x0000); // CPU interface
  _write_cmd_data(ILI9225_OSC_CTRL, 0x0D01); // Set Osc  /*0e01*/
  _write_cmd_data(ILI9225_VCI_RECYCLING, 0x0020); // Set VCI recycling
  _write_cmd_data(ILI9225_RAM_ADDR_SET1, 0x0000); // RAM Address
  _write_cmd_data(ILI9225_RAM_ADDR_SET2, 0x0000); // RAM Address

  /* Set GRAM area */
  _write_cmd_data(ILI9225_GATE_SCAN_CTRL, 0x0000);
  _write_cmd_data(ILI9225_VERTICAL_SCROLL_CTRL1, 0x00DB);
  _write_cmd_data(ILI9225_VERTICAL_SCROLL_CTRL2, 0x0000);
  _write_cmd_data(ILI9225_VERTICAL_SCROLL_CTRL3, 0x0000);
  _write_cmd_data(ILI9225_PARTIAL_DRIVING_POS1, 0x00DB);
  _write_cmd_data(ILI9225_PARTIAL_DRIVING_POS2, 0x0000);
  _write_cmd_data(ILI9225_HORIZONTAL_WINDOW_ADDR1, 0x00AF);
  _write_cmd_data(ILI9225_HORIZONTAL_WINDOW_ADDR2, 0x0000);
  _write_cmd_data(ILI9225_VERTICAL_WINDOW_ADDR1, 0x00DB);
  _write_cmd_data(ILI9225_VERTICAL_WINDOW_ADDR2, 0x0000);

  /* Set GAMMA curve */
  _write_cmd_data(ILI9225_GAMMA_CTRL1, 0x0000);
  _write_cmd_data(ILI9225_GAMMA_CTRL2, 0x0808);
  _write_cmd_data(ILI9225_GAMMA_CTRL3, 0x080A);
  _write_cmd_data(ILI9225_GAMMA_CTRL4, 0x000A);
  _write_cmd_data(ILI9225_GAMMA_CTRL5, 0x0A08);
  _write_cmd_data(ILI9225_GAMMA_CTRL6, 0x0808);
  _write_cmd_data(ILI9225_GAMMA_CTRL7, 0x0000);
  _write_cmd_data(ILI9225_GAMMA_CTRL8, 0x0A00);
  _write_cmd_data(ILI9225_GAMMA_CTRL9, 0x0710);
  _write_cmd_data(ILI9225_GAMMA_CTRL10, 0x0710);

  _write_cmd_data(ILI9225_DISP_CTRL1, 0x0012);
  delay(50);
  _write_cmd_data(ILI9225_DISP_CTRL1, 0x1017);
}

void GxCTRL_ILI9225::setWindowAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
#if 0
  if (x1 - x0 > 100)
  {
    Serial.print("("); Serial.print(x0); Serial.print(", "); Serial.print(y0); Serial.print(", ");
    Serial.print(x1); Serial.print(", "); Serial.print(y1); Serial.print(") -> ");
  }
  switch (_rotation)
  {
    case 1:
      {
        uint16_t tx0 = x0;
        uint16_t tx1 = x1;
        x0 = _tft_width - 1 - y1;
        x1 = _tft_width - 1 - y0;
        y0 = tx0;
        y1 = tx1;
      }
      break;
    case 2:
      {
        uint16_t tx0 = x0;
        uint16_t ty0 = y0;
        x0 = _tft_width - 1 - x1;
        x1 = _tft_width - 1 - tx0;
        y0 = _tft_height - 1 - y1;
        y1 = _tft_height - 1 - ty0;
      }
      break;
    case 3:
      {
        uint16_t tx0 = x0;
        uint16_t tx1 = x1;
        x0 = y0;
        x1 = y1;
        y0 = _tft_height - 1 - tx1;
        y1 = _tft_height - 1 - tx0;
      }
      break;
  }
  if (x1 - x0 > 100)
  {
    Serial.print("("); Serial.print(x0); Serial.print(", "); Serial.print(y0); Serial.print(", ");
    Serial.print(x1); Serial.print(", "); Serial.print(y1); Serial.println(")");
  }
  _write_cmd_data(0x0037, x0); // horizontal address start
  _write_cmd_data(0x0036, x1); // horizontal address end
  _write_cmd_data(0x0039, y0); // vertical address start
  _write_cmd_data(0x0038, y1); // vertical address end
  _write_cmd_data(0x0020, x0); // horizontal GRAM address set
  _write_cmd_data(0x0021, y1); // vertical GRAM address set
  IO.writeCommand16Transaction(0x22);
#else
  switch (_rotation)
  {
    case 0:
      _write_cmd_data(0x0037, x0); // horizontal address start
      _write_cmd_data(0x0036, x1); // horizontal address end
      _write_cmd_data(0x0039, y0); // vertical address start
      _write_cmd_data(0x0038, y1); // vertical address end
      _write_cmd_data(0x0020, x0); // horizontal GRAM address set
      _write_cmd_data(0x0021, y1); // vertical GRAM address set
      break;
    case 1:
      _write_cmd_data(0x0037, _tft_width - 1 - y1); // horizontal address start
      _write_cmd_data(0x0036, _tft_width - 1 - y0); // horizontal address end
      _write_cmd_data(0x0039, x0); // vertical address start
      _write_cmd_data(0x0038, x1); // vertical address end
      _write_cmd_data(0x20, _tft_width - 1 - y1); // horizontal GRAM address set
      _write_cmd_data(0x21, x0); // vertical GRAM address set
      break;
    case 2:
      _write_cmd_data(0x0037, _tft_width - 1 - x1); // horizontal address start
      _write_cmd_data(0x0036, _tft_width - 1 - x0); // horizontal address end
      _write_cmd_data(0x0039, _tft_height - 1 - y1); // vertical address start
      _write_cmd_data(0x0038, _tft_height - 1 - y0); // vertical address end
      _write_cmd_data(0x20, _tft_width - 1 - x1); // horizontal GRAM address set
      _write_cmd_data(0x21, _tft_height - 1 - y1); // vertical GRAM address set
      break;
    case 3:
      _write_cmd_data(0x0037, y0); // horizontal address start
      _write_cmd_data(0x0036, y1); // horizontal address end
      _write_cmd_data(0x0039, _tft_height - 1 - x1); // vertical address start
      _write_cmd_data(0x0038, _tft_height - 1 - x0); // vertical address end
      _write_cmd_data(0x20, y0); // horizontal GRAM address set
      _write_cmd_data(0x21, _tft_height - 1 - x1); // vertical GRAM address set
      break;
  }
  IO.writeCommand16Transaction(0x22);
#endif
}

void GxCTRL_ILI9225::setRotation(uint8_t r)
{
  _rotation = r & 3;
  // done by SW for now
}

void GxCTRL_ILI9225::write_wr_reg(uint16_t data)
{
  IO.writeCommand16Transaction(data);
}

void GxCTRL_ILI9225::write_wr_data(uint16_t data)
{
  IO.writeData16Transaction(data);
}

void GxCTRL_ILI9225::_write_cmd_data(uint16_t cmd , uint16_t data)
{
  IO.writeCommand16Transaction(cmd);
  IO.writeData16Transaction(data);
}

