/*
 * Copyright (c) 2020 Aaron Christophel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "display.h"

// #include <lvgl.h>
#include "fast_spi.h"
// #include "images.h"
#include "battery.h"
#include "touch.h"
#include "accl.h"
// #include "menu.h"
#include "ble.h"
#include "heartrate.h"
#include "backlight.h"
#include "inputoutput.h"
#include "bootloader.h"
#include "time.h"
#include "push.h"

#include "time.h"
#include "sleep.h"
#include "pedometer.h"

#include "watchface.h"

#define LCD_BUFFER_SIZE 15000  //LCD Buffer set to 15kbytes (approx = 1/2 RAM) meaning you can write up to 7500 pixels into the buffer without having to run over

uint8_t lcdBuffer[LCD_BUFFER_SIZE + 4];
uint32_t windowArea = 0;
uint32_t windowWidth = 0;
uint32_t windowHeight = 0;

void inc_tick() {
    // lv_tick_inc(40);
}

void init_display() {
    initDisplay();
    watchface_init();
}

void display_enable(bool state) {
    uint8_t temp[2];
    startWrite_display();
    if (state) {
        spiCommand(ST77XX_DISPON);
        spiCommand(ST77XX_SLPOUT);
    } else {
        spiCommand(ST77XX_SLPIN);
        spiCommand(ST77XX_DISPOFF);
    }
    endWrite_display();
}

void setAddrWindowDisplay(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    uint8_t temp[4];
    //y += 80; // when rotated screen
    spiCommand(0x2A);
    temp[0] = (x >> 8);
    temp[1] = x;
    temp[2] = ((x + w - 1) >> 8);
    temp[3] = (x + w - 1);
    write_fast_spi(temp, 4);
    spiCommand(0x2B);
    temp[0] = (y >> 8 );
    temp[1] = y;
    temp[2] = ((y + h - 1) >> 8);
    temp[3] = ((y + h - 1) & 0xFF);
    write_fast_spi(temp, 4);
    spiCommand(0x2C);
}

void initDisplay() {
    uint8_t temp[25];
    pinMode(LCD_CS, OUTPUT);
    pinMode(LCD_RS, OUTPUT);
    pinMode(LCD_RESET, OUTPUT);
    pinMode(LCD_DET, OUTPUT);

    digitalWrite(LCD_CS, HIGH);
    digitalWrite(LCD_RS, HIGH);

    digitalWrite(LCD_RESET, HIGH);
    delay(20);
    digitalWrite(LCD_RESET, LOW);
    delay(100);
    digitalWrite(LCD_RESET, HIGH);
    delay(100);
    startWrite_display();
    spiCommand(54);
    temp[0] = 0x00;//0xC0 when rotated screen
    write_fast_spi(temp, 1);
    spiCommand(58);
    temp[0] = 5;
    write_fast_spi(temp, 1);
    spiCommand(178);
    temp[0] = 12;
    temp[1] = 12;
    temp[2] = 0;
    temp[3] = 51;
    temp[4] = 51;
    write_fast_spi(temp, 5);
    spiCommand(183);
    temp[0] = 53;
    write_fast_spi(temp, 1);
    spiCommand(187);
    temp[0] = 25;
    write_fast_spi(temp, 1);
    spiCommand(192);
    temp[0] = 44;
    write_fast_spi(temp, 1);
    spiCommand(194);
    temp[0] = 1;
    write_fast_spi(temp, 1);
    spiCommand(195);
    temp[0] = 18;
    write_fast_spi(temp, 1);
    spiCommand(196);
    temp[0] = 32;
    write_fast_spi(temp, 1);
    spiCommand(198);
    temp[0] = 15;
    write_fast_spi(temp, 1);
    spiCommand(208);
    temp[0] = 164;
    temp[1] = 161;
    write_fast_spi(temp, 2);
    spiCommand(224);
    temp[0] = 208;
    temp[1] = 4;
    temp[2] = 13;
    temp[3] = 17;
    temp[4] = 19;
    temp[5] = 43;
    temp[6] = 63;
    temp[7] = 84;
    temp[8] = 76;
    temp[9] = 24;
    temp[10] = 13;
    temp[11] = 11;
    temp[12] = 31;
    temp[13] = 35;
    write_fast_spi(temp, 14);
    spiCommand(225);
    temp[0] = 208;
    temp[1] = 4;
    temp[2] = 12;
    temp[3] = 17;
    temp[4] = 19;
    temp[5] = 44;
    temp[6] = 63;
    temp[7] = 68;
    temp[8] = 81;
    temp[9] = 47;
    temp[10] = 31;
    temp[11] = 31;
    temp[12] = 32;
    temp[13] = 35;
    write_fast_spi(temp, 14);
    spiCommand(33);
    spiCommand(17);
    delay(120);
    spiCommand(41);
    spiCommand(0x11);
    spiCommand(0x29);
    endWrite_display();
}

void spiCommand(uint8_t d) {
    digitalWrite(LCD_RS, LOW);
    write_fast_spi(&d, 1);
    digitalWrite(LCD_RS, HIGH);
}

void startWrite_display(void) {
    enable_spi(true);
    digitalWrite(LCD_CS, LOW);
}

void endWrite_display(void) {
    digitalWrite(LCD_CS, HIGH);
    enable_spi(false);
}

/*
  Write a character to the screen position (x,y)
*/
void drawChar(coord pos, uint8_t pixelsPerPixel, char character, uint16_t colourFG, uint16_t colourBG) {
#ifndef ENABLE_ASTEROIDS
    startWrite_display();

    //Width and height of the character on the display
    int characterDispWidth = FONT_WIDTH * pixelsPerPixel;
    int characterDispHeight = FONT_HEIGHT * pixelsPerPixel;
    setDisplayWriteRegion({pos.x, pos.y}, characterDispWidth, characterDispHeight);  //Set the window of display memory to write to
    //Depending on the font, an offset to the current character index might be needed to skip over the unprintable characters
    int offset = FONT_NEEDS_OFFSET ? 32 : 0;

    //Row goes between 0 and font height, column goes between 0 and the font width
    for (int row = 0; row < FONT_HEIGHT; row++) {
        for (int col = 0; col < FONT_WIDTH; col++) {
            //(font[character][col] >> row) & 1 will return true if the font dictates that (col, row) should have a pixel there
            //drawCharPixelToBuffer writes into the LCD buffer the correct colour data for the current character pixel
            drawCharPixelToBuffer({col, row}, pixelsPerPixel, (font[character - offset][col] >> row) & 1, colourFG, colourBG);
        }
    }
    spiCommand(0x2C);
    //Size 8 is probably the largest useful font, and at that size, a character takes up < 6000 bytes in the buffer, meaning we are nowhere near to filling up the buffer with a character
    write_fast_spi(lcdBuffer, characterDispWidth * characterDispHeight * 2);  //Write the character to the display

    endWrite_display();
#endif
}

/*
  Add pixel data into the LCD buffer for the character's current pixel
  (logic is explained in Writeup.md)
*/
void drawCharPixelToBuffer(coord charPos, uint8_t pixelsPerPixel, bool pixelInCharHere, uint16_t colourFG, uint16_t colourBG) {
    int columnFontIndexScaledByPixelCount = charPos.x * pixelsPerPixel;
    int rowFontIndexScaledByPixelCount = charPos.y * pixelsPerPixel;
    int pixelsPerRow = FONT_WIDTH * pixelsPerPixel;

    if (colourFG == COLOUR_WHITE && colourBG == COLOUR_BLACK) {
        //If we are writing white on black (very common), hardcode the colours and don't use the ones from the parameter
        //This is slightly more efficient
        for (int i = 0; i < pixelsPerPixel; i++) {
            for (int j = 0; j < pixelsPerPixel; j++) {
                lcdBuffer[2 * ((rowFontIndexScaledByPixelCount + i) * pixelsPerRow + (columnFontIndexScaledByPixelCount + j))] = pixelInCharHere ? 0xFF : 0x00;
                lcdBuffer[2 * ((rowFontIndexScaledByPixelCount + i) * pixelsPerRow + (columnFontIndexScaledByPixelCount + j)) + 1] = pixelInCharHere ? 0xFF : 0x00;
            }
        }
    } else {
        for (int i = 0; i < pixelsPerPixel; i++) {
            for (int j = 0; j < pixelsPerPixel; j++) {
                lcdBuffer[2 * ((rowFontIndexScaledByPixelCount + i) * pixelsPerRow + (columnFontIndexScaledByPixelCount + j))] = pixelInCharHere ? (colourFG >> 8) & 0xFF : (colourBG >> 8) & 0xFF;
                lcdBuffer[2 * ((rowFontIndexScaledByPixelCount + i) * pixelsPerRow + (columnFontIndexScaledByPixelCount + j)) + 1] = pixelInCharHere ? colourFG & 0xFF : colourBG & 0xFF;
            }
        }
    }
}

/*
  Write a string to the specified position using a string literal (null terminated char array)
*/
void drawString(coord pos, uint8_t pixelsPerPixel, char* string, uint16_t colourFG, uint16_t colourBG) {

    asteroidDrawString(pos.x, pos.y, string, 0.5 + ((float)pixelsPerPixel/4), colourFG);
    return;

    int currentLine = 0;      //Current line
    int charPos = 0;          //Position of the character we are on along the line
    int i = 0;                //Character index
    while (string[i] != 0) {  //Loop through every character of the string (only stop when you reach the null terminator)
        drawChar({pos.x + charPos * pixelsPerPixel * FONT_WIDTH + pixelsPerPixel * charPos,
                  pos.y + currentLine * FONT_HEIGHT * pixelsPerPixel},
                 pixelsPerPixel, string[i], colourFG, colourBG);
        //Move to the next character
        charPos++;
        i++;
    }
}

/*
  Write an (up to 9 digit) integer to x,y, without preceding zeroes (useful when you know the numbers you are writing will have the same number of digits on rewriting)
  The logic for writing the string is basically the same as drawString
*/
void drawIntWithoutPrecedingZeroes(coord pos, uint8_t pixelsPerPixel, int toWrite, uint16_t colourFG, uint16_t colourBG) {
    if (toWrite == 0) {
        drawChar({pos.x, pos.y}, pixelsPerPixel, '0', colourFG, colourBG);
        return;
    }
    //Byte array for storing the digits
    uint8_t digits[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t i = 0;
    //Write the number (of num digits n) into the LAST n indexes of the digits array (everything preceding the number will be 0)
    while (toWrite) {
        digits[8 - i] = toWrite % 10;
        toWrite /= 10;
        i++;
    }
    //Find which index holds the first (non-zero) digit of the number
    for (i = 0; i < 9; i++) {
        if (digits[i] == 0)
            continue;
        break;
    }
    int currentLine = 0;  //Current line
    int charPos = 0;      //Position of the character we are on along the line
    char charToWrite[1];
    //Do the normal writing routine but for every digit of the number, starting from the first digit
    for (; i < 9; i++) {  //Loop through every character of the string
        //If printing the next character would result in it being of screen
        if (pos.x + charPos * pixelsPerPixel * FONT_WIDTH + pixelsPerPixel * charPos > 240 - FONT_WIDTH) {
            currentLine++;
            charPos = 0;
        }
        sprintf(charToWrite, "%d", digits[i]);
        drawChar({pos.x + charPos * pixelsPerPixel * FONT_WIDTH + pixelsPerPixel * charPos, pos.y + currentLine * FONT_HEIGHT * pixelsPerPixel}, pixelsPerPixel, charToWrite[0], colourFG, colourBG);
        charPos++;
    }
}

/*
  Write a number always with 9 digits to x,y (with preceding zeroes for variable length rewrites)
*/
void drawIntWithPrecedingZeroes(coord pos, uint8_t pixelsPerPixel, int toWrite, uint16_t colourFG, uint16_t colourBG) {
    if (toWrite == 0) {
        drawString({pos.x, pos.y}, pixelsPerPixel, "000000000", colourFG, colourBG);
        return;
    }
    //Byte array for storing the digits
    uint8_t digits[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t i = 0;
    //Write the number (of num digits n) into the LAST n indexes of the digits array (everything preceding the number will be 0)
    while (toWrite) {
        digits[8 - i] = toWrite % 10;
        toWrite /= 10;
        i++;
    }
    int currentLine = 0;  //Current line
    int charPos = 0;      //Position of the character we are on along the line
    char charToWrite[1];
    //Do the normal writing routine but for every digit of the number, starting from the 0th index (writing preceding zeroes)
    for (i = 0; i < 9; i++) {  //Loop through every character of the string
        //If printing the next character would result in it being of screen
        if (pos.x + charPos * pixelsPerPixel * FONT_WIDTH + pixelsPerPixel * charPos > 240 - FONT_WIDTH) {
            currentLine++;
            charPos = 0;
        }
        sprintf(charToWrite, "%d", digits[i]);
        drawChar({pos.x + charPos * pixelsPerPixel * FONT_WIDTH + pixelsPerPixel * charPos, pos.y + currentLine * FONT_HEIGHT * pixelsPerPixel}, pixelsPerPixel, charToWrite[0], colourFG, colourBG);
        charPos++;
    }
}

/*
  Draw a rect with origin x,y and width w, height h
*/
void drawFilledRect(coord pos, uint32_t w, uint32_t h, uint16_t colour) {
    startWrite_display();
    setDisplayWriteRegion({pos.x, pos.y}, w, h);
    spiCommand(0x2C);  //Memory write
    uint32_t numberOfBytesToWriteToLCD;
    uint32_t numberBytesInWindowArea = (windowArea * 2);
    uint32_t lcdBufferSize = LCD_BUFFER_SIZE;  //Size of LCD buffer
    //If we are comfortable that the number of bytes the current window will hold in a buffer is less than the max buffer size:
    if (numberBytesInWindowArea < lcdBufferSize)
        lcdBufferSize = numberBytesInWindowArea;  //Set the buffer size to be that of the window area * 2 (number of bytes that the window would occupy)

    for (int i = 0; i <= lcdBufferSize; i++) {  //Loop through buffer
        //Write every pixel (half-word) into the LCD buffer
        lcdBuffer[i++] = (colour >> 8) & 0xFF;  //Post increment meaning that it first writes to position i, then increments i
        lcdBuffer[i] = colour & 0xFF;           //Writes to the (now) position of i++
    }
    do {
        if (numberBytesInWindowArea >= LCD_BUFFER_SIZE)
            numberOfBytesToWriteToLCD = LCD_BUFFER_SIZE;
        else
            numberOfBytesToWriteToLCD = numberBytesInWindowArea;
        write_fast_spi(lcdBuffer, numberOfBytesToWriteToLCD);
        numberBytesInWindowArea -= numberOfBytesToWriteToLCD;
    } while (numberBytesInWindowArea > 0);
    endWrite_display();
}

void drawFilledRect2(coord pos, uint32_t w, uint32_t h, uint16_t colour) {
    startWrite_display();
    setDisplayWriteRegion({pos.x, pos.y}, w, h);
    spiCommand(0x2C);  //Memory write
    uint32_t numberOfBytesToWriteToLCD;
    uint32_t numberBytesInWindowArea = (windowArea * 2);
    uint32_t lcdBufferSize = LCD_BUFFER_SIZE;  //Size of LCD buffer
    //If we are comfortable that the number of bytes the current window will hold in a buffer is less than the max buffer size:
    if (numberBytesInWindowArea < lcdBufferSize)
        lcdBufferSize = numberBytesInWindowArea;  //Set the buffer size to be that of the window area * 2 (number of bytes that the window would occupy)

    for (int i = 0; i <= lcdBufferSize; i++) {  //Loop through buffer
        //Write every pixel (half-word) into the LCD buffer
        lcdBuffer[i++] = colour & 0xFF;  //Post increment meaning that it first writes to position i, then increments i
        lcdBuffer[i] = (colour >> 8) & 0xFF;           //Writes to the (now) position of i++
    }
    do {
        if (numberBytesInWindowArea >= LCD_BUFFER_SIZE)
            numberOfBytesToWriteToLCD = LCD_BUFFER_SIZE;
        else
            numberOfBytesToWriteToLCD = numberBytesInWindowArea;
        write_fast_spi(lcdBuffer, numberOfBytesToWriteToLCD);
        numberBytesInWindowArea -= numberOfBytesToWriteToLCD;
    } while (numberBytesInWindowArea > 0);
    endWrite_display();
}

/*
  Draw a rectangle with outline of width lineWidth
 */
void drawUnfilledRect(coord pos, uint32_t w, uint32_t h, uint8_t lineWidth, uint16_t colour) {
    drawFilledRect({pos.x, pos.y}, w, h, COLOUR_BLACK);  //Clear rect of stuff
    //Top rect
    drawFilledRect({pos.x, pos.y}, w, lineWidth, colour);
    //Bottom rect
    drawFilledRect({pos.x, pos.y + h - lineWidth}, w, lineWidth, colour);
    //Left rect
    drawFilledRect({pos.x, pos.y}, lineWidth, h, colour);
    //Right rect
    drawFilledRect({pos.x + w - lineWidth, pos.y}, lineWidth, h, colour);
}

/*
  Draw a rect outline with character in the middle (look at writeup for explanation)
 */
void drawUnfilledRectWithChar(coord pos, uint32_t w, uint32_t h, uint8_t lineWidth, uint16_t rectColour, char character, uint8_t fontSize) {
    drawUnfilledRect({pos.x, pos.y}, w, h, lineWidth, rectColour);
    drawChar({(pos.x + (w / 2)) - (STR_WIDTH("-", fontSize) / 2),
              (pos.y + (h / 2)) - ((FONT_HEIGHT * fontSize) / 2)},
             fontSize, character, COLOUR_WHITE, COLOUR_BLACK);
}

/*
  Set the column and row RAM addresses for writing to the display
  You must select a region in the LCD RAM to write pixel data to
  This region has an xStart, xEnd, yStart and yEnd address
  As you write half-words (pixels) over SPI, the RAM fills horizontally per row
*/
void setDisplayWriteRegion(coord pos, uint32_t w, uint32_t h) {
    uint8_t buf[4];  //Parameter buffer
    windowHeight = h;
    windowWidth = w;
    windowArea = w * h;    //Calculate window area
    spiCommand(0x2A);  //Column address set
    buf[0] = 0x00;         //Padding write value to make it 16 bit
    buf[1] = pos.x;
    buf[2] = 0x00;
    buf[3] = (pos.x + w - 1);
    write_fast_spi(buf, 4);
    spiCommand(0x2B);  //Row address set
    buf[0] = 0x00;
    buf[1] = pos.y;
    buf[2] = 0x00;
    buf[3] = ((pos.y + h - 1) & 0xFF);
    write_fast_spi(buf, 4);
}

/*
  Clear display (clear whole display when no arg (or false) is passed in)
*/
void clearDisplay(bool leaveAppDrawer) {
    // drawFilledRect({0, 0}, 240, leaveAppDrawer ? 213 : 240, 0x0000);
    drawFilledRect({0, 0}, 240, 240, 0x0000);
}

// line renderer
void drawPixel(framebuffer *buffer, int x,int y,byte color)
{
    if (x<0 || y<0 || x>=buffer->width || y>=buffer->height)
        return;

    byte *pDest=(byte*)buffer->pixels  + ((x) + (y * buffer->pitch>>1));
    *pDest=color;
}

void drawLine(framebuffer *buffer,int x,int y,int x2,int y2, uint32_t color)
{
    // if (buffer->pixels == 0) {
    //     buffer->pixels = lcdBuffer;
    // }

    int lx=FPABS((x-x2));
    int ly=FPABS((y-y2));

    int l=lx>ly ? lx : ly;
    int fl=(l<<FPP);
    if ((fl>>FPP)==0) fl=FPONE;

    int sx=x<<FPP;
    int sy=y<<FPP;
    int xx=(x2-x)<<FPP;
    int yy=(y2-y)<<FPP;

    xx=FPDIV(xx,fl);
    yy=FPDIV(yy,fl);

    while(l>0) {
        coord pos;
        pos.x = sx >> FPP;
        pos.y = sy >> FPP;
        if (pos.x < 240 || pos.y < 240) {
            drawFilledRect(pos, 1, 1, color);
        }
        // drawPixel(buffer,(sx>>FPP),(sy>>FPP),color);
        sx+=xx;
        sy+=yy;
        l--;
    }
}


int currentScreen = 0;

void display_booting()
{
    clearDisplay();
    drawFilledRect({32,32}, 10, 10, COLOUR_RED);
}

void display_home()
{
    clearDisplay();
    display_screen();
}

void display_charging()
{
    clearDisplay();
    // drawFilledRect({32,32}, 10, 10, COLOUR_RED);
}

void padSpaces(char *tmp, int c) {
    int ln = strlen(tmp);
    for(int i=0; i<c; i++) {
        tmp[i + ln] = ' ';
        tmp[i + ln + 1] = 0;
    }
}

touch_data_struct td;

static int infoHash = 0;
static accl_data_struct accl;
void _display_info()
{
    int x = 24;
    int y = 24;
    char tmp[32];
    sprintf(tmp, "ICEDMAN %d %d", accl.x, accl.y);
    // sprintf(tmp, "ICEDMAN");
    padSpaces(tmp, 4);
    drawString({ x, y }, 2, tmp, COLOUR_WHITE, COLOUR_BLACK);
    y += 24 + 2;
}

static int prevInfoHash = 0;
void _display_screen()
{
    accl = get_accl_data();
    get_heartrate_ms();
    int heart = get_heartrate();
    int bat = get_battery_percent();

    char tmp[32];
    int ii = 0;
    int steps = get_steps();
    time_data_struct time = get_time();
    tmp[ii++] = time.hr;
    tmp[ii++] = time.min;
    tmp[ii++] = time.day;
    tmp[ii++] = bat;
    tmp[ii++] = heart;
    tmp[ii++] = steps;
    tmp[ii++] = 0;

    int hs = dataHash(tmp);
    if (hs == prevInfoHash) {
        return;
    }
    prevInfoHash = hs;

    _display_info();

    int x = 24;
    int y = 24;
    y += 24 + 2;

    getTime(tmp);
    padSpaces(tmp, 4);
    drawString({ x, y }, 4, tmp, COLOUR_WHITE, COLOUR_BLACK);
    y += 24 * 2;

    getDay(tmp);
    padSpaces(tmp, 4);
    drawString({ x, y }, 2, tmp, COLOUR_WHITE, COLOUR_BLACK);
    y += 24;

    getDate(tmp);
    padSpaces(tmp, 4);
    drawString({ x, y }, 2, tmp, COLOUR_WHITE, COLOUR_BLACK);
    y += 24;

    sprintf(tmp, "power: %d", bat);
    padSpaces(tmp, 4);
    drawString({ x, y }, 2, tmp, COLOUR_WHITE, COLOUR_BLACK);
    y += 24;

    sprintf(tmp, "heart: %d",heart);
    padSpaces(tmp, 4);
    drawString({ x, y }, 2, tmp, COLOUR_WHITE, COLOUR_BLACK);
    y += 24;

    sprintf(tmp, "steps: %d",steps);
    padSpaces(tmp, 4);
    drawString({ x, y }, 2, tmp, COLOUR_WHITE, COLOUR_BLACK);
    y += 24;

    // asteroidDrawChar(32, 80, 'X', 4, COLOUR_RED);
    // asteroidDrawString(32, 80, "HELLO", 3, COLOUR_RED);
}

void display_screen()
{
    // _display_screen();
    if (currentScreen == 1) {
        _display_screen();
        return;
    }
    watchface_draw();
}

void handle_events()
{
    if (get_button()) {
        if (currentScreen != 0) {
            currentScreen = 0;
            watchface_clear();
            set_sleep_time();
            return;
        } else {
            if (!get_sleep()) {
                sleep_down();
            }
        }
    }

    td = get_touch();

    if (td.gesture == TOUCH_SLIDE_DOWN) {
        currentScreen ++;
        prevInfoHash = 0;
        watchface_clear();
        set_sleep_time();

        // if (currentScreen == 1) {
        //     start_hrs3300();
        // } else {
        //     end_hrs3300();
        // }

    } else if (td.gesture == TOUCH_SLIDE_UP) {
        currentScreen --;
        prevInfoHash = 0;
        watchface_clear();
        set_sleep_time();

        // if (currentScreen == 1) {
        //     start_hrs3300();
        // } else {
        //     end_hrs3300();
        // }
    }

    if (currentScreen < 0) currentScreen = 1;
    if (currentScreen > 1) currentScreen = 0;
}

unsigned int dataHash(const char *s)
{
    int hash = 0;

    while (*s != 0)
    {
        hash *= 37;
            hash += *s;
        s++;
    }

    return hash;
}
