/*
 * inter_cpu_comm.c
 *
 *  Created on: Mar 1, 2025
 *      Author: CAO HIEU
 */

#include "inter_cpu_comm.h"

#define FRAME_START_BYTE1  0xE2
#define FRAME_START_BYTE2  0xF2
#define FRAME_END_BYTE1    0xF3
#define FRAME_END_BYTE2    0xE3

#define IPC_MEMSET                      memset
#define IPC_MEMCPY                      memcpy

#define IPC_isVALID(b)                  ((b) != NULL && (b)->pBuff != NULL && (b)->size > 0)

#define IPC_MIN(x, y)                   ((x) < (y) ? (x) : (y))
#define IPC_MAX(x, y)                   ((x) > (y) ? (x) : (y))
#define IPC_SEND_EVT(b, type, bp)       do { if ((b)->evt_fn != NULL) { (b)->evt_fn((b), (type), (bp)); } } while (0)

void BufferIPC_ClearRAM_PreInit(void) {
	IPC_MEMSET((void*)SHARED_RAM_START_ADDR, 0, IPC_SHARED_RAM_SIZE);
}

volatile BufferIPC_t* BufferIPC_GetChannel(IPC_Channel_t channel) {
    if (channel == IPC_CHANNEL_CM4_TO_CM7) {
        return (volatile BufferIPC_t*) IPC_CM4_TO_CM7_ADDR;
    } else if (channel == IPC_CHANNEL_CM7_TO_CM4) {
        return (volatile BufferIPC_t*) IPC_CM7_TO_CM4_ADDR;
    }
    return NULL;
}

uint8_t BufferIPC_Init(IPC_VOLATILE BufferIPC_t* buff, void* buffdata, size_t size) {
    if (buff == NULL || buffdata == NULL || size == 0) {
        return 0;
    }
    IPC_MEMSET((void *)buff, 0x00, sizeof(*buff));
    buff->size = size;
    buff->pBuff = (uint8_t*)buffdata;
    return 1;
}

void BufferIPC_SetEvtFn(IPC_VOLATILE BufferIPC_t* buff, IPC_evt_fn evt_fn) {
    if (IPC_isVALID(buff)) {
        buff->evt_fn = evt_fn;
    }
}

uint8_t BufferIPC_IsReady(IPC_VOLATILE BufferIPC_t* buff) {
    return IPC_isVALID(buff);
}

void BufferIPC_Free(IPC_VOLATILE BufferIPC_t* buff) {
    if (IPC_isVALID(buff)) {
        buff->pBuff = NULL;
    }
}

size_t BufferIPC_GetFree(IPC_VOLATILE BufferIPC_t* buff) {
    size_t free_space, w, r;
    if (!IPC_isVALID(buff)) {
        return 0;
    }
    w = buff->w;
    r = buff->r;
    if (w == r) {
        free_space = buff->size;
    } else if (r > w) {
        free_space = r - w;
    } else {
        free_space = buff->size - (w - r);
    }
    // 1 byte to check free or empty
    return free_space - 1;
}

size_t BufferIPC_GetFull(IPC_VOLATILE BufferIPC_t* buff) {
    size_t full, w, r;
    if (!IPC_isVALID(buff)) {
        return 0;
    }
    w = buff->w;
    r = buff->r;
    if (w == r) {
        full = 0;
    } else if (w > r) {
        full = w - r;
    } else {
        full = buff->size - (r - w);
    }
    return full;
}

size_t BufferIPC_Write(IPC_VOLATILE BufferIPC_t* buff, const void* data, size_t len) {
    size_t free_space, to_write, first_chunk;
    const uint8_t* d = (const uint8_t*)data;
    if (!IPC_isVALID(buff) || data == NULL || len == 0) {
        return 0;
    }
    free_space = BufferIPC_GetFree(buff);
    to_write = IPC_MIN(free_space, len);
    if (to_write == 0) {
        return 0;
    }
    first_chunk = IPC_MIN(buff->size - buff->w, to_write);
    IPC_MEMCPY(&buff->pBuff[buff->w], d, first_chunk);
    buff->w += first_chunk;
    to_write -= first_chunk;
    //(wrap-around)
    if (to_write > 0) {
        IPC_MEMCPY(buff->pBuff, &d[first_chunk], to_write);
        buff->w = to_write;
    }
    if (buff->w >= buff->size) {
        buff->w = 0;
    }
    IPC_SEND_EVT(buff, IPC_EVT_WRITE, first_chunk + to_write);
    return first_chunk + to_write;
}

size_t BufferIPC_Read(IPC_VOLATILE BufferIPC_t* buff, void* data, size_t len) {
    size_t full, to_read, first_chunk;
    uint8_t* d = (uint8_t*)data;
    if (!IPC_isVALID(buff) || data == NULL || len == 0) {
        return 0;
    }
    full = BufferIPC_GetFull(buff);
    to_read = IPC_MIN(full, len);
    if (to_read == 0) {
        return 0;
    }
    first_chunk = IPC_MIN(buff->size - buff->r, to_read);
    IPC_MEMCPY(d, &buff->pBuff[buff->r], first_chunk);
    buff->r += first_chunk;
    to_read -= first_chunk;
    if (to_read > 0) {
        IPC_MEMCPY(&d[first_chunk], buff->pBuff, to_read);
        buff->r = to_read;
    }
    if (buff->r >= buff->size) {
        buff->r = 0;
    }
    IPC_SEND_EVT(buff, IPC_EVT_READ, first_chunk + to_read);
    return first_chunk + to_read;
}

size_t BufferIPC_Peek(IPC_VOLATILE BufferIPC_t* buff, size_t skip_count, void* data, size_t len) {
    size_t full, to_peek, first_chunk;
    size_t r = buff->r;
    uint8_t* d = (uint8_t*)data;
    if (!IPC_isVALID(buff) || data == NULL || len == 0) {
        return 0;
    }
    full = BufferIPC_GetFull(buff);
    if (skip_count >= full) {
        return 0;
    }
    r = (r + skip_count) % buff->size;
    full -= skip_count;
    to_peek = IPC_MIN(full, len);
    if (to_peek == 0) {
        return 0;
    }
    first_chunk = IPC_MIN(buff->size - r, to_peek);
    IPC_MEMCPY(d, &buff->pBuff[r], first_chunk);
    to_peek -= first_chunk;
    if (to_peek > 0) {
        IPC_MEMCPY(&d[first_chunk], buff->pBuff, to_peek);
    }
    return first_chunk + to_peek;
}

void BufferIPC_Reset(IPC_VOLATILE BufferIPC_t* buff) {
    if (IPC_isVALID(buff)) {
        buff->r = 0;
        buff->w = 0;
        IPC_SEND_EVT(buff, IPC_EVT_RESET, 0);
    }
}

void* BufferIPC_GetLinearBlockReadAddress(IPC_VOLATILE BufferIPC_t* buff) {
    if (!IPC_isVALID(buff)) {
        return NULL;
    }
    return &buff->pBuff[buff->r];
}

size_t BufferIPC_GetLinearBlockReadLength(IPC_VOLATILE BufferIPC_t* buff) {
    size_t first_chunk, w, r;
    if (!IPC_isVALID(buff)) {
        return 0;
    }
    w = buff->w;
    r = buff->r;
    if (w > r) {
        first_chunk = w - r;
    } else if (r > w) {
        first_chunk = buff->size - r;
    } else {
        first_chunk = 0;
    }
    return first_chunk;
}

size_t BufferIPC_Skip(IPC_VOLATILE BufferIPC_t* buff, size_t len) {
    size_t full;
    if (!IPC_isVALID(buff) || len == 0) {
        return 0;
    }
    full = BufferIPC_GetFull(buff);
    len = IPC_MIN(len, full);
    buff->r = (buff->r + len) % buff->size;
    IPC_SEND_EVT(buff, IPC_EVT_READ, len);
    return len;
}

void* BufferIPC_GetLinearBlockWriteAddress(IPC_VOLATILE BufferIPC_t* buff) {
    if (!IPC_isVALID(buff)) {
        return NULL;
    }
    return &buff->pBuff[buff->w];
}

size_t BufferIPC_GetLinearBlockWriteLength(IPC_VOLATILE BufferIPC_t* buff) {
    size_t w, r, len;
    if (!IPC_isVALID(buff)) {
        return 0;
    }
    w = buff->w;
    r = buff->r;
    if (w >= r) {
        len = buff->size - w;
        if (r == 0 && len > 0) {
            len--; 
        }
    } else {
        len = r - w - 1;
    }
    return len;
}

size_t BufferIPC_Advance(IPC_VOLATILE BufferIPC_t* buff, size_t len) {
    size_t free_space;
    if (!IPC_isVALID(buff) || len == 0) {
        return 0;
    }
    free_space = BufferIPC_GetFree(buff);
    len = IPC_MIN(len, free_space);
    buff->w = (buff->w + len) % buff->size;
    IPC_SEND_EVT(buff, IPC_EVT_WRITE, len);
    return len;
}

 /**
 * @brief Writes a frame into the IPC buffer.
 * The frame is packaged in the following format:
 *   [FRAME_START_BYTE1][FRAME_START_BYTE2] + payload + [FRAME_END_BYTE1][FRAME_END_BYTE2]
 *
 * @param buff   Pointer to the IPC buffer.
 * @param data   The ASCII string to be sent (null-terminated).
 * @return The total number of bytes written to the buffer (frame length), 
 *         or 0 if there is not enough space.
 */

size_t BufferIPC_WriteFrame(IPC_VOLATILE BufferIPC_t* buff, const char* data) {
    size_t payload_len, frame_len, cont_free;
    uint8_t start_marker[2] = { FRAME_START_BYTE1, FRAME_START_BYTE2 };
    uint8_t end_marker[2]   = { FRAME_END_BYTE1, FRAME_END_BYTE2 };

    if (buff == NULL || data == NULL) {
        return 0;
    }

    payload_len = strlen(data);
    frame_len = payload_len + 4;

    if (BufferIPC_GetFree(buff) < frame_len) {
        return 0;
    }

    cont_free = BufferIPC_GetLinearBlockWriteLength(buff);
    if (cont_free < frame_len) {
        buff->w = 0;
    }

    if (BufferIPC_Write(buff, start_marker, 2) != 2) {
        return 0;
    }
    if (BufferIPC_Write(buff, data, payload_len) != payload_len) {
        return 0;
    }
    if (BufferIPC_Write(buff, end_marker, 2) != 2) {
        return 0;
    }

    return frame_len;
}

/**
 * @brief Finds and returns a complete frame in the buffer.
 *
 * This function scans the available continuous block in the buffer to check 
 * if there is a complete frame that starts with FRAME_START_BYTE1, FRAME_START_BYTE2 
 * and ends with FRAME_END_BYTE1, FRAME_END_BYTE2.
 *
 * If a frame is found, the function returns the address of the payload 
 * (excluding the 2 start bytes) and updates:
 *    - *payload_len: the length of the payload
 *    - *total_frame_len: the total number of bytes in the frame (payload + 4 marker bytes)
 *
 * If no complete frame is found (or only the start marker exists), the function returns NULL.
 *
 * @param buff             Pointer to the IPC buffer.
 * @param payload_len[out] Outputs the payload length (excluding markers).
 * @param total_frame_len[out] Outputs the total number of bytes in the frame (including start and end markers).
 * @return Pointer to the payload in the buffer if a frame is found, NULL otherwise.
 */
uint8_t* BufferIPC_GetFrame(IPC_VOLATILE BufferIPC_t* buff, size_t* payload_len, size_t* total_frame_len) {
    size_t linear_len;
    uint8_t* data;
    size_t i;

    if (!IPC_isVALID(buff) || payload_len == NULL || total_frame_len == NULL) {
        return NULL;
    }
    
    linear_len = BufferIPC_GetLinearBlockReadLength(buff);
    if (linear_len < 4) {
        return NULL;
    }

    data = (uint8_t*)BufferIPC_GetLinearBlockReadAddress(buff);

    if (data[0] != FRAME_START_BYTE1 || data[1] != FRAME_START_BYTE2) {
        return NULL;
    }

    for (i = 2; i <= linear_len - 2; i++) {
        if (data[i] == FRAME_END_BYTE1 && data[i+1] == FRAME_END_BYTE2) {
            *payload_len = i - 2;        // Payload is between start and end markers
            *total_frame_len = i + 2;    // Total frame = start marker (2 bytes) + payload + end marker (2 bytes)
            return &data[2];             // Return the payload address (excluding start marker)
        }
    }

    return NULL;
}

/* Example usage on the receiver side:
 *
 * size_t payload_len, frame_len;
 * uint8_t* payload;
 * 
 * // Check if a complete frame is available in the buffer
 * payload = BufferIPC_GetFrame(rb_cm4_to_cm7, &payload_len, &frame_len);
 * if (payload != NULL) {
 *     // Example: Transmit the payload over UART
 *     // Note: payload_len only includes the data, excluding markers
 *     HAL_UART_Transmit(&huart2, payload, payload_len, 1000);
 *
 *     // After processing, skip the entire frame (including start and end markers)
 *     BufferIPC_Skip(rb_cm4_to_cm7, frame_len);
 * }
 *
 * If no complete frame is found (or only the start marker exists), 
 * BufferIPC_GetFrame will return NULL.
 */


