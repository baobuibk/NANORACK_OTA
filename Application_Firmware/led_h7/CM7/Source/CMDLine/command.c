/*
 * command.c
 *
 * Created: 5/19/2024 8:37:53 PM
 *  Author: Admin
 */ 
#include "scheduler.h"
#include "command.h"
#include "UART.h"
#include <stdlib.h>
#include "cmdline.h"
#include "bootmode.h"

/* Private typedef -----------------------------------------------------------*/

const char * ErrorCode[5] = {"\r\nOK\r\n", "\r\nCMDLINE_BAD_CMD\r\n", "\r\nCMDLINE_TOO_MANY_ARGS\r\n",
"\r\nCMDLINE_TOO_FEW_ARGS\r\n", "\r\nCMDLINE_INVALID_ARG\r\n" };

tCmdLineEntry g_psCmdTable[] = {{ "help", Cmd_help, " : format: help"},					
								{"get_temp", Cmd_get_temp , " : format: get_temp"},
								{"reset", Cmd_bootloader , " : jump to bootloader mode"},
								{0,0,0}
								};

volatile static	ringbuffer_t *p_CommandRingBuffer;
static	char s_commandBuffer[COMMAND_MAX_LENGTH];
static uint8_t	s_commandBufferIndex = 0;


void	command_init(void)
{
	USART2_init();
	p_CommandRingBuffer = uart_get_USART2_rx_buffer_address();
	memset((void *)s_commandBuffer, 0, sizeof(s_commandBuffer));
	s_commandBufferIndex = 0;
	USART2_send_string("OBC FOTA V1.0.0 \r\n>");
	//UARTprintf("\r\nHIGH_POINT: %d\r\nLOW_POINT: %d\r\nPWM: %d\r\n", eeprom_read_word(HIGH_TEMP_POINT_ADR), eeprom_read_word(LOW_TEMP_POINT_ADR), eeprom_read_byte(HEATER_PWM_ADR));
}

static const double rounders[MAX_PRECISION + 1] =
{
	0.5,				// 0
	0.05,				// 1
	0.005,				// 2
};

char * ftoa(double f, char * buf, int precision)
{
	char * ptr = buf;
	char * p = ptr;
	char * p1;
	char c;
	long intPart;
	if (precision > MAX_PRECISION)
	precision = MAX_PRECISION;
	if (f < 0)
	{
		f = -f;
		*ptr++ = '-';
	}
	if (precision < 0)  
	{
		if (f < 1.0) precision = 6;
		else if (f < 10.0) precision = 5;
		else if (f < 100.0) precision = 4;
		else if (f < 1000.0) precision = 3;
		else if (f < 10000.0) precision = 2;
		else if (f < 100000.0) precision = 1;
		else precision = 0;
	}
	if (precision)
	f += rounders[precision];
	intPart = f;
	f -= intPart;

	if (!intPart)
	*ptr++ = '0';
	else
	{
		p = ptr;
		while (intPart)
		{
			*p++ = '0' + intPart % 10;
			intPart /= 10;
		}
		p1 = p;
		while (p > ptr)
		{
			c = *--p;
			*p = *ptr;
			*ptr++ = c;
		}
		ptr = p1;
	}
	if (precision)
	{
		*ptr++ = '.';
		while (precision--)
		{
			f *= 10.0;
			c = f;
			*ptr++ = '0' + c;
			f -= c;
		}
	}
	*ptr = 0;
	return buf;
}

void command_task_update(void*)
{
	char rxData;
	int8_t	ret_val;
	while (! rbuffer_empty(p_CommandRingBuffer))
	{
		rxData = rbuffer_remove(p_CommandRingBuffer);
		USART2_send_char(rxData);
		if ((rxData == '\r') || (rxData == '\n'))		//got a return or new line
		{
			if (s_commandBufferIndex > 0)		//if we got the CR or LF at the begining, discard	
			{
				s_commandBuffer[s_commandBufferIndex] = 0;
				s_commandBufferIndex++;
				ret_val = CmdLineProcess(s_commandBuffer);		
				s_commandBufferIndex = 0;		
				USART2_send_string(ErrorCode[ret_val]);
				USART2_send_string("> ");
			}	
			else USART2_send_string("\r\n> ");
		}
		else if ((rxData == 8) || (rxData == 127))	
		{
			if (s_commandBufferIndex > 0) s_commandBufferIndex--;
		}
		else
		{
			s_commandBuffer[s_commandBufferIndex] = rxData;
			s_commandBufferIndex ++;
			if (s_commandBufferIndex >= COMMAND_MAX_LENGTH) s_commandBufferIndex= 0;
		}
	}	
}



int Cmd_help(int argc, char *argv[]) {
	tCmdLineEntry *pEntry;

	USART2_send_string("\nAvailable commands: \r\n");
	
	// Point at the beginning of the command table.
	pEntry = &g_psCmdTable[0];

	// Enter a loop to read each entry from the command table.  The
	// end of the table has been reached when the command name is NULL.
	while (pEntry->pcCmd) {
		// Print the command name and the brief description.
		USART2_send_string(pEntry->pcCmd);
		USART2_send_string(pEntry->pcHelp);
		USART2_send_string("\r\n");

		// Advance to the next entry in the table.
		pEntry++;

	}
	// Return success.
	return (CMDLINE_OK);
}

//*****************************************************************************
//
// Format: get_temp
// Get the temperature from NTC
//
//*****************************************************************************
int	Cmd_get_temp(int argc, char *argv[])
{
	if (argc < 2) return CMDLINE_TOO_FEW_ARGS;
	if (argc > 2) return CMDLINE_TOO_MANY_ARGS;

	uint8_t hehe = atoi(argv[1]);
	UARTprintf("\r\nNTC %d: ", hehe);

	return CMDLINE_OK;
}

//*****************************************************************************
//
// Format:
// Jump to bootloader mode
//
//*****************************************************************************
int Cmd_bootloader(int argc, char *argv[])
{
	full_system_reset();
	return CMDLINE_OK;
}

