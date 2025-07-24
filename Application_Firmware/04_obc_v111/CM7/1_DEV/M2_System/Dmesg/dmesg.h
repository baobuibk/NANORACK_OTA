/************************************************
 *  @file     : dmesg.h
 *  @date     : May 9, 2025
 *  @author   : CAO HIEU
 *-----------------------------------------------
 *  Description :
 *    [-]
 ************************************************/

#ifndef M2_SYSTEM_DMESG_DMESG_H_
#define M2_SYSTEM_DMESG_DMESG_H_
#include "stddef.h"
#include "CLI_Terminal/CLI_Src/embedded_cli.h"

#define DMESG_MSG_MAX_LENGTH 128

typedef void (*Dmesg_LogCallback)(const char *log, size_t len);

void Dmesg_Init(void);
void Dmesg_HardWrite(const char *msg);
void Dmesg_SafeWrite(const char *msg);

void Dmesg_GetLogs(EmbeddedCli *cli);
void Dmesg_GetLatestN(size_t N, EmbeddedCli *cli);

#endif /* M2_SYSTEM_DMESG_DMESG_H_ */
