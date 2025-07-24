#ifndef MODFSP_H
#define MODFSP_H

#include "stdint.h"

#define PKT_MIN_LENGTH              9
#define PKT_MAX_LENGTH              65539
#define DATA_MAX_LENGTH             5120

#define MODFSP_TIMEOUT_MS    		300U
/*
 [SOF1] [SOF2] [ID] [Length] [Data] [Crc16 low] [Crc16 high] [EOF1] [EOF2]
 */

typedef struct {
    uint16_t crc; /*!< Current CRC value */
} MODFSP_CRC_t;

#define SFP_START1_BYTE     0xC0U
#define SFP_START2_BYTE     0xDEU

#define SFP_STOP1_BYTE      0xDAU
#define SFP_STOP2_BYTE      0xEDU

typedef enum {
    SFP_DECODE_START1 = 0U,
    SFP_DECODE_START2,
    SFP_DECODE_ID,          
    SFP_DECODE_LEN_LOW,
    SFP_DECODE_LEN_HIGH,
    SFP_DECODE_DATA,     
    SFP_DECODE_CRC,       
    SFP_DECODE_STOP1,   
    SFP_DECODE_STOP2,   
    SFP_DECODE_END, 
} MODFSP_DecodeState_t;

typedef struct
{
    MODFSP_DecodeState_t state;
    uint8_t         id;        
    uint16_t        length;                //length of data
    uint8_t         data[DATA_MAX_LENGTH];   
    uint16_t        index;
    MODFSP_CRC_t    crc16;                 
    uint16_t        crc16_data;      
    uint32_t 		last_rx_time;
} MODFSP_Data_t;    

typedef enum {
    MODFSP_OK = 0x00, /*!< Function returns successfully */
    MODFSP_ERR,       /*!< General error for function status */
    MODFSP_INPROG,    /*!< Receive is in progress */
    MODFSP_VALID,     /*!< packet valid and ready to be read as CRC is valid and STOP received */
    MODFSP_ERRCRC,    /*!< CRC integrity error for the packet. Will not wait STOP byte if received */
    MODFSP_ERRSTOP,   /*!< Packet error with STOP byte, wrong character received for STOP */
    MODFSP_WAITDATA,  /*!< Packet state is in start mode, waiting start byte to start receiving */
    MODFSP_ERRMEM,    /*!< No enough memory available for write */
    MODFSP_ERRTIMEOUT
} MODFSP_Return_t;

// Command handler function pointer type
typedef void (*MODFSP_Handler_t)(MODFSP_Data_t *ctx, const uint8_t *payload, uint16_t len);

// Command structure
typedef struct {
    uint8_t id;
    MODFSP_Handler_t handler;
} MODFSP_Command_t;

// Function prototypes
MODFSP_Return_t MODFSP_Init(MODFSP_Data_t *this);
void MODFSP_Reset(MODFSP_Data_t *this);
MODFSP_Return_t MODFSP_Read(MODFSP_Data_t *this, const uint8_t *rx_data);
MODFSP_Return_t MODFSP_Send(MODFSP_Data_t *this, uint8_t id, const void* data, uint16_t len);
MODFSP_Return_t MODFSP_Process(MODFSP_Data_t *this);

// Command table management
MODFSP_Return_t MODFSP_ProcessGlobal(void);
MODFSP_Data_t *MODFSP_GetGlobalInstance(void);

void MODFSP_RegisterCommand(uint8_t id, MODFSP_Handler_t handler);
const MODFSP_Command_t *MODFSP_GetCommandTable(void);
uint8_t MODFSP_GetCommandTableSize(void);

// Application callback (to be implemented by user)
void MODFSP_ApplicationHandler(MODFSP_Data_t *ctx, uint8_t id, const uint8_t *payload, uint16_t len);

#endif // MODFSP_H
