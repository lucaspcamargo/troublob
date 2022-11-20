#pragma once

#include <genesis.h>

#define PCTRL_PAL_COUNT 4
#define PCTRL_PAL_LEN 16
#define PCTRL_PAL_TOTAL (PCTRL_PAL_COUNT*PCTRL_PAL_LEN)


static u8 PCTRL_brightness = 0xff;
static u16 PCTRL_src_hud_lines[PCTRL_PAL_TOTAL];
static u16 PCTRL_src_lines[PCTRL_PAL_TOTAL];
static u16 PCTRL_result_lines[PCTRL_PAL_TOTAL];

void PCTRL_init ()
{
    PCTRL_brightness = 0xff;
    PAL_getColors(0, PCTRL_src_lines, PCTRL_PAL_TOTAL);
    memcpy(PCTRL_result_lines, PCTRL_src_lines, PCTRL_PAL_TOTAL*sizeof(u16));
    memcpy(PCTRL_src_hud_lines, PCTRL_src_lines, PCTRL_PAL_TOTAL*sizeof(u16));
}

void PCTRL_set_source(u8 line, u16* data, bool hud)
{
    memcpy((hud?PCTRL_src_hud_lines:PCTRL_src_lines)+(line*PCTRL_PAL_LEN), data, PCTRL_PAL_LEN*sizeof(u16));
}

void PCTRL_step()
{
    PAL_setColors(0, PCTRL_src_lines, PCTRL_PAL_TOTAL, CPU);
}

inline void PCTRL_hud_h_int()
{
    //PAL_setColors(0, PCTRL_src_hud_lines, PCTRL_PAL_TOTAL, CPU);
}

inline void PCTRL_hud_end_h_int()
{
    //PAL_setColors(0, PCTRL_src_lines, PCTRL_PAL_TOTAL, CPU);
}

void PCTRL_fade_in(u16 num_fr)
{
    for(u16 i = 0; i < PCTRL_PAL_TOTAL; i++)
        PAL_setColor(i, 0);
    PAL_fadeInAll(PCTRL_src_lines, num_fr, TRUE);
}

void PCTRL_fade_out(u16 num_fr)
{
    PAL_fadeOutAll(num_fr, TRUE);
}
