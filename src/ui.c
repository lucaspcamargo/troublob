/*
This file is part of Blob Genesis.

Blob Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Blob Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/

#include "ui.h"




typedef struct UIContext_st
{
    bool intialized;
    Pool *widgetPool;
} UIContext_d;


void ui_init(UIContext *ctx)
{
    UIContext_d *c = (UIContext_d*) ctx;

}
void ui_destroy(UIContext *ctx);