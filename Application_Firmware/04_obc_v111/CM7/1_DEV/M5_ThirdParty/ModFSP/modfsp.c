#include "modfsp.h"
#include "modfsp_port.h"
#include "string.h"

#include "stdio.h"

#define CRC16_XMODEM_POLY  0x1021
#define CRC16_XMODEM_INIT  0x0000
#define MAX_COMMANDS       32

// Static command table
static MODFSP_Command_t command_table[MAX_COMMANDS];
static uint8_t command_count = 0;

// Static instance for global usage (can be removed if you prefer instance-based approach)
static MODFSP_Data_t global_instance;

MODFSP_Return_t MODFSP_Init(MODFSP_Data_t *this)
{
    if (this == NULL) {
        return MODFSP_ERR;
    }
    
    MODFSP_Reset(this);
    return MODFSP_OK;
}

static void crc_init(MODFSP_Data_t *this, MODFSP_CRC_t *crc_obj)
{
    (void)this;
    memset(crc_obj, 0x00, sizeof(*crc_obj));
    crc_obj->crc = CRC16_XMODEM_INIT;
}

void MODFSP_Reset(MODFSP_Data_t *this)
{
    if (this == NULL) return;
    
    memset(&this->data, 0x00, sizeof(this->data));
    this->state = SFP_DECODE_START1; 
    this->crc16.crc = CRC16_XMODEM_INIT;
    this->index = 0;
    this->length = 0;
    this->crc16_data = 0;
    this->id = 0;
}

uint16_t crc16_xmodem_update(uint16_t crc, uint8_t data)
{
    crc ^= ((uint16_t)data) << 8;
    for (uint8_t i = 0; i < 8; ++i)
    {
        if (crc & 0x8000)
            crc = (crc << 1) ^ CRC16_XMODEM_POLY;
        else
            crc <<= 1;
    }
    return crc;
}

static void crc_update(MODFSP_CRC_t* crc_obj, uint8_t data)
{
    crc_obj->crc = crc16_xmodem_update(crc_obj->crc, data);
}

static uint16_t crc_finish(MODFSP_CRC_t* crc_obj)
{
    return crc_obj->crc;
}

static void go_to_next_rx_state_decode(MODFSP_Data_t *this)
{
    MODFSP_DecodeState_t next_state = SFP_DECODE_END;
    switch (this->state)
    {
    case SFP_DECODE_START1:
        next_state = SFP_DECODE_START2;
        break;
    case SFP_DECODE_START2:
        next_state = SFP_DECODE_ID;
        break;
    case SFP_DECODE_ID:
        next_state = SFP_DECODE_LEN_LOW;
        break;
    case SFP_DECODE_LEN_LOW:
        next_state = SFP_DECODE_LEN_HIGH;
        break;
    case SFP_DECODE_LEN_HIGH:
        if (this->length > 0) {
            next_state = SFP_DECODE_DATA;
        } else {
            next_state = SFP_DECODE_CRC;
        }
        break;
    case SFP_DECODE_DATA:
        next_state = SFP_DECODE_CRC;
        break;
    case SFP_DECODE_CRC:
        next_state = SFP_DECODE_STOP1;
        break;
    case SFP_DECODE_STOP1:
        next_state = SFP_DECODE_STOP2;
        break;
    case SFP_DECODE_STOP2:
        next_state = SFP_DECODE_START1;
        break;
    default:
        break;
    }
    if (next_state != SFP_DECODE_END)
    {
        this->state = next_state;
        this->index = 0;
    }
}

MODFSP_Return_t MODFSP_Read(MODFSP_Data_t *this, const uint8_t *rx_data)
{
    uint8_t byte = 0;
    MODFSP_Return_t res = MODFSP_OK;

    if (this == NULL || rx_data == NULL) {
        return MODFSP_ERR;
    }

    byte = *rx_data;

    switch (this->state)
    {
        case SFP_DECODE_START1:
        {

            if (byte == SFP_START1_BYTE)
            {
            	MODFSP_Log("Start1");
                MODFSP_Reset(this); /* Reset instance and make it ready for receiving */
                crc_init(this, &this->crc16);
                go_to_next_rx_state_decode(this);
            }
            break;
        }
        case SFP_DECODE_START2:
        {
            if (byte == SFP_START2_BYTE)
            {
            	MODFSP_Log("Start2");
                go_to_next_rx_state_decode(this);
            } else {
                MODFSP_Reset(this);
            }
            break;
        }
        case SFP_DECODE_ID:
        {
            this->id = byte;
            MODFSP_Log("ID: 0x%02X (%d)", byte, byte);
            crc_update(&this->crc16, byte);
            go_to_next_rx_state_decode(this);
            break;
        }
        case SFP_DECODE_LEN_LOW:
        {
        	MODFSP_Log("LEN-LOW: %d", byte);
            crc_update(&this->crc16, byte);
            this->length = byte;
            go_to_next_rx_state_decode(this);
            break;
        }
        case SFP_DECODE_LEN_HIGH:
        {
        	MODFSP_Log("LEN-HIGH: %d", byte);
            crc_update(&this->crc16, byte);
            this->length |= ((uint16_t)byte) << 8;
            go_to_next_rx_state_decode(this);
            break;
        }
        case SFP_DECODE_DATA:
        {
            if (this->index < sizeof(this->data))
            {
            	MODFSP_Log("DATA[%d]: 0x%02X (%d)", this->index - 1, byte, byte);
                this->data[this->index++] = byte;
                crc_update(&this->crc16, byte);
                if (this->index == this->length)
                {
                    go_to_next_rx_state_decode(this);
                }
            }
            else
            {
                MODFSP_Reset(this);
                res = MODFSP_ERRMEM;
                return res;
            }
            break;
        }
        case SFP_DECODE_CRC:
        {
            if (this->index < sizeof(this->crc16_data))
            {
                this->crc16_data |= (uint16_t)byte << (8 * this->index);
                ++this->index;
            }

            /* Check if we received all CRC bytes */
            if (this->index == sizeof(this->crc16_data))
            {
                uint16_t calculated_crc = crc_finish(&this->crc16);

                /* Check if calculated CRC matches the received data */
                if (calculated_crc == this->crc16_data)
                {
                	MODFSP_Log("CRC OK!");
                    go_to_next_rx_state_decode(this);
                }
                else
                {
                	MODFSP_Log("ERROR CRC!!!");
                    MODFSP_Reset(this);
                    res = MODFSP_ERRCRC;
                    return res;
                }
            }
            break;
        }

        case SFP_DECODE_STOP1:
        {
            if (byte == SFP_STOP1_BYTE)
            {
            	MODFSP_Log("Stop1");
                go_to_next_rx_state_decode(this);
            } else {
                MODFSP_Reset(this);
                res = MODFSP_ERRSTOP;
                return res;
            }
            break;
        }

        case SFP_DECODE_STOP2:
        {
            if (byte == SFP_STOP2_BYTE)
            {
            	MODFSP_Log("Stop2");
                res = MODFSP_VALID; /* Packet fully valid, take data from it */
                go_to_next_rx_state_decode(this);
                return res;
            }
            else
            {
                MODFSP_Reset(this);
                res = MODFSP_ERRSTOP;
                return res;
            }
            break;
        }
        default:
        {
            MODFSP_Reset(this);
            res = MODFSP_ERR;
            return res;
        }
    }

    res = (this->state == SFP_DECODE_START1) ? MODFSP_WAITDATA : MODFSP_INPROG;
    return res;
}

MODFSP_Return_t MODFSP_Send(MODFSP_Data_t *this, uint8_t id, const void* data, uint16_t len)
{
    MODFSP_Return_t res = MODFSP_OK;
    MODFSP_CRC_t crc;
    uint16_t org_len = len;
    const uint8_t* pdata = (const uint8_t*)data;
    uint8_t byte;
    uint16_t crc_value;

    if (this == NULL) {
        return MODFSP_ERR;
    }

    uint16_t min_mem = 9U + len; // 2 start + 2 stop + 1 id + 1 len + 2 CRC + data

    if (MODFSP_GetSpaceForTx() < min_mem){
        return MODFSP_ERRMEM;
    }

    crc_init(this, &crc);

    byte = SFP_START1_BYTE;
    MODFSP_SendByte(&byte);
    byte = SFP_START2_BYTE;
    MODFSP_SendByte(&byte);

    byte = id;
    MODFSP_SendByte(&byte);
    crc_update(&crc, byte);

    // length byte low first
    byte = (uint8_t)(len & 0xFF);
    MODFSP_SendByte(&byte);
    crc_update(&crc, byte);

    byte = (uint8_t)((len >> 8) & 0xFF);
    MODFSP_SendByte(&byte);
    crc_update(&crc, byte);

    for (uint16_t i = 0; i < org_len; i++) {
        byte = pdata[i];
        MODFSP_SendByte(&byte);
        crc_update(&crc, byte);
    }

    crc_value = crc_finish(&crc);

    byte = (uint8_t)(crc_value & 0xFF);
    MODFSP_SendByte(&byte);
    byte = (uint8_t)((crc_value >> 8) & 0xFF);
    MODFSP_SendByte(&byte);

    byte = SFP_STOP1_BYTE;
    MODFSP_SendByte(&byte);
    byte = SFP_STOP2_BYTE;
    MODFSP_SendByte(&byte);

    return res;
}

MODFSP_Return_t MODFSP_Process(MODFSP_Data_t *this)
{
    uint8_t byte = 0;
    uint32_t now = MODFSP_GetTick();
    int res_read = MODFSP_ReadByte(&byte);

    if (this == NULL) {
        return MODFSP_ERR;
    }

    if (res_read == 0)
    {
        MODFSP_Return_t res = MODFSP_Read(this, &byte);

        if (res == MODFSP_VALID) {
            this->last_rx_time = now;
            MODFSP_Log("Call Handler");
            MODFSP_ApplicationHandler(this, this->id, this->data, this->length);
        }
        else if (res == MODFSP_INPROG) {
            this->last_rx_time = now;
        }

        return res;
    }
    else
    {
        if ((this->state != SFP_DECODE_START1) &&
            ((now - this->last_rx_time) > MODFSP_TIMEOUT_MS))
        {
        	MODFSP_Log("Timeout - Reset state machine");
            MODFSP_Reset(this);
            this->last_rx_time = now;
            return MODFSP_ERRTIMEOUT;
        }
    }

    return MODFSP_WAITDATA;
}


// Global process function using static instance
MODFSP_Return_t MODFSP_ProcessGlobal(void)
{
    return MODFSP_Process(&global_instance);
}

// Command table management functions
void MODFSP_RegisterCommand(uint8_t id, MODFSP_Handler_t handler)
{
    if (command_count < MAX_COMMANDS && handler != NULL) {
        command_table[command_count].id = id;
        command_table[command_count].handler = handler;
        command_count++;
    }
}

const MODFSP_Command_t *MODFSP_GetCommandTable(void)
{
    return command_table;
}

uint8_t MODFSP_GetCommandTableSize(void)
{
    return command_count;
}

// Default application handler implementation
__attribute__((weak)) void MODFSP_ApplicationHandler(MODFSP_Data_t *ctx, uint8_t id, const uint8_t *payload, uint16_t len)
{
    // Search for registered command handler
    for (uint8_t i = 0; i < command_count; i++) {
        if (command_table[i].id == id) {
            command_table[i].handler(ctx, payload, len);
            return;
        }
    }
    
    // Default handler - do nothing or log unknown command
    // User can override this function in their application
}

// Utility function to get global instance
MODFSP_Data_t *MODFSP_GetGlobalInstance(void)
{
    return &global_instance;
}
