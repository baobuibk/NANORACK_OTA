/*
 * cli_setup.h
 *
 *  Created on: Feb 27, 2025
 *      Author: CAO HIEU
 */

#ifndef M2_SYSTEM_CLI_TERMINAL_CLI_SETUP_CLI_SETUP_H_
#define M2_SYSTEM_CLI_TERMINAL_CLI_SETUP_CLI_SETUP_H_
#include "CLI_Terminal/CLI_Src/embedded_cli.h"
#include "utils.h"
// Definitions for CLI sizes
//#define CLI_BUFFER_SIZE 		2048
#define CLI_RX_BUFFER_SIZE 		16
#define CLI_CMD_BUFFER_SIZE 	64
#define CLI_HISTORY_SIZE 		128
#define CLI_MAX_BINDING_COUNT 	32
#define CLI_AUTO_COMPLETE 		1
//#define CLI_INITATION_CM4		"CM4-OBC@STM32:~ $ "
#define CLI_INITATION_USB		"USB-OBC@STM32:~ $ "

/**
 * Char for char is needed for instant char echo, so size 1
 */
//#define UART_RX_BUFF_SIZE 1

/**
 * Function to setup the configuration settings for the CLI,
 * based on the definitions from this header file
 */
Std_ReturnType SystemCLI_Init();

/**
 * Getter function, to keep only one instance of the EmbeddedCli pointer in this file.
 * @return
 */
EmbeddedCli *getUsbCdcCliPointer();
//EmbeddedCli *getUartCm4CliPointer();

#endif /* M2_SYSTEM_CLI_TERMINAL_CLI_SETUP_CLI_SETUP_H_ */
