#pragma once

#include <genesis.h>

#define PCTRL_PAL_COUNT 4
#define PCTRL_PAL_LEN 16
#define PCTRL_PAL_TOTAL (PCTRL_PAL_COUNT*PCTRL_PAL_LEN)
#define PCTRL_OP_MAX 4


enum PalCtrlOperation {
    PCTRL_OP_NOOP = 0,
    PCTRL_OP_CYCLE = 1
} ENUM_PACK;

typedef struct {
    u8 operation;   // what to do
    u8 idx_base;    // where to do it
    u8 len;         // how much
    u8 period_mask; // bit-anded with framecounter, if zero, execute this frame
    void *args;     // extra operator data
    void *_private; // operator has free reign over this
} PalCtrlOperatorDescriptor;

void PCTRL_op_clear_all();
void PCTRL_init();
void PCTRL_set_source(u8 line, u16* data, bool hud);
void PCTRL_step(u16 framecounter);
void PCTRL_fade_in(u16 num_fr);
void PCTRL_fade_out(u16 num_fr);
bool PCTRL_op_add(PalCtrlOperatorDescriptor *desc);
void PCTRL_op_clear_all();
