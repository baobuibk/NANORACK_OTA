/*
 * inter_cpu_comm.h
 *
 *  Created on: Mar 1, 2025
 *      Author: CAO HIEU
 */

#ifndef M1_DRIVERS_IPC_INTER_CPU_COMM_H_
#define M1_DRIVERS_IPC_INTER_CPU_COMM_H_

#include <string.h>
#include <stdint.h>

#define MEM_ALIGN(x)                        (((x) + 0x00000003) & ~0x00000003)

typedef enum {
    IPC_CHANNEL_CM4_TO_CM7 = 0,
    IPC_CHANNEL_CM7_TO_CM4
} IPC_Channel_t;

/* Shared RAM between 2 cores is SRAM4 in D3 domain */
#define SHARED_RAM_START_ADDR              0x38000000
#define SHARED_RAM_LEN                     0x0000FFFF
#define IPC_SHARED_RAM_SIZE                (21 * 1024)

/* IPC from CM4 to CM7 */
#define IPC_CM4_TO_CM7_ADDR                MEM_ALIGN(SHARED_RAM_START_ADDR)
#define IPC_CM4_TO_CM7_LEN                 MEM_ALIGN(sizeof(BufferIPC_t))
#define IPC_DATA_CM4_TO_CM7_ADDR           MEM_ALIGN(IPC_CM4_TO_CM7_ADDR + IPC_CM4_TO_CM7_LEN)
#define IPC_DATA_CM4_TO_CM7_LEN            MEM_ALIGN(0x00001000)

/* IPC from CM7 to CM4 */
#define IPC_CM7_TO_CM4_ADDR                MEM_ALIGN(IPC_DATA_CM4_TO_CM7_ADDR + IPC_DATA_CM4_TO_CM7_LEN)
#define IPC_CM7_TO_CM4_LEN                 MEM_ALIGN(sizeof(BufferIPC_t))
#define IPC_DATA_CM7_TO_CM4_ADDR           MEM_ALIGN(IPC_CM7_TO_CM4_ADDR + IPC_CM7_TO_CM4_LEN)
#define IPC_DATA_CM7_TO_CM4_LEN            MEM_ALIGN(0x00004000)

#define IPC_VOLATILE                       volatile

typedef enum {
    IPC_EVT_READ,                          /*!< Read event */
    IPC_EVT_WRITE,                         /*!< Write event */
    IPC_EVT_RESET,                         /*!< Reset event */
} IPC_evt_type_t;

struct s_BufferIPC;

typedef void (*IPC_evt_fn)(IPC_VOLATILE struct s_BufferIPC* pBuff, IPC_evt_type_t evt, size_t bp);


typedef struct s_BufferIPC {

    uint8_t* pBuff;                              /*!< Pointer to IPCer data.
                                                    IPCer is considered initialized when `IPC != NULL` and `size > 0` */
    size_t size;                                /*!< Size of IPCer data. Size of actual IPCer is `1` byte less than value holds */
    size_t r;                                   /*!< Next read pointer. IPCer is considered empty when `r == w` and full when `w == r - 1` */
    size_t w;                                   /*!< Next write pointer. IPCer is considered empty when `r == w` and full when `w == r - 1` */
    IPC_evt_fn evt_fn;                     /*!< Pointer to event callback function */

} BufferIPC_t;

volatile BufferIPC_t* BufferIPC_GetChannel(IPC_Channel_t channel);
void BufferIPC_ClearRAM_PreInit(void);
/*************************************************
 *               	    INIT	                 *
 *************************************************/
uint8_t BufferIPC_Init(IPC_VOLATILE BufferIPC_t* buff, void* buffdata, size_t size);
void BufferIPC_SetEvtFn(IPC_VOLATILE BufferIPC_t* buff, IPC_evt_fn evt_fn);
uint8_t BufferIPC_IsReady(IPC_VOLATILE BufferIPC_t* buff);
void BufferIPC_Free(IPC_VOLATILE BufferIPC_t* buff);

/*************************************************
 *               	 READ/WRITE	                 *
 *************************************************/
size_t BufferIPC_Write(IPC_VOLATILE BufferIPC_t* buff, const void* data, size_t len);
size_t BufferIPC_Read(IPC_VOLATILE BufferIPC_t* buff, void* data, size_t len);
size_t BufferIPC_Peek(IPC_VOLATILE BufferIPC_t* buff, size_t skip_count, void* data, size_t len);

/*************************************************
 *                BufferIPC INFOR	             *
 *************************************************/
 size_t BufferIPC_GetFull(IPC_VOLATILE BufferIPC_t* buff);
 size_t BufferIPC_GetFree(IPC_VOLATILE BufferIPC_t* buff);

 /*************************************************
 *              ReadBlock Management	          *
 *************************************************/
void* BufferIPC_GetLinearBlockReadAddress(IPC_VOLATILE BufferIPC_t* buff);
size_t BufferIPC_GetLinearBlockReadLength(IPC_VOLATILE BufferIPC_t* buff);
size_t BufferIPC_Skip(IPC_VOLATILE BufferIPC_t* buff, size_t len);

 /*************************************************
 *              WriteBlock Management	          *
 *************************************************/
void* BufferIPC_GetLinearBlockWriteAddress(IPC_VOLATILE BufferIPC_t* buff);
size_t BufferIPC_GetLinearBlockWriteLength(IPC_VOLATILE BufferIPC_t* buff);
size_t BufferIPC_Advance(IPC_VOLATILE BufferIPC_t* buff, size_t len);

 /*************************************************
 *                 Frame Management	              *
 *************************************************/
uint8_t* BufferIPC_GetFrame(IPC_VOLATILE BufferIPC_t* buff, size_t* payload_len, size_t* total_frame_len);
size_t BufferIPC_WriteFrame(IPC_VOLATILE BufferIPC_t* buff, const char* data);

#endif /* M1_DRIVERS_IPC_INTER_CPU_COMM_H_ */

