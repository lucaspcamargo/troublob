#pragma once

#include <genesis.h>
#include "save.h"


inline void BGM_play_loops(const u8 *data, s16 loop_count)
{
    if(!SAVE_curr_data()->opts.disable_music)
    {
        XGM_setLoopNumber(loop_count);
        XGM_startPlay(data);
    }
}

inline void BGM_play(const u8 *data)
{
    BGM_play_loops(data, -1);
}

inline void BGM_stop_all()
{
    XGM_stopPlay();
}


inline void BGM_pause()
{
    if(!SAVE_curr_data()->opts.disable_music)
        XGM_pausePlay();
}


inline void BGM_resume()
{
    if(!SAVE_curr_data()->opts.disable_music)
        XGM_resumePlay();
}
