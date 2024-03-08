#include <genesis.h>
#include "title2.h"

#include "dweep_config.h"
#include "resources.h"
#include "palette_ctrl.h"
#include "sfx.h"
#include "input.h"
#include "hud.h"


#define TITLE_FRAMES (300 + 2 * PAL_STD_FADE_DURATION)
#define TITLE_DISCLAIMER_FRAMES 200

extern const Bitmap sgdk_logo;

void _TITLE_disclaimer()
{
    const u16 txt_y = 16;

    // pure black
    VDP_clearPlane(BG_A, TRUE);
    SPR_clear();
    u16 zeroes[64];
    memset(zeroes, 0x0000, 64*2);
    PAL_setColors(0, zeroes, 64, DMA);
    SYS_doVBlankProcess();

    VDP_fillTileMapRect(BG_B, TILE_FONT_INDEX, 0, txt_y-2, 320/8, 12);
    VDP_drawTextBG(BG_B, "Dweep Genesis ", 10, txt_y);
    VDP_drawTextBG(BG_B, GAME_VERSION, 24, txt_y);
    VDP_drawTextBG(BG_B, "by Lucas Pires Camargo", 8, txt_y+2);
    VDP_drawTextBG(BG_B, "Original game by Steve Pavlina", 4, txt_y+6);
    VDP_drawTextBG(BG_B, "Original music by Michael Huang", 4, txt_y+7);

    PCTRL_set_line(PAL_LINE_BG_0, sgdk_logo.palette->data);
    u16 orig_gray = PCTRL_sample_color(PAL_LINE_HUD, 6);
    PCTRL_force_color(PAL_LINE_HUD, 6, 0x0000);
    VDP_drawBitmapEx(BG_A, &sgdk_logo, TILE_ATTR_FULL(PAL_LINE_BG_0, 0, 0, 0, TILE_USER_INDEX), 16, 4, FALSE);

    u16 framecounter = 0;
    for(int i = 0; i < TITLE_DISCLAIMER_FRAMES; i++)
    {
        SPR_update();
        PCTRL_step(framecounter);
        SYS_doVBlankProcess();
        framecounter++;
    }
    PCTRL_force_color(PAL_LINE_HUD, 6, orig_gray);
}


void _TITLE_logo()
{
    VDP_clearPlane(BG_A, TRUE);
    SPR_clear();
    u16 zeroes[64];
    memset(zeroes, 0x0000, 64*2);
    PAL_setColors(0, zeroes, 64, DMA);
    SYS_doVBlankProcess();

    VDP_clearPlane(BG_B, TRUE);
    SYS_doVBlankProcess();

    VDP_drawImageEx(BG_A, &title_cachacella, TILE_ATTR_FULL(PAL0, 0, 0, 0, TILE_USER_INDEX), 0, 0, 0, DMA);
    SYS_doVBlankProcess();

    PCTRL_set_all(title_cachacella.palette->data);
    PCTRL_fade_in(PAL_STD_FADE_DURATION);
    static const u32 logo_frames = 240;
    u32 framecounter = 0;
    while(framecounter < logo_frames + PAL_STD_FADE_DURATION*2)
    {
        if(framecounter == PAL_STD_FADE_DURATION + 10)
            SFX_play(SFX_cachacella);

        if(framecounter == PAL_STD_FADE_DURATION + logo_frames)
            PCTRL_fade_out(PAL_STD_FADE_DURATION);

        SPR_update();
        PCTRL_step(framecounter);
        SYS_doVBlankProcess();

        framecounter++;
    }

    // reset system palette
    PCTRL_set_line(0, pal_tset_hud.data);
}

void TITLE_main(const DirectorCommand *curr_cmd, DirectorCommand *next_cmd)
{
    (void) curr_cmd; // ignore

    INPUT_set_cursor_visible(FALSE);

    _TITLE_disclaimer();
    _TITLE_logo();

    // force palette to black
    u16 zeroes[64];
    memset(zeroes, 0x0000, 64*2);
    PAL_setColors(0, zeroes, 64, DMA);
    SYS_doVBlankProcess();

    // now load graphics in the peace of darkness
    u16 tilecnt = TILE_USER_INDEX;
    VDP_setPlaneSize(64, 64, TRUE);
    VDP_clearPlane(BG_A, TRUE);

    VDP_drawImageEx(BG_A, &title_dweep, TILE_ATTR_FULL(PAL1, 0, 0, 0, tilecnt), 12, -1, 0, DMA);
    tilecnt += title_dweep.tileset->numTile;
    VDP_loadTileSet(title_bg.tileset, tilecnt, DMA);
    PCTRL_set_line(PAL1, title_dweep.palette->data);

    static const s16 DELTA = 200;
    static const s16 DWEEP_Y = 128;
    static const s16 GENESIS_Y = 180;
    static const s16 DWEEP_FINAL_X = 60;
    static const s16 GENESIS_FINAL_X = 72;
    static const s16 DWEEP_INITIAL_X = DWEEP_FINAL_X + DELTA;
    static const s16 GENESIS_INITIAL_X = GENESIS_FINAL_X - DELTA;

    Sprite * spr_ltr_dweep = SPR_addSprite(&title_letters_dweep, DWEEP_INITIAL_X, DWEEP_Y, TILE_ATTR(PAL2, 1, 0, 0));
    Sprite * spr_ltr_genesis = SPR_addSprite(&title_letters_genesis, GENESIS_INITIAL_X, GENESIS_Y, TILE_ATTR(PAL3, 1, 0, 0));
    Sprite * spr_ltr_prompt = SPR_addSprite(&title_prompt, 96, 210, TILE_ATTR(PAL_LINE_HUD, 1, 0, 0));
    PCTRL_set_line(PAL2, title_letters_dweep.palette->data);
    PCTRL_set_line(PAL3, title_letters_genesis.palette->data);

    for(u8 x = 0; x <= 20; x++)
        for(u8 y = 0; y <= 16; y++)
            VDP_setTileMapEx(BG_B, title_bg.tilemap, TILE_ATTR_FULL(PAL_LINE_HUD, 0, 0, 0, tilecnt), x*2, y*2, 0, 0, 2, 2, CPU);


    XGM_startPlay(bgm_title);

    PCTRL_fade_in(PAL_STD_FADE_DURATION*3);
    u32 framecounter = 0;
    u32 press_frame = 0;
    s16 scroll_a = -DELTA;
    for(;;)
    {
        VDP_setHorizontalScroll(BG_B, -((framecounter/2)&0xf));
        VDP_setVerticalScroll(BG_B, (framecounter/2)&0xf);
        SPR_setVisibility(spr_ltr_prompt, framecounter >= 192? (framecounter%64 >= 32? HIDDEN : VISIBLE) : HIDDEN);

        s16 dx = SPR_getPositionX(spr_ltr_dweep);
        s16 gx = SPR_getPositionX(spr_ltr_genesis);
        if(dx != DWEEP_FINAL_X)
            dx -= 2;
        if(gx != GENESIS_FINAL_X)
            gx += 2;
        SPR_setPosition(spr_ltr_dweep, dx, DWEEP_Y);
        SPR_setPosition(spr_ltr_genesis, gx, GENESIS_Y);

        if(scroll_a)
            scroll_a += 2;
        VDP_setVerticalScroll(BG_A, scroll_a);

        bool im_change;
        u16 changed, state;
        INPUT_get_last_state(&im_change, &changed, &state);

        if((!press_frame) && (changed & state & (JOY_ALL & ~(BUTTON_DIR))))
        {
            press_frame = framecounter;
            PCTRL_fade_out(PAL_STD_FADE_DURATION);
        }


        SPR_update();
        PCTRL_step(framecounter);
        SYS_doVBlankProcess();

        framecounter++;

        if(press_frame && framecounter >= (press_frame + PAL_STD_FADE_DURATION + 1))
            break;
    }

    VDP_setPlaneSize(64, 32, TRUE);
    HUD_preinit(); // reload lost data packed around planes

    XGM_stopPlay();
    SPR_releaseSprite(spr_ltr_dweep);
    SPR_releaseSprite(spr_ltr_genesis);
    SPR_releaseSprite(spr_ltr_prompt);

    memset(next_cmd, 0x00, sizeof(DirectorCommand));
    next_cmd->cmd = DIREC_CMD_LEVEL;
    next_cmd->arg0 = 1; // lvl 1
}

