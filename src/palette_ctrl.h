#pragma once

#include <genesis.h>
#include "pctrl_ops/iface.h"

#define PCTRL_PAL_COUNT 4
#define PCTRL_PAL_LEN 16
#define PCTRL_PAL_TOTAL (PCTRL_PAL_COUNT*PCTRL_PAL_LEN)
#define PCTRL_OP_MAX 4


enum PalCtrlOperation {
    PCTRL_OP_NOOP = 0,
    PCTRL_OP_CYCLE = 1
};

typedef struct {
    u8 operation;   // what to do
    u8 idx_base;    // where to do it
    u8 len;         // how much
    u8 period_mask; // bit-anded with framecounter, if zero, execute this frame
    void *args;     // extra operator data
    void *_private; // operator has free reign over this
} PalCtrlOperatorDescriptor;


//static u8 PCTRL_brightness = 0xff; -- For now, use PAL_fade as usual
static u16 PCTRL_src_hud_lines[PCTRL_PAL_TOTAL];
    // from failed attempt to have palette change mid-frame, for hud to use same palette lines that bg uses
    //  (we'd have 1 more palette line. That's 33.3̅% more palette overall! double for the hud!)
    // NOTE: now I know I can write, in theory, 18 bytes every h-int
    //       dont call function, use a pure-asm intrerrupt handler (or something that compiles as such?)
    //       a palette line is 32 bytes, need two scanlines for cram tx for each line
    //       eager to see cram dots in action
static u16 PCTRL_src_lines[PCTRL_PAL_TOTAL];
static u16 PCTRL_result_lines[PCTRL_PAL_TOTAL];
static PalCtrlOperatorDescriptor PCTRL_operators[PCTRL_OP_MAX];

void PCTRL_op_clear_all();

void PCTRL_init ()
{
    PAL_getColors(0, PCTRL_src_lines, PCTRL_PAL_TOTAL);
    memcpy(PCTRL_result_lines, PCTRL_src_lines, PCTRL_PAL_TOTAL*sizeof(u16));
    memcpy(PCTRL_src_hud_lines, PCTRL_src_lines, PCTRL_PAL_TOTAL*sizeof(u16));

    // bla[nc]k slate
    u16 zeroes[64];
    memset(zeroes, 0, 64);
    PAL_setColors(PAL0, zeroes, 64, DMA);

    PCTRL_op_clear_all();
}

void PCTRL_set_source(u8 line, u16* data, bool hud)
{
    memcpy((hud?PCTRL_src_hud_lines:PCTRL_src_lines)+(line*PCTRL_PAL_LEN), data, PCTRL_PAL_LEN*sizeof(u16));
}

void PCTRL_step(u16 framecounter)
{
    if(PAL_isDoingFade())
        return;

    for(u8 i = 0; i < PCTRL_PAL_TOTAL; i++)
    {
        PalCtrlOperatorDescriptor *curr = PCTRL_operators + i;
        if(!curr->operation || !curr->period_mask)
            continue;

        if(framecounter & curr->period_mask)
            continue;

        switch(PCTRL_operators[i].operation)
        {
            case PCTRL_OP_CYCLE:
            {
                u16 tmp = PCTRL_src_lines[PCTRL_operators[i].idx_base];
                for(u8 j = 0; j < PCTRL_operators[i].len; j++)
                    PCTRL_src_lines[PCTRL_operators[i].idx_base+j] =
                        (j == PCTRL_operators[i].len-1)?
                            tmp:
                            PCTRL_src_lines[PCTRL_operators[i].idx_base+j+1];
                break;
            }
            default:
                break;
        }
    }


    PAL_setColors(0, PCTRL_src_lines, PCTRL_PAL_TOTAL, DMA_QUEUE);
}

inline void PCTRL_hud_h_int() // failed attempt to have palette change mid-frame
{
    //PAL_setColors(0, PCTRL_src_hud_lines, PCTRL_PAL_TOTAL, CPU);
}

inline void PCTRL_hud_end_h_int() // failed attempt to have palette change mid-frame
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

bool PCTRL_op_add(PalCtrlOperatorDescriptor *desc)
{
    if(!desc->operation)
        return FALSE;

    for(int i = 0; i < PCTRL_OP_MAX; i++)
    {
        if(PCTRL_operators[i].operation == PCTRL_OP_NOOP)
        {
            memcpy(PCTRL_operators+i, desc, sizeof(PalCtrlOperatorDescriptor));
            return TRUE;
        };
    }
    return FALSE;
}

void PCTRL_op_clear_all()
{
    for(int i = 0; i < PCTRL_OP_MAX; i++)
        PCTRL_operators[i].operation = PCTRL_OP_NOOP;
}
