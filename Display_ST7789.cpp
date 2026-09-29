#include "Display_ST7789.h"
   
SPIClass LCDspi(FSPI);
void SPI_Init()
{
  LCDspi.begin(EXAMPLE_PIN_NUM_SCLK,EXAMPLE_PIN_NUM_MISO,EXAMPLE_PIN_NUM_MOSI); 
}

void LCD_WriteCommand(uint8_t Cmd)  
{ 
  LCDspi.beginTransaction(SPISettings(SPIFreq, MSBFIRST, SPI_MODE0));
  digitalWrite(EXAMPLE_PIN_NUM_LCD_CS, LOW);  
  digitalWrite(EXAMPLE_PIN_NUM_LCD_DC, LOW); 
  LCDspi.transfer(Cmd);
  digitalWrite(EXAMPLE_PIN_NUM_LCD_CS, HIGH);  
  LCDspi.endTransaction();
}
void LCD_WriteData(uint8_t Data) 
{ 
  LCDspi.beginTransaction(SPISettings(SPIFreq, MSBFIRST, SPI_MODE0));
  digitalWrite(EXAMPLE_PIN_NUM_LCD_CS, LOW);  
  digitalWrite(EXAMPLE_PIN_NUM_LCD_DC, HIGH);  
  LCDspi.transfer(Data);  
  digitalWrite(EXAMPLE_PIN_NUM_LCD_CS, HIGH);  
  LCDspi.endTransaction();
}    
void LCD_WriteData_Word(uint16_t Data)
{
  LCDspi.beginTransaction(SPISettings(SPIFreq, MSBFIRST, SPI_MODE0));
  digitalWrite(EXAMPLE_PIN_NUM_LCD_CS, LOW);  
  digitalWrite(EXAMPLE_PIN_NUM_LCD_DC, HIGH); 
  LCDspi.transfer16(Data);
  digitalWrite(EXAMPLE_PIN_NUM_LCD_CS, HIGH);  
  LCDspi.endTransaction();
}   
void LCD_WriteData_nbyte(uint8_t* SetData,uint8_t* ReadData,uint32_t Size) 
{ 
  LCDspi.beginTransaction(SPISettings(SPIFreq, MSBFIRST, SPI_MODE0));
  digitalWrite(EXAMPLE_PIN_NUM_LCD_CS, LOW);  
  digitalWrite(EXAMPLE_PIN_NUM_LCD_DC, HIGH);  
  LCDspi.transferBytes(SetData, ReadData, Size);
  digitalWrite(EXAMPLE_PIN_NUM_LCD_CS, HIGH);  
  LCDspi.endTransaction();
} 

void LCD_Reset(void)
{
  digitalWrite(EXAMPLE_PIN_NUM_LCD_CS, LOW);       
  delay(50);
  digitalWrite(EXAMPLE_PIN_NUM_LCD_RST, LOW); 
  delay(50);
  digitalWrite(EXAMPLE_PIN_NUM_LCD_RST, HIGH); 
  delay(50);
}
void LCD_Init(void)
{
  pinMode(EXAMPLE_PIN_NUM_LCD_CS, OUTPUT);
  pinMode(EXAMPLE_PIN_NUM_LCD_DC, OUTPUT);
  // Solo configuramos Reset si es un pin válido físico
  if (EXAMPLE_PIN_NUM_LCD_RST >= 0) {
    pinMode(EXAMPLE_PIN_NUM_LCD_RST, OUTPUT); 
  }
  
  SPI_Init();

  // Reset por software e inicio del ILI9341
  LCD_WriteCommand(0x01); // Software Reset
  delay(150);
  
  LCD_WriteCommand(0x28); // Display OFF

  // ------------- Secuencia de comandos Oficial ILI9341 -------------
  LCD_WriteCommand(0xCF);  
  LCD_WriteData(0x00); 
  LCD_WriteData(0x83); 
  LCD_WriteData(0X30); 

  LCD_WriteCommand(0xED);  
  LCD_WriteData(0x64); 
  LCD_WriteData(0x03); 
  LCD_WriteData(0X12); 
  LCD_WriteData(0X81); 

  LCD_WriteCommand(0xE8);  
  LCD_WriteData(0x85); 
  LCD_WriteData(0x01); 
  LCD_WriteData(0x79); 

  LCD_WriteCommand(0xCB);  
  LCD_WriteData(0x39); 
  LCD_WriteData(0x2C); 
  LCD_WriteData(0x00); 
  LCD_WriteData(0x34); 
  LCD_WriteData(0x02); 

  LCD_WriteCommand(0xF7);  
  LCD_WriteData(0x20); 

  LCD_WriteCommand(0xEA);  
  LCD_WriteData(0x00); 
  LCD_WriteData(0x00); 

  LCD_WriteCommand(0xC0);    // Power Control 1
  LCD_WriteData(0x26); 

  LCD_WriteCommand(0xC1);    // Power Control 2
  LCD_WriteData(0x11); 

  LCD_WriteCommand(0xC5);    // VCOM Control 1
  LCD_WriteData(0x35); 
  LCD_WriteData(0x3E); 

  LCD_WriteCommand(0xC7);    // VCOM Control 2
  LCD_WriteData(0xBE); 

  LCD_WriteCommand(0x36);    // Memory Access Control (Orientación)
  LCD_WriteData(0x48);       // Orientación vertical estándar 

  // =========================================================
  // ÚNICO CAMBIO: Activar la inversión de color (INVON)
  // Corrige los colores amarillos/invertidos en pantallas IPS
  // =========================================================
  LCD_WriteCommand(0x21); 

  LCD_WriteCommand(0x3A);    // Pixel Format Set
  LCD_WriteData(0x55);       // 16-bit por píxel (RGB565)

  LCD_WriteCommand(0xB1);    // Frame Rate Control
  LCD_WriteData(0x00);  
  LCD_WriteData(0x1B); 

  LCD_WriteCommand(0xF2);    // 3G Gamma Control
  LCD_WriteData(0x08); 

  LCD_WriteCommand(0x26);    // Gamma Set
  LCD_WriteData(0x01); 

  LCD_WriteCommand(0xE0);    // Positive Gamma Correction
  LCD_WriteData(0x1F); LCD_WriteData(0x1A); LCD_WriteData(0x18); LCD_WriteData(0x0A);
  LCD_WriteData(0x0F); LCD_WriteData(0x06); LCD_WriteData(0x45); LCD_WriteData(0X87);
  LCD_WriteData(0x32); LCD_WriteData(0x0A); LCD_WriteData(0x07); LCD_WriteData(0x02);
  LCD_WriteData(0x07); LCD_WriteData(0x05); LCD_WriteData(0x00);

  LCD_WriteCommand(0xE1);    // Negative Gamma Correction
  LCD_WriteData(0x00); LCD_WriteData(0x25); LCD_WriteData(0x27); LCD_WriteData(0x05);
  LCD_WriteData(0x10); LCD_WriteData(0x09); LCD_WriteData(0x3A); LCD_WriteData(0x78);
  LCD_WriteData(0x4D); LCD_WriteData(0x05); LCD_WriteData(0x18); LCD_WriteData(0x0D);
  LCD_WriteData(0x38); LCD_WriteData(0x3A); LCD_WriteData(0x1F);

  LCD_WriteCommand(0x11);    // Sleep Out
  delay(120); 

  LCD_WriteCommand(0x29);    // Display ON
  delay(20);
}

void LCD_SetCursor(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend)
{ 
  if (HORIZONTAL) {
    LCD_WriteCommand(0x2A);
    LCD_WriteData(Xstart >> 8);
    LCD_WriteData(Xstart + Offset_X);
    LCD_WriteData(Xend >> 8);
    LCD_WriteData(Xend + Offset_X);
    
    LCD_WriteCommand(0x2B);
    LCD_WriteData(Ystart >> 8);
    LCD_WriteData(Ystart + Offset_Y);
    LCD_WriteData(Yend >> 8);
    LCD_WriteData(Yend + Offset_Y);
  }
  else {
    LCD_WriteCommand(0x2A);
    LCD_WriteData(Ystart >> 8);
    LCD_WriteData(Ystart + Offset_Y);
    LCD_WriteData(Yend >> 8);
    LCD_WriteData(Yend + Offset_Y);

    LCD_WriteCommand(0x2B);
    LCD_WriteData(Xstart >> 8);
    LCD_WriteData(Xstart + Offset_X);
    LCD_WriteData(Xend >> 8);
    LCD_WriteData(Xend + Offset_X);
  }
  LCD_WriteCommand(0x2C);
}

void LCD_addWindow(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend,uint16_t* color)
{             
  uint16_t Show_Width = Xend - Xstart + 1;
  uint16_t Show_Height = Yend - Ystart + 1;
  uint32_t numBytes = Show_Width * Show_Height * sizeof(uint16_t);
  LCD_SetCursor(Xstart, Ystart, Xend, Yend);
  LCD_WriteData_nbyte((uint8_t*)color, NULL, numBytes);        
}

uint8_t LCD_Backlight = 50;
void Backlight_Init()
{
  ledcAttach(LCD_Backlight_PIN, Frequency, Resolution);   
  ledcWrite(LCD_Backlight_PIN, Dutyfactor);  
  Set_Backlight(LCD_Backlight);                  
}

void Set_Backlight(uint8_t Light)                     
{
  if(Light > Backlight_MAX || Light < 0)
    printf("Set Backlight parameters in the range of 0 to 100 \r\n");
  else{
    uint32_t Backlight = Light*10;
    if(Backlight == 1000)
      Backlight = 1024;
    ledcWrite(LCD_Backlight_PIN, Backlight);
  }
}