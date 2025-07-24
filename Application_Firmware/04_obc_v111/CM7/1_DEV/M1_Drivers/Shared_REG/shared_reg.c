/************************************************
 *  @file     : shared_reg.c
 *  @date     : May 14, 2025
 *  @author   : CAO HIEU
 *-----------------------------------------------
 *  Description :
 *    [-]
 ************************************************/

#include "shared_reg.h"
#include <string.h>

void SharedREG_Write(SharedDir_t dir, uint16_t slot, uint8_t data)
{
    if (slot > SHARED_SLOT_MAX) return;
    if (!SHARED_IS_WRITER(dir)) return;

    if (dir == DIR_M7_TO_M4)
        SHARED.M7_to_M4[slot] = data;
    else
        SHARED.M4_to_M7[slot] = data;
}

uint8_t SharedREG_Read(SharedDir_t dir, uint16_t slot)
{
    if (slot > SHARED_SLOT_MAX) return 0;

    return (dir == DIR_M7_TO_M4) ?
            SHARED.M7_to_M4[slot] :
            SHARED.M4_to_M7[slot];
}

void SharedREG_Clear(SharedDir_t dir)
{
    if (!SHARED_IS_WRITER(dir)) return;

    if (dir == DIR_M7_TO_M4)
        memset((void *)SHARED.M7_to_M4, 0, sizeof(SHARED.M7_to_M4));
    else
        memset((void *)SHARED.M4_to_M7, 0, sizeof(SHARED.M4_to_M7));
}

void SharedREG_Init(SharedDir_t dir)
{
    if (!SHARED_IS_WRITER(dir)) return;

    SharedREG_Clear(dir);
//    if (dir == DIR_M4_TO_M7)
//    {
//        SharedREG_Write(DIR_M4_TO_M7, 0, 0);
//    }
}
