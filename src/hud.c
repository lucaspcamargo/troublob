#include "hud.h"

#include "palette_ctrl.h"
#include "./strings/strings_en.h"
#include "sfx.h"
#include "gfx_utils.h"
#include "playfield.h"
#include "tools.h"

#define TILE_HUD_INDEX (1728) // for hud, we use spare space in window tile mappings
#define TILE_HUD_FONT_INDEX (TILE_FONT_INDEX)

#define D_BG_P 0  // "dialog background priority"
#define DIALOG_BG_CORNER    (TILE_HUD_INDEX+14)
#define DIALOG_BG_BORDER_H  (DIALOG_BG_CORNER+1)
#define DIALOG_BG_BORDER_V  (DIALOG_BG_CORNER+2)
#define DIALOG_BG_BLANK     (DIALOG_BG_CORNER+3)

static u8 hud_state = HUD_ST_UNINITIALIZED;
static u8 hud_state_timer = 0;
static bool hud_dirty = FALSE;

static const u16 HUD_DIALOG_ERROR_STR = 0;
static const u8 HUD_DIALOG_DELAY = 4;

static u16 hud_dialog_charid;
static const char * hud_dialog_str;
static u8 hud_dialog_len;
Sprite *hud_dialog_portrait = NULL;

#define HUD_INVENTORY_COUNT 10   // hud does not distinguish move tool
static enum ToolId hud_inventory[HUD_INVENTORY_COUNT];
static u16 hud_inventory_curr;
static bool hud_inventory_marker;

void _HUD_draw();
void _HUD_draw_marker();

void HUD_preinit()
{
    // set hud palette on its line
    PCTRL_set_source(PAL_LINE_HUD, pal_tset_hud.data);

    // load HUD tiles
    VDP_loadTileSet(&tset_hud, TILE_HUD_INDEX, DMA);
    VDP_loadFont(&tset_hud_font, DMA);
}


void HUD_init()
{
    HUD_inventory_clear();
    HUD_set_visible(TRUE);
    hud_state = HUD_ST_NORMAL;
    hud_inventory_marker = FALSE;
    // will draw on next update
}


void HUD_set_visible(bool visible)
{
    VDP_setWindowVPos(TRUE, visible ? HUD_Y_TILE : 28);

}
void _HUD_draw()
{
    // tiles on BG A
    VDP_setTileMapEx(WINDOW, &map_hud, TILE_ATTR_FULL(PAL_LINE_HUD, 0, 0, 0, TILE_HUD_INDEX), 0, HUD_Y_TILE, 0, 0, 40, 28-HUD_Y_TILE, DMA);

    for(int i = 0; i < HUD_INVENTORY_COUNT; i++)
    {
        u16 frame_idx;
        bool flipH = FALSE, flipV = FALSE;
        if(!TOOL_get_gfx(hud_inventory[i], &frame_idx, &flipH, &flipV))
            continue;
        GFX_draw_sprite_in_plane_2x2(WINDOW, 2+3*i, HUD_Y_TILE+1,
                                     TILE_ATTR_FULL(PAL_LINE_SPR_A, 1, flipV?1:0, flipH?1:0, PLF_theme_data_idx_table(PLF_THEME_TOOLS)[0][frame_idx]));
    }
    if (hud_inventory_marker)
        _HUD_draw_marker();
    hud_dirty = FALSE;
}

void _HUD_draw_marker()
{
    if(hud_inventory_curr >= HUD_INVENTORY_COUNT)
        return; // oob

    const u8 sx = 1 + 3*hud_inventory_curr;
    static const u8 sy = 24;
    // top row
    VDP_setTileMapXY(WINDOW, TILE_ATTR_FULL(PAL_LINE_HUD, 1, 0, 0, TILE_HUD_INDEX + 1), sx+0, sy);
    VDP_setTileMapXY(WINDOW, TILE_ATTR_FULL(PAL_LINE_HUD, 1, 0, 0, TILE_HUD_INDEX + 2), sx+1, sy);
    VDP_setTileMapXY(WINDOW, TILE_ATTR_FULL(PAL_LINE_HUD, 1, 0, 0, TILE_HUD_INDEX + 2), sx+2, sy);
    VDP_setTileMapXY(WINDOW, TILE_ATTR_FULL(PAL_LINE_HUD, 1, 0, 0, TILE_HUD_INDEX + 3), sx+3, sy);
    // left right borders
    VDP_setTileMapXY(WINDOW, TILE_ATTR_FULL(PAL_LINE_HUD, 1, 0, 0, TILE_HUD_INDEX + 8), sx+0, sy+1);
    VDP_setTileMapXY(WINDOW, TILE_ATTR_FULL(PAL_LINE_HUD, 1, 0, 0, TILE_HUD_INDEX + 8), sx+0, sy+2);
    VDP_setTileMapXY(WINDOW, TILE_ATTR_FULL(PAL_LINE_HUD, 1, 0, 0, TILE_HUD_INDEX + 9), sx+3, sy+1);
    VDP_setTileMapXY(WINDOW, TILE_ATTR_FULL(PAL_LINE_HUD, 1, 0, 0, TILE_HUD_INDEX + 9), sx+3, sy+2);
    // bottom row
    VDP_setTileMapXY(WINDOW, TILE_ATTR_FULL(PAL_LINE_HUD, 1, 0, 0, TILE_HUD_INDEX + 10), sx+0, sy+3);
    VDP_setTileMapXY(WINDOW, TILE_ATTR_FULL(PAL_LINE_HUD, 1, 0, 0, TILE_HUD_INDEX + 12), sx+1, sy+3);
    VDP_setTileMapXY(WINDOW, TILE_ATTR_FULL(PAL_LINE_HUD, 1, 0, 0, TILE_HUD_INDEX + 12), sx+2, sy+3);
    VDP_setTileMapXY(WINDOW, TILE_ATTR_FULL(PAL_LINE_HUD, 1, 0, 0, TILE_HUD_INDEX + 13), sx+3, sy+3);
}


void HUD_update()
{
    u8 prev_state = hud_state;
    switch(hud_state)
    {
        case HUD_ST_NORMAL:
        {
            bool show_marker = hud_state_timer%64 >= 32;
            if(show_marker != hud_inventory_marker)
            {
                hud_inventory_marker = show_marker;
                hud_dirty = TRUE;
            }
            if(hud_dirty)
                _HUD_draw();
            break;
        }
        case HUD_ST_DIALOG_STARTING:
        {
            static const u8 MAX_VAL = (320/8)+3-1;
            static const u8 STEP_SIZE = 4;

            u8 turn_max = (hud_state_timer*STEP_SIZE) + STEP_SIZE;
            for (u8 curr = hud_state_timer*STEP_SIZE; curr < turn_max; curr++)
            {
                for(u8 y = 0; y < 4; y++)
                {
                    s16 x = curr;// - y;
                    if(x >= 0 && x < 40 )
                    {
                        uint16_t tileval = TILE_ATTR_FULL(PAL_LINE_HUD, D_BG_P, 0, 0, DIALOG_BG_BLANK);
                        if(x == 0 || x == 39)
                        {
                            if(y == 0 || y == 3)
                                tileval = TILE_ATTR_FULL(PAL_LINE_HUD, D_BG_P, y?1:0, x?1:0, DIALOG_BG_CORNER);
                            else
                                tileval = TILE_ATTR_FULL(PAL_LINE_HUD, D_BG_P, 0, x?1:0, DIALOG_BG_BORDER_V);
                        }
                        else if(y == 0 || y == 3)
                            tileval = TILE_ATTR_FULL(PAL_LINE_HUD, D_BG_P, y?1:0, 0, DIALOG_BG_BORDER_H);

                        VDP_setTileMapXY(WINDOW, tileval, x, y + 24);
                    }
                }
                if(curr >= MAX_VAL)
                {
                    hud_state = HUD_ST_DIALOG_TYPING;
                }
            }
        }
        break;
        case HUD_ST_DIALOG_TYPING:
        {
            if(!hud_state_timer)
            {
                hud_dialog_portrait = SPR_addSprite(&spr_dweep, 4, 196, TILE_ATTR(PAL_LINE_SPR_A, 1, 0, 1));
                if(hud_dialog_portrait)
                {
                    SPR_setAnimAndFrame(hud_dialog_portrait, 0, 0);
                    SPR_setAlwaysOnTop(hud_dialog_portrait, TRUE);
                    SPR_setPriority(hud_dialog_portrait, TRUE);
                }
            }

            if (!(hud_state_timer % HUD_DIALOG_DELAY))
            {
                u8 curr_char_index = hud_state_timer / HUD_DIALOG_DELAY;
                u8 curr_char = hud_dialog_str[curr_char_index];
                if(!curr_char)
                {
                    hud_state = HUD_ST_DIALOG_IDLE;
                    break;
                }
                // TODO mark words and skip lines
                u8 xpos = 4 + curr_char_index;
                u8 ypos = 25;
                if(curr_char > 32 && curr_char < 128)
                {
                    VDP_setTileMapXY(WINDOW, TILE_ATTR_FULL(PAL_LINE_HUD, 1, 0, 0, TILE_HUD_FONT_INDEX+(curr_char - 32)), xpos, ypos);
                    SFX_play(SFX_go1 + (random()%3));
                }
            }
        }
        break;
        case HUD_ST_DIALOG_IDLE:
        {
        }
        break;
        default:
            break;
    }

    if(hud_state != prev_state)
        hud_state_timer = 0;
    else
        hud_state_timer++;
}

void HUD_dialog_start(u8 character_id, u16 string_id)
{
    if(string_id >= STR_EN_COUNT)
        string_id = HUD_DIALOG_ERROR_STR;
    hud_state = HUD_ST_DIALOG_STARTING;
    hud_state_timer = 0;
    hud_dialog_charid = character_id;
    hud_dialog_str = STR_EN_GET(string_id);
    hud_dialog_len = strlen(hud_dialog_str);
}

void HUD_dialog_end()
{
    if(hud_dialog_portrait)
    {
        SPR_setVisibility(hud_dialog_portrait, HIDDEN);
        SPR_releaseSprite(hud_dialog_portrait);
        hud_dialog_portrait = NULL;
    }
    _HUD_draw();
    hud_state = HUD_ST_NORMAL;
    hud_state_timer = 0;
}


void HUD_inventory_set(const enum ToolId * tool_arr)
{
    memcpy(hud_inventory, tool_arr, sizeof(hud_inventory));
    hud_dirty = TRUE;
}

void HUD_inventory_set_curr_idx(u8 idx)
{
    if(hud_inventory_curr != idx)
    {
        hud_inventory_curr = idx;
        hud_dirty = TRUE;
    }
}

void HUD_inventory_pop(u8 idx)
{
    for(u8 i = idx; i < HUD_INVENTORY_COUNT; i++)
    {
        hud_inventory[i] = i < HUD_INVENTORY_COUNT - 1? hud_inventory[i+1] : TOOL_NONE;
    }
    if(hud_inventory_curr == idx)
        hud_inventory_curr = 0;
    hud_dirty = TRUE;
}


void HUD_inventory_pop_curr()
{
    HUD_inventory_pop(hud_inventory_curr);
}

void HUD_inventory_clear()
{
    memset(hud_inventory, 0x00, sizeof(hud_inventory));
    hud_inventory_curr = 0;
    hud_dirty = TRUE;
}

enum ToolId HUD_inventory_curr()
{
    return hud_inventory[hud_inventory_curr];
}


void HUD_on_click(s16 x, s16 y)
{
    // check if on a tool square
    if(x >= 16 && x < 248 && y >= (HUD_Y_POS+8) && y < (HUD_Y_POS+24))
    {
        s16 tool_square_x = (x-16)%24;
        if(tool_square_x < 16)
        {
            u8 new_idx = (x-16)/24;
            if(hud_inventory[new_idx] != TOOL_NONE)
            {
                SFX_play(SFX_dull);
                hud_inventory_curr = new_idx;
                hud_dirty = TRUE;
            }
        }
    }
}
