/*
 * command.h
 *
 * Created: 5/19/2024 8:38:57 PM
 *  Author: Admin
 */ 


#ifndef COMMAND_H_
#define COMMAND_H_

#include "cmdline.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define	COMMAND_MAX_LENGTH	32
#define MAX_PRECISION	(2)

void command_init(void);
void command_task_update(void*);

int	Cmd_help(int argc, char *argv[]);

char * ftoa(double f, char * buf, int precision);
int	Cmd_get_temp(int argc, char *argv[]);
int Cmd_bootloader(int argc, char *argv[]);
#endif /* COMMAND_H_ */
