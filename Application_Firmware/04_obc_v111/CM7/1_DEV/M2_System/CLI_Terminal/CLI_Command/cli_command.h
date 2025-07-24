/*
 * cli_binding.h
 *
 *  Created on: Feb 27, 2025
 *      Author: CAO HIEU
 */

#ifndef M2_SYSTEM_CLI_TERMINAL_CLI_COMMAND_CLI_COMMAND_H_
#define M2_SYSTEM_CLI_TERMINAL_CLI_COMMAND_CLI_COMMAND_H_

#include "CLI_Terminal/CLI_Src/embedded_cli.h"

const CliCommandBinding *getCliStaticBindings(void) ;
uint16_t getCliStaticBindingCount(void);

#endif /* M2_SYSTEM_CLI_TERMINAL_CLI_COMMAND_CLI_COMMAND_H_ */
