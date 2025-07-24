#include "stdint.h"

#define MODFSP_ENABLE_LOG        1

#define MODFSP_LOG_METHOD        1

#define MODFSP_LOG_BUFFER_SIZE   256

#if MODFSP_ENABLE_LOG

void MODFSP_Log(const char *fmt, ...);

#else

#define MODFSP_Log(...)    ((void)0)

#endif

void MODFSP_SendByte(const uint8_t *byte);

int MODFSP_ReadByte(uint8_t *byte);

uint16_t MODFSP_GetSpaceForTx(void);

uint32_t MODFSP_GetTick(void);

