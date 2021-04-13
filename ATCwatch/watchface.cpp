#include <lvgl.h>
#include "watchface.h"
#include "face.h"

#include "display.h"
#include "time.h"
#include "ble.h"
#include "battery.h"

#define READ16(src, p) (((uint16_t)(src[p+1]) << 8) | (uint16_t)(src[p]))
#define READ16_(src, p) (((uint16_t)(src[p]) << 8) | (uint16_t)(src[p+1]))
#define READ32(src, p) (((uint32_t)(src[p+3]) << 24) | ((uint32_t)(src[p+2]) << 16) | \
                    ((uint32_t)(src[p+1]) << 8) | (uint32_t)(src[p]))

#define NORMALIZE(v, source, target) ((v / source) * target )

#define EntitiesOffset 11
#define SpriteOffsetsOffset 200
#define SpriteSizesOffset 1200
#define SpriteDataOffset 1700

#define CANVAS_WIDTH 240
#define CANVAS_HEIGHT 240

#define MAX_SPRITES 64

static bool hasBg;

void watchface_init(void)
{
    for(int j=0; ; j++) {
        watchface_gfx_t *g = &watchface_gfx[j];
        if (!g->type) break;
        g->sprite = 0;
    }

    const uint8_t *ptr = (uint8_t*)face;

    uint8_t header = ptr[0];
    uint8_t entityCount = ptr[1];
    uint8_t spriteCount = ptr[2];
    uint16_t id = READ16(ptr, 3);
    uint8_t bg = ptr[5];
    uint8_t bg_width = ptr[8];
    uint8_t bg_height = ptr[9];

    hasBg = bg;

    // printf("header: %d\n", header);
    // printf("entityCount: %d\n", entityCount);
    // printf("spriteCount: %d\n", spriteCount);
    // printf("id: %d\n", id);
    // printf("bg: %d\n", bg);
    // printf("bg_width: %d\n", bg_width);
    // printf("bg_height: %d\n", bg_height);

    for (int i = 0; i < entityCount; i++) {
        const uint8_t offset = EntitiesOffset + i * 6;
        const uint8_t type = ptr[offset];

        watchface_gfx_t *gfx = 0;

        for(int j=0; ; j++) {
            watchface_gfx_t *g = &watchface_gfx[j];
            if (!g->type) break;
            if (g->type == type) {
                // printf("%d %s\n", g->type, g->description);
                gfx = g;
                break;
            }
        }

        if (!gfx) continue;

        gfx->type = type;
        gfx->x = ptr[offset + 1];
        gfx->y = ptr[offset + 2];
        gfx->w = ptr[offset + 3];
        gfx->h = ptr[offset + 4];
        gfx->sprite = ptr[offset + 5];

        if (gfx->sprite >= MAX_SPRITES) gfx->sprite = 0;
    }

    for (int i = 0; i < spriteCount && i < MAX_SPRITES; i++) {
        const uint32_t spriteOffset = SpriteDataOffset + READ32(ptr, SpriteOffsetsOffset + i * 4);
        const uint16_t spriteSize = READ16(ptr, SpriteSizesOffset + i * 2);
        const uint16_t spriteType = READ16_(ptr, spriteOffset);
        watchface_spr[i].offset = spriteOffset + 2;
        watchface_spr[i].size = spriteType == 0x0821 ? spriteSize - 2 : 0;
        // // printf("%d 0x%04x\n", i, spriteType);
    }

    watchface_draw();
}

void draw_sprite(int id, int x, int y, int w, int h)
{
    const uint8_t *ptr = (uint8_t*)face;
    watchface_gfx_spr_t *s = &watchface_spr[id];

    int px = 0;
    int py = 0;
    for (uint32_t i = s->offset; i < s->offset + s->size; i += 3) {
        uint16_t pixel = READ16(ptr, i);
        uint8_t sz = ptr[i+2];

        uint8_t gg = NORMALIZE((pixel >> (5 + 6)) & 0x1f, 0x1f, 0xff);
        uint8_t bb = NORMALIZE((pixel >> 5) & 0x3f, 0x3f, 0xff);
        uint8_t rr = NORMALIZE(pixel & 0x1f, 0x1f, 0xff);
        // uint16_t rgb = (rr << (5+6)) | (gg << 5) | bb;
        uint16_t rgb = (bb << (5+6)) | (gg << 5) | rr;

#if 0
        for(int k =0; k<sz; k++) {

            coord pos;
            pos.x = x + px;
            pos.y = y + py;

            drawFilledRect(pos, 1, 1, rgb);

            px++;
            if (px >= w) {
                px = 0;
                py ++;
                if (py >= h) break;
            }
        }
#endif

        for(int k=0; k<sz; ) {
            int ssz = sz - k;
            if (px + ssz > w) {
                ssz = w - px;
            }

            // display_draw_hline(x + px, y + py, ssz, rgb);
            coord pos;
            pos.x = x + px;
            pos.y = y + py;
            drawFilledRect(pos, ssz, 1, rgb);

            px += ssz;
            if (px >= w) {
                px = 0;
                py ++;
            }
            k += ssz;
        }
    }
}

unsigned int Hash(const char *s)
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

static uint32_t prevHash  = 0;
void watchface_draw(void)
{  
    // background
    // if (hasBg) {
        // for (int i = 0; i < 5; i++) {
        //     draw_sprite(i, 0, 24*i, 240, 24);
        // }
    // }

    time_data_struct tm = get_time();
    uint8_t dofw = getDayOfWeek();

    int ii = 0;
    char tmp[32];
    tmp[ii++] = tm.hr;
    tmp[ii++] = tm.min;
    tmp[ii++] = dofw;
    tmp[ii++] = get_vars_ble_connected();
    tmp[ii++] = get_battery_percent();
    tmp[ii++] = 0;

    uint32_t hs = Hash(tmp);
    if (hs == prevHash) return;
    prevHash = hs;

    for(int j=0; ;j++) {
        watchface_gfx_t *g = &watchface_gfx[j];
        if (!g->type) break;
        if (!g->sprite) continue;

        int id = g->sprite;
        switch(g->type) {
        case 0x40:
            id = g->sprite + (tm.hr / 10) % 10;
            break;
        case 0x41:
            id = g->sprite + (tm.hr % 10);
            break;
        case 0x43:
            id = g->sprite + (tm.min / 10) % 10;
            break;
        case 0x44:
            id = g->sprite + (tm.min % 10);
            break;
        case 0x60:
        case 0x61:
            id = g->sprite + (dofw % 7);
            break;
        // case 0xd4:
        case 0xd1: // battery icon
            id = g->sprite;
            // id = g->sprite + (get_battery_percent() / 100);
            break;
        case 0xc0:
            if (!get_vars_ble_connected()) {
                id = 0;
                coord pos;
                pos.x = g->x;
                pos.y = g->y;
                drawFilledRect(pos, g->w, g->h, 0);

            }
            break;
        }

        if (id == 0)
            continue;

        draw_sprite(id, g->x, g->y, g->w, g->h);

        if (g->type == 0xd1) {
            int p = 2;
            int ww = g->w * get_battery_percent() / 100;
            // for(int i=p; i<g->h-(p*2); i++) {
            //     display_draw_hline(g->x + ww + p, g->y + i, g->w - ww - (p*2) - 1, 0);
            // }
            coord pos;
            pos.x = g->x + ww +p;
            pos.y = g->y + p;
            drawFilledRect(pos, g->w - ww - (p*2) - 1, g->h-(p*2), 0);
        }
    }

}

void watchface_clear()
{
    clearDisplay();
    prevHash = 0;
}