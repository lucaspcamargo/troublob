#include "palette_ctrl.h"

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
    PAL_setColors(PAL0, zeroes, 64, DMA_QUEUE);
    SYS_doVBlankProcess();

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
        if(!curr->operation)
            continue;

        if(framecounter & curr->period_mask)
            continue;

        u16 tmp;
        u8 j;
        switch(curr->operation)
        {
            case PCTRL_OP_CYCLE:
                tmp = PCTRL_src_lines[curr->idx_base];
                for(j = 0; j < (curr->len-1); j++)
                    PCTRL_src_lines[curr->idx_base+j] =
                        PCTRL_src_lines[curr->idx_base+j+1];
                PCTRL_src_lines[curr->idx_base+curr->len-1] = tmp;
                break;
            default:
                break;
        }
    }


    PAL_setColors(0, PCTRL_src_lines, PCTRL_PAL_TOTAL, DMA_QUEUE);
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
