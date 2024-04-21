/*
This file is part of Dweep Genesis.

Dweep Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Dweep Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/

#include "i18n.h"
#include "./strings/strings_en.h"
#include "./strings/strings_es.h"
#include "./strings/strings_it.h"
#include "./strings/strings_pt.h"

static enum Language _i18n_curr = LANG_EN;

inline void _i18n_lang_get(enum Language lang, const char **buf, const u16 **offsets, u16 *str_count)
{
    switch(lang)
    {
        default:
        case LANG_EN:
            (*buf) = str_en_blob;
            (*offsets) = str_en_offsets;
            (*str_count) = STR_EN_COUNT;
            break;

        case LANG_ES:
            (*buf) = str_es_blob;
            (*offsets) = str_es_offsets;
            (*str_count) = STR_ES_COUNT;
            break;

        case LANG_IT:
            (*buf) = str_it_blob;
            (*offsets) = str_it_offsets;
            (*str_count) = STR_IT_COUNT;
            break;

        case LANG_PT:
            (*buf) = str_pt_blob;
            (*offsets) = str_pt_offsets;
            (*str_count) = STR_PT_COUNT;
            break;

    }
}

const char * i18n_strl(u16 id, enum Language lang)
{
    const char *buf;
    const u16 *offsets;
    u16 str_count;

    _i18n_lang_get(lang, &buf, &offsets, &str_count);
    if(id >= str_count)
        id = 0;

    return buf + offsets[id];
}

const char * i18n_str(u16 id)
{
    return i18n_strl(id, _i18n_curr);
}


enum Language i18n_lang_curr()
{
    return _i18n_curr;
}

void i18n_lang_set(enum Language lang)
{
    _i18n_curr = lang;
}
