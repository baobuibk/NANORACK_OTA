/************************************************
 *  @file     : shared_reg.h
 *  @date     : May 14, 2025
 *  @author   : CAO HIEU
 *-----------------------------------------------
 *  Description :
 *    [-]
 ************************************************/

#ifndef M1_DRIVERS_SHARED_REG_SHARED_REG_H_
#define M1_DRIVERS_SHARED_REG_SHARED_REG_H_

#include <stdint.h>
#define SHARED_BASE  (0x3800F800UL)

typedef struct {
    volatile uint8_t M7_to_M4[1024];   /* offset 0x000 */
    volatile uint8_t M4_to_M7[1024];   /* offset 0x400 */
} SharedBuf_t;

#define SHARED   (*(SharedBuf_t *)SHARED_BASE)

typedef enum {
    DIR_M7_TO_M4,
    DIR_M4_TO_M7
} SharedDir_t;

#define SHARED_SLOT_MAX   1023u

#ifdef CORE_CM7
    #define SHARED_IS_WRITER(dir)   ((dir) == DIR_M7_TO_M4)
#else /* CORE_CM4 */
    #define SHARED_IS_WRITER(dir)   ((dir) == DIR_M4_TO_M7)
#endif

void SharedREG_Write(SharedDir_t dir, uint16_t slot, uint8_t data);
uint8_t SharedREG_Read(SharedDir_t dir, uint16_t slot);
void SharedREG_Clear(SharedDir_t dir);
void SharedREG_Init(SharedDir_t dir);

#endif /* M1_DRIVERS_SHARED_REG_SHARED_REG_H_ */
