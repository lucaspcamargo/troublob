#pragma once

#include "dweep_config.h"
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
} PalCtrlOperatorDescriptor;

void PCTRL_op_clear_all();
void PCTRL_init();
void PCTRL_set_line(u8 line, const u16* data);
void PCTRL_set_all(const u16* data);
void PCTRL_force_color(u8 line, u8 index, u16 data);
u16 PCTRL_sample_color(u8 line, u8 index);
void PCTRL_step(u16 framecounter);
void PCTRL_fade_in(u16 num_fr);
void PCTRL_fade_out(u16 num_fr);
bool PCTRL_is_fading();
bool PCTRL_is_dark();
bool PCTRL_op_add(const PalCtrlOperatorDescriptor *desc);
void PCTRL_op_clear_all();
