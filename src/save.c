/*
This file is part of Blob Genesis.

Blob Genesis is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Blob Genesis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Foobar. If not, see <https://www.gnu.org/licenses/>.
*/

#include "save.h"
#include <string.h>


static const u16 SAVE_DATA_SIZE = sizeof(SaveData);
static const u16 SAVE_DATA_SIZE_NO_CHECKSUM = (sizeof(SaveData)-2);
static const u16 SAVE_DATA_SIZE_NO_CHECKSUM_W = (sizeof(SaveData)/2-1);


#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))


static SaveData _save_buf;
static u8 _save_curr = 0xff;
static enum SaveSlotState _save_slot_states[SAVE_NUM_SLOTS];
static enum SaveInitStatus _save_init_status = SAVE_INIT_UNINITIALIZED;


// adapted from https://stackoverflow.com/questions/10564491/function-to-calculate-a-crc16-checksum
u16 _SAVE_crc16(const u16 *data, u16 sz)
{
    u16 out = 0;
    u8 bits_read = 0;
    u16 bit_flag;

    if(data == NULL)
        return 0;

    while(sz > 0)
    {
        bit_flag = out >> 15;
        out <<= 1;
        out |= (*data >> bits_read) & 1;
        bits_read++;
        if(bits_read > 7)
        {
            bits_read = 0;
            data++;
            sz--;
        }
        if(bit_flag)
            out ^= SAVE_CHECKSUM_POLY;
    }

    s32 i;
    for (i = 0; i < 16; ++i) {
        bit_flag = out >> 15;
        out <<= 1;
        if(bit_flag)
            out ^= SAVE_CHECKSUM_POLY;
    }

    u16 crc = 0;
    i = 0x8000;
    s32 j = 0x0001;
    for (; i != 0; i >>=1, j <<= 1) {
        if (i & out) crc |= j;
    }

    return crc;
}

void _SAVE_sram_read(u8 * dst, u16 offset, u16 size)
{
    SRAM_enableRO();
    for(u16 i = 0; i < size; i++)
        dst[i] = SRAM_readByte(offset + i);
    SRAM_disable();
}

void _SAVE_sram_write(u8 * src, u16 offset, u16 size)
{
    SRAM_enable();
    for(u16 i = 0; i < size; i++)
        SRAM_writeByte(offset + i, src[i]);
    SRAM_disable();
}

u16 _SAVE_sram_offset(u8 slot_idx)
{
    return slot_idx * SAVE_DATA_SIZE;
}

bool _SAVE_is_zeroes(u8 *buf, u16 size)
{
    while(size--) if(*buf++) return FALSE;
    return TRUE;
}


enum SaveInitStatus SAVE_init()
{
    BUILD_BUG_ON( SAVE_DATA_SIZE % 2 );

    memset(&_save_buf, 0x00, SAVE_DATA_SIZE);
    _save_init_status = SAVE_INIT_OK;
    SaveData buf;
    u8 * const buf_ptr = (u8*) &buf;


    // recover last save slot number
    SRAM_enableRO();
    _save_curr = SRAM_readWord(_SAVE_sram_offset(SAVE_NUM_SLOTS));
    SRAM_disable();
    if(_save_curr >= SAVE_NUM_SLOTS)
        _save_curr = 0;

    // check all slots
    bool all_new = TRUE;
    for(u8 slot = 0; slot < SAVE_NUM_SLOTS; slot++)
    {
        _SAVE_sram_read(buf_ptr, _SAVE_sram_offset(slot), SAVE_DATA_SIZE);

        if(_save_curr == slot)
            memcpy(&_save_buf, buf_ptr, SAVE_DATA_SIZE);

        if(_SAVE_is_zeroes(buf_ptr, SAVE_DATA_SIZE))
            _save_slot_states[slot] = SAVE_SLOT_NEW;
        else
        {
            all_new = FALSE;
            u16 crc = _SAVE_crc16((u16*)buf_ptr, SAVE_DATA_SIZE_NO_CHECKSUM_W);
            if(crc!=buf.checksum)
            {
                _save_slot_states[slot] = SAVE_SLOT_CORRUPTED;
                _save_init_status = SAVE_INIT_CORRUPTED;
            }
            else
                _save_slot_states[slot] = SAVE_SLOT_USED;
        }
    }

    if(all_new)
        _save_init_status = SAVE_INIT_ALL_NEW;

    return _save_init_status;
}

enum SaveInitStatus SAVE_init_status()
{
    return _save_init_status;
}

bool SAVE_is_initialized()
{
    return _save_init_status != SAVE_INIT_UNINITIALIZED && _save_curr != 0xff;
}

void SAVE_commit()
{
    u16 new_crc = _SAVE_crc16((u16*) &_save_buf, SAVE_DATA_SIZE_NO_CHECKSUM);
    _save_buf.checksum = new_crc;
    SAVE_slot_write(&_save_buf, _save_curr);
    _save_slot_states[_save_curr] = SAVE_SLOT_USED;
}

void SAVE_slot_select(u8 slot_idx)
{
    if(slot_idx == _save_curr)
        return;

    if(slot_idx >= SAVE_NUM_SLOTS)
        return;

    if(_save_slot_states[slot_idx] == SAVE_SLOT_USED)
        SAVE_slot_read(&_save_buf, slot_idx);
    else
        memset(&_save_buf, 0x00, SAVE_DATA_SIZE);
    _save_curr = slot_idx;
}

enum SaveSlotState SAVE_slot_state(u8 slot_idx)
{
    return slot_idx >= SAVE_NUM_SLOTS? SAVE_SLOT_INVALID : _save_slot_states[slot_idx];
}

void SAVE_slot_read(SaveData *dst, u8 slot_idx)
{
    _SAVE_sram_read((u8*)dst, _SAVE_sram_offset(slot_idx), sizeof(SaveData));
}

void SAVE_slot_write(SaveData *src, u8 slot_idx)
{
    _SAVE_sram_write((u8*)src, _SAVE_sram_offset(slot_idx), sizeof(SaveData));
}

u8 SAVE_curr_slot()
{
    return _save_curr;
}

SaveData * SAVE_curr_data()
{
    return &_save_buf;
}


const char * SAVE_enum_init_status_str(enum SaveInitStatus st)
{
    switch(st)
    {
        case SAVE_INIT_OK:
            return "NEW";
        case SAVE_INIT_CORRUPTED:
            return "CORRUPTED";
        case SAVE_INIT_ALL_NEW:
            return "ALL_NEW";
        case SAVE_INIT_UNINITIALIZED:
            return "UNINITIALIZED";
        default:
            return "UNKNOWN";
    }
}

const char * SAVE_enum_slot_state_str(enum SaveSlotState st)
{
    switch(st)
    {
        case SAVE_SLOT_NEW:
            return "NEW";
        case SAVE_SLOT_USED:
            return "USED";
        case SAVE_SLOT_CORRUPTED:
            return "CORRUPTED";
        case SAVE_SLOT_INVALID:
            return "INVALID";
        default:
            return "UNKNOWN";
    }
}

