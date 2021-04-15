#include "asteroidsfont.h"
#include "display.h"
#include "fast_spi.h"

typedef struct
{
    byte points[8]; // 4 bits x, 4 bits y
} asteroids_char_t;

#define FONT_UP 0xFE
#define FONT_LAST 0xFF

#define P(x,y)  ((((x) & 0xF) << 4) | (((y) & 0xF) << 0))

asteroids_char_t asteroids_font[256];
coord startPosition;
coord nextPosition;

void initAsteroids() {
#ifdef ENABLE_ASTEROIDS
    asteroids_font['0' - 0x20] = { P(0,0), P(8,0), P(8,12), P(0,12), P(0,0), P(8,12), FONT_LAST };
    asteroids_font['1' - 0x20] = { P(4,0), P(4,12), P(3,10), FONT_LAST };
    asteroids_font['2' - 0x20] = { P(0,12), P(8,12), P(8,7), P(0,5), P(0,0), P(8,0), FONT_LAST };
    asteroids_font['3' - 0x20] = { P(0,12), P(8,12), P(8,0), P(0,0), FONT_UP, P(0,6), P(8,6), FONT_LAST };
    asteroids_font['4' - 0x20] = { P(0,12), P(0,6), P(8,6), FONT_UP, P(8,12), P(8,0), FONT_LAST };
    asteroids_font['5' - 0x20] = { P(0,0), P(8,0), P(8,6), P(0,7), P(0,12), P(8,12), FONT_LAST };
    asteroids_font['6' - 0x20] = { P(0,12), P(0,0), P(8,0), P(8,5), P(0,7), FONT_LAST };
    asteroids_font['7' - 0x20] = { P(0,12), P(8,12), P(8,6), P(4,0), FONT_LAST };
    asteroids_font['8' - 0x20] = { P(0,0), P(8,0), P(8,12), P(0,12), P(0,0), FONT_UP, P(0,6), P(8,6), };
    asteroids_font['9' - 0x20] = { P(8,0), P(8,12), P(0,12), P(0,7), P(8,5), FONT_LAST };
    asteroids_font[' ' - 0x20] = { FONT_LAST };
    asteroids_font['.' - 0x20] = { P(3,0), P(4,0), FONT_LAST };
    asteroids_font[',' - 0x20] = { P(2,0), P(4,2), FONT_LAST };
    asteroids_font['-' - 0x20] = { P(2,6), P(6,6), FONT_LAST };
    asteroids_font['+' - 0x20] = { P(1,6), P(7,6), FONT_UP, P(4,9), P(4,3), FONT_LAST };
    asteroids_font['!' - 0x20] = { P(4,0), P(3,2), P(5,2), P(4,0), FONT_UP, P(4,4), P(4,12), FONT_LAST };
    asteroids_font['#' - 0x20] = { P(0,4), P(8,4), P(6,2), P(6,10), P(8,8), P(0,8), P(2,10), P(2,2) };
    asteroids_font['^' - 0x20] = { P(2,6), P(4,12), P(6,6), FONT_LAST };
    asteroids_font['=' - 0x20] = { P(1,4), P(7,4), FONT_UP, P(1,8), P(7,8), FONT_LAST };
    asteroids_font['*' - 0x20] = { P(0,0), P(4,12), P(8,0), P(0,8), P(8,8), P(0,0), FONT_LAST };
    asteroids_font['_' - 0x20] = { P(0,0), P(8,0), FONT_LAST };
    asteroids_font['/' - 0x20] = { P(0,0), P(8,12), FONT_LAST };
    asteroids_font['\\' - 0x20] = { P(0,12), P(8,0), FONT_LAST };
    asteroids_font['@' - 0x20] = { P(8,4), P(4,0), P(0,4), P(0,8), P(4,12), P(8,8), P(4,4), P(3,6) };
    asteroids_font['$' - 0x20] = { P(6,2), P(2,6), P(6,10), FONT_UP, P(4,12), P(4,0), FONT_LAST };
    asteroids_font['&' - 0x20] = { P(8,0), P(4,12), P(8,8), P(0,4), P(4,0), P(8,4), FONT_LAST };
    asteroids_font['[' - 0x20] = { P(6,0), P(2,0), P(2,12), P(6,12), FONT_LAST };
    asteroids_font[']' - 0x20] = { P(2,0), P(6,0), P(6,12), P(2,12), FONT_LAST };
    asteroids_font['(' - 0x20] = { P(6,0), P(2,4), P(2,8), P(6,12), FONT_LAST };
    asteroids_font[')' - 0x20] = { P(2,0), P(6,4), P(6,8), P(2,12), FONT_LAST };
    asteroids_font['{' - 0x20] = { P(6,0), P(4,2), P(4,10), P(6,12), FONT_UP, P(2,6), P(4,6), FONT_LAST };
    asteroids_font['}' - 0x20] = { P(4,0), P(6,2), P(6,10), P(4,12), FONT_UP, P(6,6), P(8,6), FONT_LAST };
    asteroids_font['%' - 0x20] = { P(0,0), P(8,12), FONT_UP, P(2,10), P(2,8), FONT_UP, P(6,4), P(6,2) };
    asteroids_font['<' - 0x20] = { P(6,0), P(2,6), P(6,12), FONT_LAST };
    asteroids_font['>' - 0x20] = { P(2,0), P(6,6), P(2,12), FONT_LAST };
    asteroids_font['|' - 0x20] = { P(4,0), P(4,5), FONT_UP, P(4,6), P(4,12), FONT_LAST };
    asteroids_font[':' - 0x20] = { P(4,9), P(4,7), FONT_UP, P(4,5), P(4,3), FONT_LAST };
    asteroids_font[';' - 0x20] = { P(4,9), P(4,7), FONT_UP, P(4,5), P(1,2), FONT_LAST };
    asteroids_font['"' - 0x20] = { P(2,10), P(2,6), FONT_UP, P(6,10), P(6,6), FONT_LAST };
    asteroids_font['\'' - 0x20] = { P(2,6), P(6,10), FONT_LAST };
    asteroids_font['`' - 0x20] = { P(2,10), P(6,6), FONT_LAST };
    asteroids_font['~' - 0x20] = { P(0,4), P(2,8), P(6,4), P(8,8), FONT_LAST };
    asteroids_font['?' - 0x20] = { P(0,8), P(4,12), P(8,8), P(4,4), FONT_UP, P(4,1), P(4,0), FONT_LAST };
    asteroids_font['A' - 0x20] = { P(0,0), P(0,8), P(4,12), P(8,8), P(8,0), FONT_UP, P(0,4), P(8,4) };
    asteroids_font['B' - 0x20] = { P(0,0), P(0,12), P(4,12), P(8,10), P(4,6), P(8,2), P(4,0), P(0,0) };
    asteroids_font['C' - 0x20] = { P(8,0), P(0,0), P(0,12), P(8,12), FONT_LAST };
    asteroids_font['D' - 0x20] = { P(0,0), P(0,12), P(4,12), P(8,8), P(8,4), P(4,0), P(0,0), FONT_LAST };
    asteroids_font['E' - 0x20] = { P(8,0), P(0,0), P(0,12), P(8,12), FONT_UP, P(0,6), P(6,6), FONT_LAST };
    asteroids_font['F' - 0x20] = { P(0,0), P(0,12), P(8,12), FONT_UP, P(0,6), P(6,6), FONT_LAST };
    asteroids_font['G' - 0x20] = { P(6,6), P(8,4), P(8,0), P(0,0), P(0,12), P(8,12), FONT_LAST };
    asteroids_font['H' - 0x20] = { P(0,0), P(0,12), FONT_UP, P(0,6), P(8,6), FONT_UP, P(8,12), P(8,0) };
    asteroids_font['I' - 0x20] = { P(0,0), P(8,0), FONT_UP, P(4,0), P(4,12), FONT_UP, P(0,12), P(8,12) };
    asteroids_font['J' - 0x20] = { P(0,4), P(4,0), P(8,0), P(8,12), FONT_LAST };
    asteroids_font['K' - 0x20] = { P(0,0), P(0,12), FONT_UP, P(8,12), P(0,6), P(6,0), FONT_LAST };
    asteroids_font['L' - 0x20] = { P(8,0), P(0,0), P(0,12), FONT_LAST };
    asteroids_font['M' - 0x20] = { P(0,0), P(0,12), P(4,8), P(8,12), P(8,0), FONT_LAST };
    asteroids_font['N' - 0x20] = { P(0,0), P(0,12), P(8,0), P(8,12), FONT_LAST };
    asteroids_font['O' - 0x20] = { P(0,0), P(0,12), P(8,12), P(8,0), P(0,0), FONT_LAST };
    asteroids_font['P' - 0x20] = { P(0,0), P(0,12), P(8,12), P(8,6), P(0,5), FONT_LAST };
    asteroids_font['Q' - 0x20] = { P(0,0), P(0,12), P(8,12), P(8,4), P(0,0), FONT_UP, P(4,4), P(8,0) };
    asteroids_font['R' - 0x20] = { P(0,0), P(0,12), P(8,12), P(8,6), P(0,5), FONT_UP, P(4,5), P(8,0) };
    asteroids_font['S' - 0x20] = { P(0,2), P(2,0), P(8,0), P(8,5), P(0,7), P(0,12), P(6,12), P(8,10) };
    asteroids_font['T' - 0x20] = { P(0,12), P(8,12), FONT_UP, P(4,12), P(4,0), FONT_LAST };
    asteroids_font['U' - 0x20] = { P(0,12), P(0,2), P(4,0), P(8,2), P(8,12), FONT_LAST };
    asteroids_font['V' - 0x20] = { P(0,12), P(4,0), P(8,12), FONT_LAST };
    asteroids_font['W' - 0x20] = { P(0,12), P(2,0), P(4,4), P(6,0), P(8,12), FONT_LAST };
    asteroids_font['X' - 0x20] = { P(0,0), P(8,12), FONT_UP, P(0,12), P(8,0), FONT_LAST };
    asteroids_font['Y' - 0x20] = { P(0,12), P(4,6), P(8,12), FONT_UP, P(4,6), P(4,0), FONT_LAST };
    asteroids_font['Z' - 0x20] = { P(0,12), P(8,12), P(0,0), P(8,0), FONT_UP, P(2,6), P(6,6), FONT_LAST };
#endif
}

int asteroidDrawChar(float x, float y, char c, float size, int clr, bool extentsOnly) { 

    if (c >= 'a' && c <= 'z') {
        c += 'A' - 'a';
    }

    framebuffer buffer;
    float width = 8 * size;
    float height = 8 * size;
    float ww = width * 1.5;
    float hh = height * 1.5;
    coord pos = { x, y + height };

    int characterDispWidth = width;
    int characterDispHeight = height;

    buffer.width = width + 4;
    buffer.height = hh + 4;

    if (!extentsOnly) {
        beginSprite(pos.x, pos.y, buffer.width, buffer.height, true);
    }

#ifdef ENABLE_ASTEROIDS
    byte *pts = asteroids_font[(int)c - ' '].points;
    int next_moveto = 1;

    float adv = x;
    for(int i = 0 ; i < 8 ; i++)
    {
        int delta = pts[i];
        if (delta == FONT_LAST)
            break;
        if (delta == FONT_UP)
        {
            next_moveto = 1;
            continue;
        }

        float dx = ((delta >> 4) & 0xF) * size;
        float dy = -((delta >> 0) & 0xF) * size;

        if (next_moveto != 0) {
            // moveto(x + dx, y + dy);
            startPosition = { x + dx, buffer.height - 4 + y + dy };

            if (x + dx > adv) {
                adv = x + dx + (size * 4);
            }

        } else {
            // lineto(x + dx, y + dy);
            nextPosition = { x + dx, buffer.height - 4 + y + dy };

            if (x + dx > adv) {
                adv = x + dx + (size * 4);
            }

            if (!extentsOnly) {
                drawLine(
                    &buffer,
                  (int)startPosition.x - x,
                  (int)startPosition.y - y + 2,
                  (int)nextPosition.x - x,
                  (int)nextPosition.y - y + 2,
                  clr
                );
            }

            startPosition = nextPosition;
        }

        next_moveto = 0;
    }

    adv -= x;
    if (adv < 12 * size) {
        adv = 12 * size;
    }

    if (!extentsOnly) {
        endSprite();
    }

    return (int)adv;
#else
    return 0;
#endif
}

int asteroidDrawString(float x, float y, char *str, float size, int clr, bool extentsOnly)
{
#ifdef ENABLE_ASTEROIDS
    int adv = 0;
    char *cc = str;
    while(cc[0] != 0) {
        adv += asteroidDrawChar(x + adv, y, cc[0], size, clr);
        cc++;
    }
    return 0;
#else
    return 0;
#endif
}