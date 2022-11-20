# dweep-gen SGDK rescomp
IMAGE title "title.png" 0

# 
# SPRITES_CORE
#
SPRITE spr_dweep ../art/spr/dweep.png 2 2 2

#
#  TILESETS
#
TILESET tset_hud ../art/spr/tilesetH.tsx AUTO ALL
PALETTE pal_tset_hud ../art/spr/tilesetH.png
TILESET tset_0 ../art/spr/tileset0.tsx AUTO ALL
PALETTE pal_tset_0 ../art/spr/tileset0.png


#
#  MAPS
#
TILEMAP map_hud ../art/map/map_H.tmx SINGLE AUTO AUTO
MAP map_1_a ../art/map/map_1.tmx A AUTO AUTO
MAP map_1_b ../art/map/map_1.tmx B AUTO AUTO
OBJECTS map_1_o ../art/map/map_1.tmx OBJ x:u16;y:u16
 
#
#  MUSIC
#  Huge thanks to libOPNMIDI and its midi2vgm tool!
#
XGM bgm_stage_1 "vgm/conv_stage_1.vgm" AUTO
XGM bgm_stage_2 "vgm/conv_stage_2.vgm" AUTO
XGM bgm_stage_3 "vgm/conv_stage_3.vgm" AUTO
XGM bgm_stage_4 "vgm/conv_stage_4.vgm" AUTO
XGM bgm_stage_5 "vgm/conv_stage_5.vgm" AUTO
XGM bgm_victory "vgm/conv_victory.vgm" AUTO
XGM bgm_defeat "vgm/conv_defeat.vgm" AUTO
 
#
#  SOUND EFEFCTS
#
WAV  sfx_boom   sfx/boom.wav    XGM 0
WAV  sfx_burn   sfx/burn.wav    XGM 0
WAV  sfx_crush  sfx/crush.wav   XGM 0
WAV  sfx_ding   sfx/ding.wav    XGM 0
WAV  sfx_dry    sfx/dry.wav     XGM 0
WAV  sfx_dull   sfx/dull.wav    XGM 0
WAV  sfx_float  sfx/float.wav   XGM 0
WAV  sfx_freeze sfx/freeze.wav  XGM 0
WAV  sfx_fuse   sfx/fuse.wav    XGM 0
WAV  sfx_glass  sfx/glass.wav   XGM 0
WAV  sfx_go1    sfx/go1.wav     XGM 0
WAV  sfx_go3    sfx/go3.wav     XGM 0
WAV  sfx_go4    sfx/go4.wav     XGM 0
WAV  sfx_mop    sfx/mop.wav     XGM 0
WAV  sfx_no     sfx/no.wav      XGM 0
WAV  sfx_short  sfx/short.wav   XGM 0
WAV  sfx_thaw   sfx/thaw.wav    XGM 0
WAV  sfx_water  sfx/water.wav   XGM 0
WAV  sfx_wrench sfx/wrench.wav  XGM 0
