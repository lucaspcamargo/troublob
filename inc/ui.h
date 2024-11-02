#pragma once

/*
This file is part of Blob Genesis.

Blob Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Blob Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/

#include "game_config.h"
#include <genesis.h>


typedef void (*UIWidgetCallback)();


enum UIWidgetFlags
{
    UIW_F_HAS_GRAPHICS = 0x01,
    UIW_F_IS_DYNAMIC = 0x02
} ENUM_PACK;


enum UIWidgetEventType
{
    UIW_EVT_CREATED,
    UIW_EVT_DESTROYED,
    UIW_EVT_GEOMETRY_CHANGED,
    UIW_EVT_FOCUSED,
    UIW_EVT_MOUSE,
    UIW_EVT_JOY_PRESS
} ENUM_PACK;


typedef struct UIWidget_st
{
    u8 x, y, w, h;
    enum UIWidgetFlags flags;
    UIWidgetCallback cb;        // the callback that implements widget functionality
    void *w_data;               // used by main callback as data pointer
    void *w_cb;                 // used by main callback as callback pointer for event handlers
} UIWidget;


typedef void * UIContext;


void ui_init(UIContext *ctx);
void ui_destroy(UIContext *ctx);

enum VDPPlane ui_render_plane_get(UIContext *ctx);
void ui_render_plane_set(UIContext *ctx, enum VDPPlane);

UIWidget * ui_widget_create(UIContext *ctx, UIWidget *parent, UIWidgetCallback cb);
void ui_widget_destroy(UIContext *ctx, UIWidgetCallback cb);

void ui_widget_add_static(UIWidget *static_tree);



