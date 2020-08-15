#include <SPI.h>
#include "myTFT.h"

MyTFT::MyTFT(uint8_t CS, uint8_t RS, uint8_t RST) 
  : TFT(CS, RS, RST)
{
  _width = ST7735_TFTHEIGHT;
  _height = ST7735_TFTWIDTH;
}

//Перекодировка текста из UTF8 в cp1251
void MyTFT::utf8rus(char *source, char *dest)
{
  int i,j,k;
  unsigned char n;
  char m[2] = { '0', '\0' };

  strcpy(dest, ""); k = strlen(source); i = j = 0;

  while (i < k) {
    n = source[i]; i++;

    if (n >= 0xC0) {
      switch (n) {
        case 0xD0: {
          n = source[i]; i++;
          if (n == 0x81) { n = 0xA8; break; }
          if (n >= 0x90 && n <= 0xBF) n = n + 0x30;
          break;
        }
        case 0xD1: {
          n = source[i]; i++;
          if (n == 0x91) { n = 0xB8; break; }
          if (n >= 0x80 && n <= 0x8F) n = n + 0x70;
          break;
        }
      }
    }

    m[0] = n; strcat(dest, m);
    j++; if (j >= 26) break; // ширина дисплея 160 точек, минимальная ширина одного символа 6 точек, итого можно отобразить 26 символов
  }
  
}

// draw a character from Adafruit_GFX
void MyTFT::drawChar(int16_t x, int16_t y, unsigned char c,
          uint16_t color, uint16_t bg, uint8_t size) 
{
  if((x >= _width)    || // Clip right
     (y >= _height)   || // Clip bottom
     ((x + 6 * size - 1) < 0) || // Clip left
     ((y + 8 * size - 1) < 0))     // Clip top
    return;

  for (int8_t i=0; i<6; i++ ) {
    uint8_t line;
    if (i == 5) 
      line = 0x0;
    else 
      line = pgm_read_byte(font+(c*5)+i);
    for (int8_t j = 0; j<8; j++) {
      if (line & 0x1) {
        if (size == 1) // default size
          drawPixel(x+i, y+j, color);
        else {  // big size
          fillRect(x+(i*size), y+(j*size), size, size, color);
        } 
      } else if (bg != color) {
        if (size == 1) // default size
          drawPixel(x+i, y+j, bg);
        else {  // big size
          fillRect(x+i*size, y+j*size, size, size, bg);
        }   
      }
      line >>= 1;
    }
  }
}

// redraw a character, based on drawChar from Adafruit_GFX
void MyTFT::reDrawChar(int16_t x, int16_t y, unsigned char c1,unsigned char c2,
          uint16_t color, uint16_t bg, uint8_t size) 
{
  if((x >= _width)    || // Clip right
     (y >= _height)   || // Clip bottom
     ((x + 6 * size - 1) < 0) || // Clip left
     ((y + 8 * size - 1) < 0))     // Clip top
    return;

  for (int8_t i=0; i<6; i++ ) {
    uint8_t line1;
    uint8_t line2;
    if (i == 5) {
      line1 = 0x0;
      line2 = 0x0;
    } else {
      line1 = pgm_read_byte(font+(c1*5)+i);
      line2 = pgm_read_byte(font+(c2*5)+i);
    }
    for (int8_t j = 0; j<8; j++) {
      if ((line1 & 0x1)!=(line2 & 0x1)) // пиксель изменился
      {
        if (line1 & 0x1) { // если была точка, то закрасим ее
          if (size == 1) // default size
            drawPixel(x+i, y+j, bg);
          else {  // big size
            fillRect(x+(i*size), y+(j*size), size, size, bg);
          } 
        } else {
          if (size == 1) // default size
            drawPixel(x+i, y+j, color);
          else {  // big size
            fillRect(x+(i*size), y+(j*size), size, size, color);
          } 
        }
      }
      line1 >>= 1;
      line2 >>= 1;
    }
  }
}

void MyTFT::drawText(int16_t x, int16_t y, char *s, char *s_prev, uint16_t color, uint16_t bg, uint8_t size)
{
  char i=0;
  char j=0;
  char c1=0;
  char c2=0;

  char msg[64];

  while(s[i] && i<27) {
    if (s_prev[j]) {c2=s_prev[j++];} else {c2=0;}
    if (s[i]!=c2) {
       reDrawChar(x+i*6*size, y, c2,s[i],color, bg, size);
    }
    i++;
  }

  while(s_prev[j] && j<27) {
    reDrawChar(x+j*6*size, y, s_prev[j],' ',color, bg, size);
    j++;
  }
}

void MyTFT::drawUTF8Text(int16_t x, int16_t y, char *s, char *s_prev, uint16_t color, uint16_t bg, uint8_t size)
{
  char buf1[32];
  char buf2[32];
  utf8rus(&s[0],&buf1[0]);
  utf8rus(&s_prev[0],&buf2[0]);
  drawText(x,y,&buf1[0],&buf2[0],color,bg,size);
}
