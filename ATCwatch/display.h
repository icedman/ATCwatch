/*
 * Copyright (c) 2020 Aaron Christophel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "Arduino.h"
#include "pinout.h"
#include "font.h"
#include "asteroidsfont.h"

// #include "font16.h"

#define ST77XX_SLPIN 0x10
#define ST77XX_SLPOUT 0x11
#define ST77XX_DISPOFF 0x28
#define ST77XX_DISPON 0x29

/*
  This structure is used for positions
 */
typedef struct {
    uint8_t x, y;
} coord;

typedef struct {
    uint8_t width;
    uint8_t height;
    uint8_t pitch;
    uint8_t *pixels;
} framebuffer;

typedef long fixed;


#define LCD_BUFFER_SIZE 15000 

#define FPMUL(x,y)      ((((x)>>6)*((y)>>6))>>4)    // multiply fixed by fixed. returns fixed
#define FPMULH(x,y)     ((((x)>>2)*((y)>>2))>>12)   // higher precision
#define FPDIV(x,y)      ((((x)<<6)/((y)>>6))<<4)    // divide fixed by fixed. returns fixed
#define FPONE           65536
#define FPP             16
#define FPI(x)          ((x)<<FPP)                  // convert int to fixed
#define FPFL(x)         ((int)(x * FPONE))          // convert float to fixed
#define FPABS(n)        (n - ((n+n) & (n>>31)))
#define FEPSILON        1                           // near zero fixed

#define COLOUR_WHITE    0b1111111111111111
#define COLOUR_RED      0b1111100000000000
#define COLOUR_GREEN    0b0000011111100000
#define COLOUR_BLUE     0b0000000000011111
#define COLOUR_YELLOW   0b1111111111100000
#define COLOUR_ORANGE   0b1111101111100000
#define COLOUR_CYAN     0b0000011111111111
#define COLOUR_MAGENTA  0b1111100000011111
#define COLOUR_BLACK    0b0000000000000000

#define STR_WIDTH(str, size) ((sizeof(str) - 1) * size * FONT_WIDTH + (sizeof(str) - 2) * size)  //Macro to get the display width of a string literal
#define NCHAR_WIDTH(numChars, size) (numChars * size * FONT_WIDTH + (numChars - 1) * size)       //Macro to get the display width of n characters

void init_display();
void display_enable(bool state);
void inc_tick();

void setAddrWindowDisplay(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
void initDisplay();
void spiCommand(uint8_t d);
void startWrite_display(void);
void endWrite_display(void);

void drawFilledRect(coord pos, uint32_t w, uint32_t h, uint16_t colour);
void setDisplayWriteRegion(coord pos, uint32_t w, uint32_t h);
void clearDisplay(bool leaveAppDrawer = false);
void drawChar(coord pos, uint8_t pixelsPerPixel, char character, uint16_t colourFG, uint16_t colourBG);
void drawCharPixelToBuffer(coord charPos, uint8_t pixelsPerPixel, bool pixelInCharHere, uint16_t colourFG, uint16_t colourBG);
void drawString(coord pos, uint8_t pixelsPerPixel, char* string, uint16_t colourFG = COLOUR_WHITE, uint16_t colourBG = COLOUR_BLACK);
void drawIntWithoutPrecedingZeroes(coord pos, uint8_t pixelsPerPixel, int toWrite, uint16_t colourFG = COLOUR_WHITE, uint16_t colourBG = COLOUR_BLACK);
void drawIntWithPrecedingZeroes(coord pos, uint8_t pixelsPerPixel, int toWrite, uint16_t colourFG = COLOUR_WHITE, uint16_t colourBG = COLOUR_BLACK);
void drawUnfilledRect(coord pos, uint32_t w, uint32_t h, uint8_t lineWidth, uint16_t colour);
void drawUnfilledRectWithChar(coord pos, uint32_t w, uint32_t h, uint8_t lineWidth, uint16_t rectColour, char character, uint8_t fontSize);
// void writeNewChar(coord pos, char toWrite);

void drawPixel(framebuffer *buffer, int x,int y, uint32_t color);
void drawLine(framebuffer *buffer, int x,int y,int x2,int y2, uint32_t color);

void display_booting();
void display_home();
void display_charging();
void display_screen();

void beginSprite(int x, int y, int w, int h, bool clear=false);
void endSprite();
uint8_t* getLCDBuffer();

unsigned int dataHash(const char *s);

void handle_events();