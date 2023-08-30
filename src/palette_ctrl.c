#include "palette_ctrl.h"

//static u8 PCTRL_brightness = 0xff; -- For now, use PAL_fade as usual

enum PalCtrlFadeState
{
    PCTRL_FADE_NONE,
    PCTRL_FADE_IN,
    PCTRL_FADE_OUT
} ENUM_PACK;

#define PCTRL_FADE_DIV_SHIFT 10
#define PCTRL_FADE_RESOLUTION (1 << PCTRL_FADE_DIV_SHIFT)
#define PCTRL_FADE_MASK ((1 << PCTRL_FADE_DIV_SHIFT) - 1)
#define PCTRL_APPLY_LUM( src, lum ) (((lum + PCTRL_rounding_term_lookup[src]) * src) >> PCTRL_FADE_DIV_SHIFT)

static u16 PCTRL_src_lines[PCTRL_PAL_TOTAL];
static u16 PCTRL_result_lines[PCTRL_PAL_TOTAL];
static PalCtrlOperatorDescriptor PCTRL_operators[PCTRL_OP_MAX];
static enum PalCtrlFadeState PCTRL_fade_state;
static u16 PCTRL_fade_timer;
static u16 PCTRL_fade_duration;
static bool PCTRL_fade_blackout;
static const u16 PCTRL_rounding_term_lookup[8] = {
    0,
    PCTRL_FADE_RESOLUTION/2,
    PCTRL_FADE_RESOLUTION/4,
    PCTRL_FADE_RESOLUTION/6,
    PCTRL_FADE_RESOLUTION/8,
    PCTRL_FADE_RESOLUTION/10,
    PCTRL_FADE_RESOLUTION/12,
    PCTRL_FADE_RESOLUTION/14 };


// TODO remove this after fix in SGDK
FORCE_INLINE s16 _fix16ToRoundedInt(fix16 value)
{
    return fix16ToInt(value + (FIX16(0.5)-1));
}


void PCTRL_init ()
{
    PAL_getColors(0, PCTRL_src_lines, PCTRL_PAL_TOTAL);
    memcpy(PCTRL_result_lines, PCTRL_src_lines, PCTRL_PAL_TOTAL*sizeof(u16));

    PCTRL_op_clear_all();
}

void PCTRL_set_source(u8 line, const u16* data)
{
    memcpy(PCTRL_src_lines+(line*PCTRL_PAL_LEN), data, PCTRL_PAL_LEN*sizeof(u16));
}

void PCTRL_step(u16 framecounter)
{
    if(PAL_isDoingFade())
        return;

    for(u8 i = 0; i < PCTRL_OP_MAX; i++)
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

    if(PCTRL_fade_blackout)
    {
        PAL_setColors(0, PCTRL_result_lines, PCTRL_PAL_TOTAL, DMA_QUEUE); // use black result directly
    }
    else if(PCTRL_fade_state == PCTRL_FADE_NONE)
    {
        PAL_setColors(0, PCTRL_src_lines, PCTRL_PAL_TOTAL, DMA_QUEUE);  // use src directly
    }
    else
    {
        // prepare destination buffer
        u16 track = (PCTRL_fade_state == PCTRL_FADE_IN)? PCTRL_fade_timer : (PCTRL_fade_duration - PCTRL_fade_timer);
        u16 lum = ((u32) track) * ((u32) PCTRL_FADE_RESOLUTION) / ((u32)PCTRL_fade_duration);
        u16 lumR = min(lum + (lum >> 2), PCTRL_FADE_MASK);
        u16 lumG = lum;
        u16 lumB = min(lum << 1, PCTRL_FADE_MASK);
        // char buf[5]; -- lefover debug code
        // sprintf(buf, "%d", lum);
        // VDP_drawTextBG(WINDOW, buf, 0, 25);
        for(int i = 0; i < PCTRL_PAL_LEN*4; i++)
        {
            //u8 line = i % 16; // maybe use this and some kind of mask to allow fading in only certain lines
            u16 src = PCTRL_src_lines[i];
            u8 src_r = (src & VDPPALETTE_REDMASK) >> VDPPALETTE_REDSFT;
            u8 src_g = (src & VDPPALETTE_GREENMASK) >> VDPPALETTE_GREENSFT;
            u8 src_b = (src & VDPPALETTE_BLUEMASK) >> VDPPALETTE_BLUESFT;
            u8 dst_r = PCTRL_APPLY_LUM( src_r, lumR );
            u8 dst_g = PCTRL_APPLY_LUM( src_g, lumG );
            u8 dst_b = PCTRL_APPLY_LUM( src_b, lumB );
            PCTRL_result_lines[i] = dst_r << VDPPALETTE_REDSFT   |
                                    dst_g << VDPPALETTE_GREENSFT |
                                    dst_b << VDPPALETTE_BLUESFT;
        }

        PAL_setColors(0, PCTRL_result_lines, PCTRL_PAL_TOTAL, DMA_QUEUE);

        PCTRL_fade_timer++;
        if(PCTRL_fade_timer == PCTRL_fade_duration)
        {
            if(PCTRL_fade_state == PCTRL_FADE_OUT)
            {
                memset(PCTRL_result_lines, 0, PCTRL_PAL_TOTAL*sizeof(u16)); // result locked to black
                PCTRL_fade_blackout = TRUE;
            }
            PCTRL_fade_state = PCTRL_FADE_NONE;
        }
    }
}

void PCTRL_fade_in(u16 num_fr)
{
    if(num_fr)
    {
        PCTRL_fade_duration = num_fr;
        PCTRL_fade_timer = 0;
        PCTRL_fade_state = PCTRL_FADE_IN;
        PCTRL_fade_blackout = FALSE;
    }
    else
    {
        PCTRL_fade_state = PCTRL_FADE_NONE;
        PCTRL_fade_blackout = FALSE;
    }
}

void PCTRL_fade_out(u16 num_fr)
{
    if(PCTRL_fade_blackout)
        return;
    if(num_fr)
    {
        PCTRL_fade_duration = num_fr;
        PCTRL_fade_timer = 0;
        PCTRL_fade_state = PCTRL_FADE_OUT;
    }
    else
    {
        PCTRL_fade_state = PCTRL_FADE_NONE;
        PCTRL_fade_blackout = TRUE;
        memset(PCTRL_result_lines, 0, PCTRL_PAL_TOTAL*sizeof(u16));
    }
}

bool PCTRL_is_fading()
{
    return PCTRL_fade_state != PCTRL_FADE_NONE;
}

bool PCTRL_is_dark()
{
    return PCTRL_fade_blackout;
}

bool PCTRL_op_add(const PalCtrlOperatorDescriptor *desc)
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
