// created by Jean-Marc Zingg to be the GxCTRL_OTM8009A_RV047 class for the GxTFT library
// code extracts taken from code and documentation from Ruijia Industry (lcd.h, lcd.c)
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// this class works with "RV047 480*854(FWVGA)" display from Ruijia Industry
// e.g. https://www.aliexpress.com/item/IPS-4-7-inch-40PIN-16M-TFT-LCD-Module-OTM8009A-Drive-IC-8-16Bit-8080-Interface/32638122740.html
// this display matches the FSMC TFT connector of the STM32F407ZG-M4 board
// e.g. https://www.aliexpress.com/item/STM32F407ZGT6-Development-Board-ARM-M4-STM32F4-cortex-M4-core-Board-Compatibility-Multiple-Extension/32795142050.html
// but needs addition of 5V to the 5V pin for backlight (NC on STM32 board).
//
// note: this display needs 16bit commands, aka "(MDDI/SPI) Address" in some OTM8009A.pdf

#include "GxCTRL_OTM8009A_RV047.h"

#define OTM8009A_MADCTL     0x3600
#define OTM8009A_MADCTL_MY  0x80
#define OTM8009A_MADCTL_MX  0x40
#define OTM8009A_MADCTL_MV  0x20
#define OTM8009A_MADCTL_ML  0x10
#define OTM8009A_MADCTL_RGB 0x00
#define OTM8009A_MADCTL_BGR 0x08

uint32_t GxCTRL_OTM8009A_RV047::readID()
{
  return readRegister(0xA1, 2, 2);
}

uint32_t GxCTRL_OTM8009A_RV047::readRegister(uint8_t nr, uint8_t index, uint8_t bytes)
{
  uint32_t rv = 0;
  bytes = gx_uint8_min(bytes, 4);
  IO.startTransaction();
  IO.writeCommand16(nr << 8);
  IO.readData(); // dummy
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

uint16_t GxCTRL_OTM8009A_RV047::readPixel(uint16_t x, uint16_t y)
{
  uint16_t rv;
  readRect(x, y, 1, 1, &rv);
  return rv;
}

#if defined(OTM8009A_RAMRD_AUTO_INCREMENT_OK) // not ok on my display

void GxCTRL_OTM8009A_RV047::readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data)
{
  uint16_t xe = x + w - 1;
  uint16_t ye = y + h - 1;
  uint32_t num = uint32_t(w) * uint32_t(h);
  IO.startTransaction();
  setWindowAddress(x, y, xe, ye);
  IO.writeCommand16(0x2E00);  // read from RAM
  IO.readData16(); // dummy
  for (; num > 0; num--)
  {
    uint16_t g = IO.readData();
    uint16_t r = IO.readData();
    uint16_t b = IO.readData();
    *data++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
  }
  IO.endTransaction();
}

#else

void GxCTRL_OTM8009A_RV047::readRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t* data)
{
  uint16_t xe = x + w - 1;
  uint16_t ye = y + h - 1;
  for (uint16_t yy = y; yy <= ye; yy++)
  {
    for (uint16_t xx = x; xx <= xe; xx++)
    {
      IO.startTransaction();
      setWindowAddress(xx, yy, xx, yy);
      IO.writeCommand16(0x2E00);  // read from RAM
      IO.readData16(); // dummy
      uint16_t g = IO.readData();
      uint16_t r = IO.readData();
      uint16_t b = IO.readData();
      *data++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
      IO.endTransaction();
    }
  }
}

#endif

void GxCTRL_OTM8009A_RV047::init()
{
  IO.writeCommand16Transaction(0xff00);  //
  IO.writeData16Transaction(0x80);
  IO.writeCommand16Transaction(0xff01);  // enable EXTC
  IO.writeData16Transaction(0x09);
  IO.writeCommand16Transaction(0xff02);  //
  IO.writeData16Transaction(0x01);

  IO.writeCommand16Transaction(0xff80);  // enable Orise mode
  IO.writeData16Transaction(0x80);
  IO.writeCommand16Transaction(0xff81); //
  IO.writeData16Transaction(0x09);

  IO.writeCommand16Transaction(0xff03);  // enable SPI+I2C cmd2 read
  IO.writeData16Transaction(0x01);

  //gamma DC
  IO.writeCommand16Transaction(0xc0b4); //1+2dot inversion
  IO.writeData16Transaction(0x10);

  IO.writeCommand16Transaction(0xC582); //REG-pump23
  IO.writeData16Transaction(0xA3);

  IO.writeCommand16Transaction(0xC590);  //Pump setting (3x=D6) (2x=96)//04/30
  IO.writeData16Transaction(0x96);      //2x

  IO.writeCommand16Transaction(0xC591);  //Pump setting(VGH/VGL)
  IO.writeData16Transaction(0x76);      //8F +15.5 / -15.5      //04/30

  IO.writeCommand16Transaction(0xD800); //GVDD=4.5V
  IO.writeData16Transaction(0x70);     //C0404 +4.8 =
  IO.writeCommand16Transaction(0xD801);  //NGVDD=4.5V
  IO.writeData16Transaction(0x75);

  //VCOMDC
  IO.writeCommand16Transaction(0xd900);  // VCOMDC=
  //IO.writeData16Transaction(0x49);     //61->5F  0620/49
  IO.writeData16Transaction(0x45);     //0730 New LC C0404

  //Gamma Setting
  //Gamma_W16P(0xE1,9,10,14,14,8,27,14,15,0,4,2,5,13,40,36,15); //2.2+ c0405 HSD
  //Gamma_W16P(0xE2,9,10,14,14,8,27,14,15,0,4,2,5,13,40,36,15); //2.2- c0405 HSD
  IO.writeCommand16Transaction(0xC181); //Frame rate 65Hz//V02
  IO.writeData16Transaction(0x66);  //8018 default = 60hz

  // RGB I/F setting VSYNC for OTM8018 0x0e

  IO.writeCommand16Transaction(0xC1a1); //external Vsync(08)  /Vsync,Hsync(0c) /Vsync,Hsync,DE(0e) //V02(0e)  / all  included clk(0f)
  IO.writeData16Transaction(0x0e);

  //-----------------------------------------------
  IO.writeCommand16Transaction(0xC489); //pre-charge Enable
  IO.writeData16Transaction(0x08);  //04/30 08->00  P-CH ON

  IO.writeCommand16Transaction(0xC0a2); //pre-charge //PULL-LOW 04/30
  IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xC0a3); //pre-charge //P-CH 04/30
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xC0a4); //pre-charge //P-CH 04/30
  IO.writeData16Transaction(0x02);

  IO.writeCommand16Transaction(0xC480); //Proch Source Output level
  IO.writeData16Transaction(0x30);
  //-------------------------------------------------

  //------For OTM8018  ---------
  IO.writeCommand16Transaction(0xC1A6); //Display On Pump
  IO.writeData16Transaction(0x01);

  IO.writeCommand16Transaction(0xC5C0); //
  IO.writeData16Transaction(0x00);

  IO.writeCommand16Transaction(0xB08B); //
  IO.writeData16Transaction(0x40);

  IO.writeCommand16Transaction(0xF5B2); //VRGH Disable
  IO.writeData16Transaction(0x15);
  IO.writeCommand16Transaction(0xF5B3); //
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xF5B4); //
  IO.writeData16Transaction(0x15);
  IO.writeCommand16Transaction(0xF5B5); //
  IO.writeData16Transaction(0x00);

  IO.writeCommand16Transaction(0xC593); //VRGH minimum,
  IO.writeData16Transaction(0x03);
  //------For OTM8018  ---------
  //-----------------------------------------------
  IO.writeCommand16Transaction(0xC481); //source bias //V02
  IO.writeData16Transaction(0x83);

  IO.writeCommand16Transaction(0xC592); //Pump45
  IO.writeData16Transaction(0x01);//(01)

  IO.writeCommand16Transaction(0xC5B1);  //DC voltage setting ;[0]GVDD output, default: 0xa8
  IO.writeData16Transaction(0xA9);

  //--------------------854x480---------------------------------
  IO.writeCommand16Transaction(0xb392); //  Enable SW_GM
  IO.writeData16Transaction(0x45);
  IO.writeCommand16Transaction(0xb390); //  SW_GM 480X854
  IO.writeData16Transaction(0x02);

  IO.writeCommand16Transaction(0xC080); //RTN
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xC081); //RTN
  IO.writeData16Transaction(0x58);
  IO.writeCommand16Transaction(0xC082); //RTN
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xC083); //RTN
  IO.writeData16Transaction(0x14);
  IO.writeCommand16Transaction(0xC084); //RTN
  IO.writeData16Transaction(0x16);
  //--------------------854x480---------------------------------
  ///============================================================================
  ///============================================================================
  //--------------------------------------------------------------------------------
  //    initial setting 2 < tcon_goa_wave >  480x854
  //--------------------------------------------------------------------------------
  // C09x : mck_shift1/mck_shift2/mck_shift3
  IO.writeCommand16Transaction(0xC090); // c091[7:0] : mck_shift1_hi[7:0]
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xC091); // c092[7:0] : mck_shift1_lo[7:0]
  IO.writeData16Transaction(0x56);    //
  IO.writeCommand16Transaction(0xC092); // c093[7:0] : mck_shift2_hi[7:0]
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xC093); // c094[7:0] : mck_shift2_lo[7:0]
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xC094); // c095[7:0] : mck_shift3_hi[7:0]
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xC095); // c096[7:0] : mck_shift3_lo[7:0]
  IO.writeData16Transaction(0x03);

  // C1Ax : hs_shift/vs_shift
  IO.writeCommand16Transaction(0xC1a6); // c1a6[7:0] : hs_shift[7:0]
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xC1a7); // c1a7[7:0] : vs_shift_hi[7:0]
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xC1a8); // c1a8[7:0] : vs_shift_lo[7:0]
  IO.writeData16Transaction(0x00);

  //--------------------------------------------------------------------------------
  //    initial setting 2 < tcon_goa_wave >
  //--------------------------------------------------------------------------------
  // CE8x : vst1, vst2, vst3, vst4
  IO.writeCommand16Transaction(0xCE80); // ce81[7:0] : vst1_shift[7:0]
  IO.writeData16Transaction(0x87);
  IO.writeCommand16Transaction(0xCE81); // ce82[7:0] : 0000,  vst1_width[3:0]
  IO.writeData16Transaction(0x03);
  IO.writeCommand16Transaction(0xCE82); // ce83[7:0] : vst1_tchop[7:0]
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCE83); // ce84[7:0] : vst2_shift[7:0]
  IO.writeData16Transaction(0x85);
  IO.writeCommand16Transaction(0xCE84); // ce85[7:0] : 0000,  vst2_width[3:0]
  IO.writeData16Transaction(0x03);
  IO.writeCommand16Transaction(0xCE85); // ce86[7:0] : vst2_tchop[7:0]
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCE86); // ce87[7:0] : vst3_shift[7:0]
  IO.writeData16Transaction(0x86);
  IO.writeCommand16Transaction(0xCE87); // ce88[7:0] : 0000,  vst3_width[3:0]
  IO.writeData16Transaction(0x03);
  IO.writeCommand16Transaction(0xCE88); // ce89[7:0] : vst3_tchop[7:0]
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCE89); // ce8a[7:0] : vst4_shift[7:0]
  IO.writeData16Transaction(0x84);
  IO.writeCommand16Transaction(0xCE8a); // ce8b[7:0] : 0000,  vst4_width[3:0]
  IO.writeData16Transaction(0x03);
  IO.writeCommand16Transaction(0xCE8b); // ce8c[7:0] : vst4_tchop[7:0]
  IO.writeData16Transaction(0x00);

  //CEAx : clka1, clka2
  IO.writeCommand16Transaction(0xCEa0); // cea1[7:0] : clka1_width[3:0], clka1_shift[11:8]
  IO.writeData16Transaction(0x38);
  IO.writeCommand16Transaction(0xCEa1); // cea2[7:0] : clka1_shift[7:0]
  IO.writeData16Transaction(0x03);
  IO.writeCommand16Transaction(0xCEa2); // cea3[7:0] : clka1_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
  IO.writeData16Transaction(0x03);
  IO.writeCommand16Transaction(0xCEa3); // cea4[7:0] : clka1_switch[7:0]
  IO.writeData16Transaction(0x58);
  IO.writeCommand16Transaction(0xCEa4); // cea5[7:0] : clka1_extend[7:0]
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCEa5); // cea6[7:0] : clka1_tchop[7:0]
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCEa6); // cea7[7:0] : clka1_tglue[7:0]
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCEa7); // cea8[7:0] : clka2_width[3:0], clka2_shift[11:8]
  IO.writeData16Transaction(0x38);
  IO.writeCommand16Transaction(0xCEa8); // cea9[7:0] : clka2_shift[7:0]
  IO.writeData16Transaction(0x02);
  IO.writeCommand16Transaction(0xCEa9); // ceaa[7:0] : clka2_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
  IO.writeData16Transaction(0x03);
  IO.writeCommand16Transaction(0xCEaa); // ceab[7:0] : clka2_switch[7:0]
  IO.writeData16Transaction(0x59);
  IO.writeCommand16Transaction(0xCEab); // ceac[7:0] : clka2_extend
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCEac); // cead[7:0] : clka2_tchop
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCEad); // ceae[7:0] : clka2_tglue
  IO.writeData16Transaction(0x00);

  //CEBx : clka3, clka4
  IO.writeCommand16Transaction(0xCEb0); // ceb1[7:0] : clka3_width[3:0], clka3_shift[11:8]
  IO.writeData16Transaction(0x38);
  IO.writeCommand16Transaction(0xCEb1); // ceb2[7:0] : clka3_shift[7:0]
  IO.writeData16Transaction(0x01);
  IO.writeCommand16Transaction(0xCEb2); // ceb3[7:0] : clka3_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
  IO.writeData16Transaction(0x03);
  IO.writeCommand16Transaction(0xCEb3); // ceb4[7:0] : clka3_switch[7:0]
  IO.writeData16Transaction(0x5a);
  IO.writeCommand16Transaction(0xCEb4); // ceb5[7:0] : clka3_extend[7:0]
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCEb5); // ceb6[7:0] : clka3_tchop[7:0]
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCEb6); // ceb7[7:0] : clka3_tglue[7:0]
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCEb7); // ceb8[7:0] : clka4_width[3:0], clka4_shift[11:8]
  IO.writeData16Transaction(0x38);
  IO.writeCommand16Transaction(0xCEb8); // ceb9[7:0] : clka4_shift[7:0]
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCEb9); // ceba[7:0] : clka4_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
  IO.writeData16Transaction(0x03);
  IO.writeCommand16Transaction(0xCEba); // cebb[7:0] : clka4_switch[7:0]
  IO.writeData16Transaction(0x5b);
  IO.writeCommand16Transaction(0xCEbb); // cebc[7:0] : clka4_extend
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCEbc); // cebd[7:0] : clka4_tchop
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCEbd); // cebe[7:0] : clka4_tglue
  IO.writeData16Transaction(0x00);

  //CECx : clkb1, clkb2
  IO.writeCommand16Transaction(0xCEc0); // cec1[7:0] : clkb1_width[3:0], clkb1_shift[11:8]
  IO.writeData16Transaction(0x30);
  IO.writeCommand16Transaction(0xCEc1); // cec2[7:0] : clkb1_shift[7:0]
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCEc2); // cec3[7:0] : clkb1_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
  IO.writeData16Transaction(0x03);
  IO.writeCommand16Transaction(0xCEc3); // cec4[7:0] : clkb1_switch[7:0]
  IO.writeData16Transaction(0x5c);
  IO.writeCommand16Transaction(0xCEc4); // cec5[7:0] : clkb1_extend[7:0]
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCEc5); // cec6[7:0] : clkb1_tchop[7:0]
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCEc6); // cec7[7:0] : clkb1_tglue[7:0]
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCEc7); // cec8[7:0] : clkb2_width[3:0], clkb2_shift[11:8]
  IO.writeData16Transaction(0x30);
  IO.writeCommand16Transaction(0xCEc8); // cec9[7:0] : clkb2_shift[7:0]
  IO.writeData16Transaction(0x01);
  IO.writeCommand16Transaction(0xCEc9); // ceca[7:0] : clkb2_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
  IO.writeData16Transaction(0x03);
  IO.writeCommand16Transaction(0xCEca); // cecb[7:0] : clkb2_switch[7:0]
  IO.writeData16Transaction(0x5d);
  IO.writeCommand16Transaction(0xCEcb); // cecc[7:0] : clkb2_extend
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCEcc); // cecd[7:0] : clkb2_tchop
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCEcd); // cece[7:0] : clkb2_tglue
  IO.writeData16Transaction(0x00);

  //CEDx : clkb3, clkb4
  IO.writeCommand16Transaction(0xCEd0); // ced1[7:0] : clkb3_width[3:0], clkb3_shift[11:8]
  IO.writeData16Transaction(0x30);
  IO.writeCommand16Transaction(0xCEd1); // ced2[7:0] : clkb3_shift[7:0]
  IO.writeData16Transaction(0x02);
  IO.writeCommand16Transaction(0xCEd2); // ced3[7:0] : clkb3_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
  IO.writeData16Transaction(0x03);
  IO.writeCommand16Transaction(0xCEd3); // ced4[7:0] : clkb3_switch[7:0]
  IO.writeData16Transaction(0x5e);
  IO.writeCommand16Transaction(0xCEd4); // ced5[7:0] : clkb3_extend[7:0]
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCEd5); // ced6[7:0] : clkb3_tchop[7:0]
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCEd6); // ced7[7:0] : clkb3_tglue[7:0]
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCEd7); // ced8[7:0] : clkb4_width[3:0], clkb4_shift[11:8]
  IO.writeData16Transaction(0x30);
  IO.writeCommand16Transaction(0xCEd8); // ced9[7:0] : clkb4_shift[7:0]
  IO.writeData16Transaction(0x03);
  IO.writeCommand16Transaction(0xCEd9); // ceda[7:0] : clkb4_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
  IO.writeData16Transaction(0x03);
  IO.writeCommand16Transaction(0xCEda); // cedb[7:0] : clkb4_switch[7:0]
  IO.writeData16Transaction(0x5f);
  IO.writeCommand16Transaction(0xCEdb); // cedc[7:0] : clkb4_extend
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCEdc); // cedd[7:0] : clkb4_tchop
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCEdd); // cede[7:0] : clkb4_tglue
  IO.writeData16Transaction(0x00);

  //CFCx :
  IO.writeCommand16Transaction(0xCFc7); // cfc8[7:0] : reg_goa_gnd_opt, reg_goa_dpgm_tail_set, reg_goa_f_gating_en, reg_goa_f_odd_gating, toggle_mod1, 2, 3, 4
  IO.writeData16Transaction(0x00);
  IO.writeCommand16Transaction(0xCFc9); // cfca[7:0] : reg_goa_gnd_period[7:0]
  IO.writeData16Transaction(0x00);

  //--------------------------------------------------------------------------------
  //    initial setting 3 < Panel setting >
  //--------------------------------------------------------------------------------
  // cbcx
  IO.writeCommand16Transaction(0xCBc4); //cbc5[7:0] : enmode H-byte of sig5  (pwrof_0, pwrof_1, norm, pwron_4 )
  IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xCBc5); //cbc6[7:0] : enmode H-byte of sig6  (pwrof_0, pwrof_1, norm, pwron_4 )
  IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xCBc6); //cbc7[7:0] : enmode H-byte of sig7  (pwrof_0, pwrof_1, norm, pwron_4 )
  IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xCBc7); //cbc8[7:0] : enmode H-byte of sig8  (pwrof_0, pwrof_1, norm, pwron_4 )
  IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xCBc8); //cbc9[7:0] : enmode H-byte of sig9  (pwrof_0, pwrof_1, norm, pwron_4 )
  IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xCBc9); //cbca[7:0] : enmode H-byte of sig10 (pwrof_0, pwrof_1, norm, pwron_4 )
  IO.writeData16Transaction(0x04);

  // cbdx
  IO.writeCommand16Transaction(0xCBd9); //cbda[7:0] : enmode H-byte of sig25 (pwrof_0, pwrof_1, norm, pwron_4 )
  IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xCBda); //cbdb[7:0] : enmode H-byte of sig26 (pwrof_0, pwrof_1, norm, pwron_4 )
  IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xCBdb); //cbdc[7:0] : enmode H-byte of sig27 (pwrof_0, pwrof_1, norm, pwron_4 )
  IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xCBdc); //cbdd[7:0] : enmode H-byte of sig28 (pwrof_0, pwrof_1, norm, pwron_4 )
  IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xCBdd); //cbde[7:0] : enmode H-byte of sig29 (pwrof_0, pwrof_1, norm, pwron_4 )
  IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xCBde); //cbdf[7:0] : enmode H-byte of sig30 (pwrof_0, pwrof_1, norm, pwron_4 )
  IO.writeData16Transaction(0x04);

  // cc8x
  IO.writeCommand16Transaction(0xCC84); //cc85[7:0] : reg setting for signal05 selection with u2d mode
  IO.writeData16Transaction(0x0C);
  IO.writeCommand16Transaction(0xCC85); //cc86[7:0] : reg setting for signal06 selection with u2d mode
  IO.writeData16Transaction(0x0A);
  IO.writeCommand16Transaction(0xCC86); //cc87[7:0] : reg setting for signal07 selection with u2d mode
  IO.writeData16Transaction(0x10);
  IO.writeCommand16Transaction(0xCC87); //cc88[7:0] : reg setting for signal08 selection with u2d mode
  IO.writeData16Transaction(0x0E);
  IO.writeCommand16Transaction(0xCC88); //cc89[7:0] : reg setting for signal09 selection with u2d mode
  IO.writeData16Transaction(0x03);
  IO.writeCommand16Transaction(0xCC89); //cc8a[7:0] : reg setting for signal10 selection with u2d mode
  IO.writeData16Transaction(0x04);

  // cc9x
  IO.writeCommand16Transaction(0xCC9e); //cc9f[7:0] : reg setting for signal25 selection with u2d mode
  IO.writeData16Transaction(0x0B);

  // ccax
  IO.writeCommand16Transaction(0xCCa0); //cca1[7:0] : reg setting for signal26 selection with u2d mode
  IO.writeData16Transaction(0x09);
  IO.writeCommand16Transaction(0xCCa1); //cca2[7:0] : reg setting for signal27 selection with u2d mode
  IO.writeData16Transaction(0x0F);
  IO.writeCommand16Transaction(0xCCa2); //cca3[7:0] : reg setting for signal28 selection with u2d mode
  IO.writeData16Transaction(0x0D);
  IO.writeCommand16Transaction(0xCCa3); //cca4[7:0] : reg setting for signal29 selection with u2d mode
  IO.writeData16Transaction(0x01);
  IO.writeCommand16Transaction(0xCCa4); //cca5[7:0] : reg setting for signal20 selection with u2d mode
  IO.writeData16Transaction(0x02);

  // ccbx
  IO.writeCommand16Transaction(0xCCb4); //ccb5[7:0] : reg setting for signal05 selection with d2u mode
  IO.writeData16Transaction(0x0D);
  IO.writeCommand16Transaction(0xCCb5); //ccb6[7:0] : reg setting for signal06 selection with d2u mode
  IO.writeData16Transaction(0x0F);
  IO.writeCommand16Transaction(0xCCb6); //ccb7[7:0] : reg setting for signal07 selection with d2u mode
  IO.writeData16Transaction(0x09);
  IO.writeCommand16Transaction(0xCCb7); //ccb8[7:0] : reg setting for signal08 selection with d2u mode
  IO.writeData16Transaction(0x0B);
  IO.writeCommand16Transaction(0xCCb8); //ccb9[7:0] : reg setting for signal09 selection with d2u mode
  IO.writeData16Transaction(0x02);
  IO.writeCommand16Transaction(0xCCb9); //ccba[7:0] : reg setting for signal10 selection with d2u mode
  IO.writeData16Transaction(0x01);

  // cccx
  IO.writeCommand16Transaction(0xCCce); //cccf[7:0] : reg setting for signal25 selection with d2u mode
  IO.writeData16Transaction(0x0E);

  // ccdx
  IO.writeCommand16Transaction(0xCCd0); //ccd1[7:0] : reg setting for signal26 selection with d2u mode
  IO.writeData16Transaction(0x10);
  IO.writeCommand16Transaction(0xCCd1); //ccd2[7:0] : reg setting for signal27 selection with d2u mode
  IO.writeData16Transaction(0x0A);
  IO.writeCommand16Transaction(0xCCd2); //ccd3[7:0] : reg setting for signal28 selection with d2u mode
  IO.writeData16Transaction(0x0C);
  IO.writeCommand16Transaction(0xCCd3); //ccd4[7:0] : reg setting for signal29 selection with d2u mode
  IO.writeData16Transaction(0x04);
  IO.writeCommand16Transaction(0xCCd4); //ccd5[7:0] : reg setting for signal30 selection with d2u mode
  IO.writeData16Transaction(0x03);

  IO.writeCommand16Transaction(0x3A00);  //  RGB 24bits D[23:0]
  IO.writeData16Transaction(0x55);

  IO.writeCommand16Transaction(0x1100); // Sleep Out
  delay(120);

  IO.writeCommand16Transaction(0x2900); // Display On
  delay(5);
}

void GxCTRL_OTM8009A_RV047::setWindowAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  IO.writeCommand16(0x2A00);
  IO.writeData16(x0 >> 8);
  IO.writeCommand16(0x2A01);
  IO.writeData16(x0 & 0x00ff);
  IO.writeCommand16(0x2A02);
  IO.writeData16(x1 >> 8);
  IO.writeCommand16(0x2A03);
  IO.writeData16(x1 & 0x00ff);
  IO.writeCommand16(0x2B00);
  IO.writeData16(y0 >> 8);
  IO.writeCommand16(0x2B01);
  IO.writeData16(y0 & 0x00ff);
  IO.writeCommand16(0x2B02);
  IO.writeData16(y1 >> 8);
  IO.writeCommand16(0x2B03);
  IO.writeData16(y1 & 0x00ff);
  IO.writeCommand16(0x2C00);
}

void GxCTRL_OTM8009A_RV047::setRotation(uint8_t r)
{
  IO.startTransaction();
  IO.writeCommand16(OTM8009A_MADCTL);
  switch (r & 3)
  {
    case 0:
      IO.writeData(0);
      break;
    case 1:
      IO.writeData(OTM8009A_MADCTL_MX | OTM8009A_MADCTL_MV);
      break;
    case 2:
      IO.writeData(OTM8009A_MADCTL_MX | OTM8009A_MADCTL_MY);
      break;
    case 3:
      IO.writeData(OTM8009A_MADCTL_MY | OTM8009A_MADCTL_MV);
      break;
  }
  IO.endTransaction();
}

