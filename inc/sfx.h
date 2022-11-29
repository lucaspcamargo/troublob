#pragma once

#include <genesis.h>
#include "dweep_config.h"
#include "resources.h"

enum SfxId {
    SFX_boom = SFX_USER_OFFSET,
    SFX_burn,
    SFX_crush,
    SFX_ding,
    SFX_dry,
    SFX_dull,
    SFX_float,
    SFX_freeze,
    SFX_fuse,
    SFX_glass,
    SFX_go1,
    SFX_go3,
    SFX_go4,
    SFX_mop,
    SFX_no,
    SFX_short,
    SFX_thaw,
    SFX_water,
    SFX_wrench,
    SFX_END
};

#if !DISABLE_SFX
static const u8 * SFX_REF[] = {
    sfx_boom,
    sfx_burn,
    sfx_crush,
    sfx_ding,
    sfx_dry,
    sfx_dull,
    sfx_float,
    sfx_freeze,
    sfx_fuse,
    sfx_glass,
    sfx_go1,
    sfx_go3,
    sfx_go4,
    sfx_mop,
    sfx_no,
    sfx_short,
    sfx_thaw,
    sfx_water,
    sfx_wrench
};

static const uint32_t SFX_SZ[] = {
    sizeof(sfx_boom),
    sizeof(sfx_burn),
    sizeof(sfx_crush),
    sizeof(sfx_ding),
    sizeof(sfx_dry),
    sizeof(sfx_dull),
    sizeof(sfx_float),
    sizeof(sfx_freeze),
    sizeof(sfx_fuse),
    sizeof(sfx_glass),
    sizeof(sfx_go1),
    sizeof(sfx_go3),
    sizeof(sfx_go4),
    sizeof(sfx_mop),
    sizeof(sfx_no),
    sizeof(sfx_short),
    sizeof(sfx_thaw),
    sizeof(sfx_water),
    sizeof(sfx_wrench)
};

static const u16 SFX_CH[] = {
    SOUND_PCM_CH2, //sfx_boom),
    SOUND_PCM_CH3, //sfx_burn),
    SOUND_PCM_CH1, //sfx_crush),
    SOUND_PCM_CH1, //sfx_ding),
    SOUND_PCM_CH1, //sfx_dry),
    SOUND_PCM_CH1, //sfx_dull),
    SOUND_PCM_CH1, //sfx_float),
    SOUND_PCM_CH1, //sfx_freeze),
    SOUND_PCM_CH1, //sfx_fuse),
    SOUND_PCM_CH1, //sfx_glass),
    SOUND_PCM_CH3, //sfx_go1),
    SOUND_PCM_CH3, //sfx_go3),
    SOUND_PCM_CH3, //sfx_go4),
    SOUND_PCM_CH1, //sfx_mop),
    SOUND_PCM_CH1, //sfx_no),
    SOUND_PCM_CH1, //sfx_short),
    SOUND_PCM_CH1, //sfx_thaw),
    SOUND_PCM_CH1, //sfx_water),
    SOUND_PCM_CH1  //sfx_wrench)
};
#endif

void SFX_register_all()
{
#if !DISABLE_SFX
    for(u8 i = SFX_USER_OFFSET; i < SFX_END; i++)
        XGM_setPCM(i, SFX_REF[i-SFX_USER_OFFSET], SFX_SZ[i-SFX_USER_OFFSET]);
#endif
}

#define SFX_play(ID) SFX_play_prio((ID),0)

void SFX_play_prio(u8 id, u8 prio)
{
#if !DISABLE_SFX
    XGM_startPlayPCM(id, prio, SFX_CH[id-SFX_USER_OFFSET]);
#endif
}
