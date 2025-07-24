/*
 * filesystem.h
 *
 *  Created on: Mar 2, 2025
 *      Author: CAO HIEU
 */

#ifndef M1_DRIVERS_FILESYSTEM_FILESYSTEM_H_
#define M1_DRIVERS_FILESYSTEM_FILESYSTEM_H_

#include "main.h"
#include "CLI_Terminal/CLI_Command/cli_command.h"
#include "utils.h"

typedef enum{
	SDFS_READY = 0,
	SDFS_RELEASE,
	SDFS_BUSY,
	SDFS_ERROR
}SDFS_StateTypedef;
extern SDFS_StateTypedef SDFS_State;

void FS_Init(void);
void FS_Gatekeeper_Task(void *pvParameters);
Std_ReturnType FS_Request_Write(const char* filename, uint8_t* buffer, uint32_t size);

void SD_Lockin(void);
void SD_Release(void);
void FS_ListFiles_path(EmbeddedCli *cli);
Std_ReturnType Link_SDFS_Driver(void);
int Vim_SDFS(EmbeddedCli *cli, const char *filename, const char *content);
int Cat_SDFS(EmbeddedCli *cli, const char *filename);

#endif /* M1_DRIVERS_FILESYSTEM_FILESYSTEM_H_ */
