#pragma once

/*
This file is part of Dweep Genesis.

Dweep Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Dweep Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/

#include "dweep_config.h"
#include <genesis.h>

enum Language
{
    LANG_EN,
    LANG_ES,
    LANG_IT,
    LANG_PT,
    LANG_COUNT,
    LANG_CURR
} ENUM_PACK;

enum StringID
{
    STR_ZERO,
    STR_DEMO,
    STR_LANG_NAME,
    STR_BLK_LVLNAME_BEGIN,
    STR_BLK_LVLHINT_BEGIN = STR_BLK_LVLNAME_BEGIN + 11,
    STR_COUNT
} ENUM_PACK;

const char * i18n_strl(u16 id, enum Language lang);
const char * i18n_str(u16 id);

enum Language i18n_lang_curr();
void i18n_lang_set(enum Language lang);
