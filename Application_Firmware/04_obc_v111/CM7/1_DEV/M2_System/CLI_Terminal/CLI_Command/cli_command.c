/*
 * cli_setup.c
 *
 *  Created on: Feb 27, 2025
 *      Author: CAO HIEU
 */
#include "cli_command.h"
#include "stdio.h"
#include "CLI_Terminal/CLI_Setup/cli_setup.h"
#include "stdlib.h"
#include "string.h"
#include "main.h"
#include "board.h"
/*************************************************
 *              More User Include                *
 *************************************************/
#include "DateTime/date_time.h"
#include "SPI_FRAM/fram_spi.h"
#include "uart_driver_dma.h"
#include "SPI_SlaveOfCM4/spi_slave.h"
#include "SPI_MasterOfEXP/spi_master.h"
#include "min_proto.h"
#include "MIN_Process/min_process.h"
#include "CLI_Terminal/CLI_Auth/simple_shield.h"
#include "Dmesg/dmesg.h"

#include "reinit.h"
//#include "inter_cpu_comm.h"
#include "utils.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "filesystem.h"
#include "mode.h"
#include "gpio_state.h"

#include "ScriptManager/script_manager.h"
#include "log_manager.h"
#include "lwl.h"

#include "AliveCM4/alive_cm4.h"

#include "SimpleDataTransfer/simple_datatrans.h"

#include "modfsp.h"

#define FRAM_USER_PWD_LEN_ADDR  0x0000
#define FRAM_USER_PWD_ADDR      0x0001
extern ShieldInstance_t auth_uart;
extern ShieldInstance_t auth_usb;
/*************************************************
 *                Command Define                 *
 *************************************************/
static void CMD_ClearCLI(EmbeddedCli *cli, char *args, void *context);
static void CMD_RtcSet(EmbeddedCli *cli, char *args, void *context);
static void CMD_RtcGet(EmbeddedCli *cli, char *args, void *context);
static void CMD_RtcSetEpoch(EmbeddedCli *cli, char *args, void *context);
static void CMD_FramWrite(EmbeddedCli *cli, char *args, void *context);
static void CMD_FramRead(EmbeddedCli *cli, char *args, void *context);
static void CMD_ls(EmbeddedCli *cli, char *args, void *context);
static void CMD_sd_release(EmbeddedCli *cli, char *args, void *context);
static void CMD_sd_lockin(EmbeddedCli *cli, char *args, void *context);
static void CMD_vim_bypass(EmbeddedCli *cli, char *args, void *context);
static void CMD_vim(EmbeddedCli *cli, char *args, void *context);
static void CMD_cat(EmbeddedCli *cli, char *args, void *context);
static void CMD_Cm4Rst(EmbeddedCli *cli, char *args, void *context);
static void CMD_Cm4Dis(EmbeddedCli *cli, char *args, void *context);
static void CMD_Cm4Ena(EmbeddedCli *cli, char *args, void *context);
static void CMD_ExpForward(EmbeddedCli *cli, char *args, void *context);
static void CMD_ExpSend(EmbeddedCli *cli, char *args, void *context);
static void CMD_ExpListen(EmbeddedCli *cli, char *args, void *context);
static void CMD_Reset(EmbeddedCli *cli, char *args, void *context);
static void CMD_AliveCheck(EmbeddedCli *cli, char *args, void *context);
static void CMD_RtosCheck(EmbeddedCli *cli, char *args, void *context);
static void CMD_LogOut(EmbeddedCli *cli, char *args, void *context);
static void CMD_PwdChange(EmbeddedCli *cli, char *args, void *context);

static void CMD_RamFill(EmbeddedCli *cli, char *args, void *context);
static void CMD_RamDump(EmbeddedCli *cli, char *args, void *context);
static void CMD_StateToCM4(EmbeddedCli *cli, char *args, void *context);
static void CMD_CollectData(EmbeddedCli *cli, char *args, void *context);
static void CMD_PullData(EmbeddedCli *cli, char *args, void *context);
static void CMD_SPISlaveRST(EmbeddedCli *cli, char *args, void *context);
static void CMD_MasterRead(EmbeddedCli *cli, char *args, void *context);

static void CMD_Dmesg(EmbeddedCli *cli, char *args, void *context);

/*************************************************
 * Command Define "Dev"              *
 *************************************************/
static void CMD_DevCM4TestConnection(EmbeddedCli *cli, char *args, void *context);

static void CMD_DevEXPPleaseReset(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevTestConnection(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevEXPSetRTC(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevNTCSetControl(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevSetTempProfile(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevStartTempProfile(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevStopTempProfile(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevSetOverrideTecProfile(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevStartOverrideTecProfile(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevStopOverrideTecProfile(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevSetSamplingProfile(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevSetLaserIntensity(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevSetPosition(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevStartSamplingCycle(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevGetInfoSample(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevGetChunk(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevGetCRCChunk(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevSetExtLaserProfile(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevTurnOnExtLaser(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevTurnOffExtLaser(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevSetLaserInt(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevSetLaserExt(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevGetCurrentLaser(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevCustomCommand(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevScriptManager(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevEraseScript(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevLogManagerDebug(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevLogManagerLog(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevCM4KeepAliveStatus(EmbeddedCli *cli, char *args, void *context);

static void CMD_Testcase(EmbeddedCli *cli, char *args, void *context);
static void CMD_FormatSD(EmbeddedCli *cli, char *args, void *context);
/*************************************************
 *                 Command  Array                *
 *************************************************/
// Guide: Command bindings are declared in the following order:
// { category, name, help, tokenizeArgs, context, binding }
// - category: Command group; set to NULL if grouping is not needed.
// - name: Command name (required)
// - help: Help string describing the command (required)
// - tokenizeArgs: Set to true to automatically split arguments when the command is called.
// - context: Pointer to a command-specific context; can be NULL.
// - binding: Callback function that handles the command.


static const CliCommandBinding cliStaticBindings_internal[] = {
	{ "Ultis",          "dev",          "Print list of developer commands",             		false,  NULL, CMD_Dev,           },
    { "Ultis",		 	"help",        	"Print list of commands [Firmware: 1]",             	false,  NULL, CMD_Help,			 },
    { "Ultis",			"cls",         	"Clears the console",                               	false,  NULL, CMD_ClearCLI,  	 },
    { "Time", 			"rtc_set",     	"Set RTC time: rtc_set <h> <m> <s> <DD> <MM> <YY>",   	true,  	NULL, CMD_RtcSet,    	 },
    { "Time",         	"rtc_get",     	"Get RTC. Usage: rtc_get <hard|soft|work|epoch|all>",  	true,  	NULL, CMD_RtcGet,    	 },
    { "Time",         	"epoch_set",   	"Set RTC time by epoch: rtc_setepoch <epoch>",        	true,  	NULL, CMD_RtcSetEpoch, 	 },
    { NULL,         	"fram_write",  	"Write to FRAM: fram_write [address] [value]",        	true,  	NULL, CMD_FramWrite, 	 },
    { NULL,         	"fram_read",   	"Read from FRAM: fram_read [address]",                	true,  	NULL, CMD_FramRead,  	 },
	{ "FileSystem", 	"ls", 			"List files in filesystem", 							false, 	NULL, CMD_ls 			 },
	{ "FileSystem", 	"format_sd", 	"Format SD card filesystem (delete all)", 				false, 	NULL, CMD_FormatSD 	     },
	{ "FileSystem", 	"sd_lockin", 	"Lock SD filesystem", 									false, 	NULL, CMD_sd_lockin  	 },
    { "FileSystem", 	"sd_release", 	"Release SD filesystem", 								false, 	NULL, CMD_sd_release 	 },
    { "FileSystem", 	"vim_bypass", 	"Write no queue: vim_bypass <filename> \"content\"", 	true, 	NULL, CMD_vim_bypass 	 },
    { "FileSystem", 	"vim", 			"Write queue: vim <filename> \"content\"", 				true, 	NULL, CMD_vim 			 },
    { "FileSystem", 	"cat", 			"Read file content: cat <filename>", 					true, 	NULL, CMD_cat 			 },
    { "RP-CM4",         "cm4_rst",     	"Trigger CM4 reset pulse (low then high)",          	false, 	NULL, CMD_Cm4Rst,    	 },
    { "RP-CM4",         "cm4_dis",     	"Disable CM4 power (drive enable low)",             	false, 	NULL, CMD_Cm4Dis,    	 },
    { "RP-CM4",         "cm4_ena",     	"Enable CM4 power (drive enable high)",             	false, 	NULL, CMD_Cm4Ena,    	 },
    { "EXP",         	"exp_forward", 	"Enable forward mode: exp_forward <cm4|usb|normal>", 	false, 	NULL, CMD_ExpForward,	 },
    { "EXP",         	"exp_send",    	"Send Msg to EXP: exp_send \"message\"",            	true,  	NULL, CMD_ExpSend,   	 },
    { "EXP",         	"exp_listen",  	"Choose w listen to EXP: exp_listen <cm4|usb|off>",  	true,  	NULL, CMD_ExpListen, 	 },
    { "System",         "alive_check", 	"Check alive OBC - EXP: alive_check",               	false, 	NULL, CMD_AliveCheck,	 },
	{ "System", 		"rtos_check", 	"Check FreeRTOS tasks: rtos_check", 					false, 	NULL, CMD_RtosCheck 	 },
	{ "System", 		"log_out", 		"Log Out", 												false, 	NULL, CMD_LogOut		 },
    { "System",         "pwd_change",   "Change user password: pwd_change <new_password>", 		true,   NULL, CMD_PwdChange    	 },
    { "System",         "dmesg",        "Print dmesg logs: dmesg [N]",                          true,   NULL, CMD_Dmesg,         },
    { NULL,         	"reset",       	"Reset MCU: reset",                                 	false, 	NULL, CMD_Reset,     	 },

	{ "Memory",         "ram_fill",     "Fill 200KB RAM_D2 with pattern data 1|2|3",    		true,   NULL, CMD_RamFill,       },
	{ NULL,             "ram_dump",     "Dump contents of 200KB RAM_D2",               			false,  NULL, CMD_RamDump,       },
	{ NULL,             "state_tocm4",  "Get or reset toCM4 state: state_tocm4 <get|reset>",    true,   NULL, CMD_StateToCM4     },

    { NULL, 			"collect_data", "Collect data: collect_data <type> <sample>", 			true,   NULL, CMD_CollectData 	 },
    { NULL, 			"pull_data", 	"Pull data status: pull_data", 							false,  NULL, CMD_PullData 		 },
    { NULL, 			"slavespi_rst", "Reset SPI Slave Device to initial state", 				false, 	NULL, CMD_SPISlaveRST 	 },
    { NULL, 			"master_read",  "Read data via SPI6 Master: master_read <size>", 		true,   NULL, CMD_MasterRead 	 },

	{ NULL,	"cm4_test_connection", 			"-",													false,  NULL, CMD_DevCM4TestConnection},

	{ NULL,	"exp_please_reset", 			"-",													false,  NULL, CMD_DevEXPPleaseReset},
	{ NULL, "test_connection", 				"Send TEST_CONNECTION_CMD with a 32-bit value", 		true, 	NULL, CMD_DevTestConnection },

	{ NULL, "exp_set_rtc", 					"exp_set_rtc - Use OBC RTC",							false,  NULL, CMD_DevEXPSetRTC},
	{ NULL, "ntc_set_control", 				"ntc_set_control <control mask>",						true,   NULL, CMD_DevNTCSetControl},
	{ NULL, "set_temp_profile", 			"<ref_t> <min_t> <max_t> <ntc_pri> <ntc_sec> <auto_rcv> <tec_mask> <heater_mask> <tec_vol> <heater_duty>",
																									true, 	NULL, CMD_DevSetTempProfile },
	{ NULL, "start_temp_profile", 			"Send START_TEMP_PROFILE_CMD", 							false, 	NULL, CMD_DevStartTempProfile },
	{ NULL, "stop_temp_profile", 			"Send STOP_TEMP_PROFILE_CMD", 							false, 	NULL, CMD_DevStopTempProfile },
	{ NULL, "set_override_tec_profile", 	"set_override_tec_profile <itv> <index> <vol>", 		true, 	NULL, CMD_DevSetOverrideTecProfile },
	{ NULL, "start_override_tec_profile", 	"Send START_OVERRIDE_TEC_PROFILE_CMD", 					false, 	NULL, CMD_DevStartOverrideTecProfile },
	{ NULL, "stop_override_tec_profile", 	"Send STOP_OVERRIDE_TEC_PROFILE_CMD", 					false, 	NULL, CMD_DevStopOverrideTecProfile },
	{ NULL, "set_sampling_profile", 		"set_sampling_profile <rate> <pre> <in> <post>", 		true, 	NULL, CMD_DevSetSamplingProfile },
	{ NULL, "set_laser_intensity", 			"set_laser_intensity <intensity>", 						true, 	NULL, CMD_DevSetLaserIntensity },
	{ NULL, "set_position", 				"set_position <position>", 								true, 	NULL, CMD_DevSetPosition },
	{ NULL, "start_sampling_cycle", 		"Send START_SAMPLING_CYCLE_CMD", 						false, 	NULL, CMD_DevStartSamplingCycle },
	{ NULL, "get_info_sample", 				"Send GET_INFO_SAMPLE_CMD", 							false, 	NULL, CMD_DevGetInfoSample },
	{ NULL, "get_chunk", 					"get_chunk <num_chunk>", 								true,	NULL, CMD_DevGetChunk },
	{ NULL, "get_chunk_crc", 				"get_chunk_crc <num_chunk>", 							true,	NULL, CMD_DevGetCRCChunk },
	{ NULL, "set_ext_laser_profile", 		"set_ext_laser_profile <intensity>", 					true, 	NULL, CMD_DevSetExtLaserProfile },
	{ NULL, "turn_on_ext_laser", 			"turn_on_ext_laser <position>", 						true, 	NULL, CMD_DevTurnOnExtLaser },
	{ NULL, "turn_off_ext_laser", 			"Send TURN_OFF_EXT_LASER_CMD", 							false, 	NULL, CMD_DevTurnOffExtLaser },
	{ NULL, "set_laser_int",				"set_laser_int <pos> <percent>",						true, 	NULL, CMD_DevSetLaserInt },
	{ NULL, "set_laser_ext",				"set_laser_ext <pos> <percent>", 						true, 	NULL, CMD_DevSetLaserExt },
	{ NULL, "get_current",					"-", 													false, 	NULL, CMD_DevGetCurrentLaser },
	{ NULL, "custom_cmd", 					"send_custom_cmd <string>", 							true, 	NULL, CMD_DevCustomCommand },

	{ NULL, "script_manager", 				"-", 													false, 	NULL, CMD_DevScriptManager},
	{ NULL, "erase_script", 				"-", 													false, 	NULL, CMD_DevEraseScript},
	{ NULL, "lwl_debug", 					"-", 													false, 	NULL, CMD_DevLogManagerDebug},
	{ NULL, "lwl_log", 						"-", 													false, 	NULL, CMD_DevLogManagerLog},
	{ NULL, "alivecm4_log", 				"-", 													false, 	NULL, CMD_DevCM4KeepAliveStatus},

	{ NULL, "testcase", 					"Run step-by-step testcase: testcase <mode>", 			true, 	NULL, CMD_Testcase },


};
/*************************************************
 *                 External Declarations         *
 *************************************************/

/*************************************************
 *             Command List Function             *
 *************************************************/
extern uint32_t _scustom_data;
extern uint32_t _ecustom_data;

extern uint8_t g_simple_ram_d3_buffer[DATA_CHUNK_SIZE];
extern MODFSP_Data_t cm4_protocol;

#define RAM_D2_200KB_START ((uint8_t*)&_scustom_data)
#define RAM_D3_START ((uint8_t*)0x38000000)
#define RAM_D2_200KB_SIZE  (200 * 1024)  // 200KB

static uint16_t UpdateCRC16_XMODEM(uint16_t crc, uint8_t byte) {
    const uint16_t polynomial = 0x1021; // CRC16 XMODEM
    crc ^= (uint16_t)byte << 8;
    for (uint8_t bit = 0; bit < 8; bit++) {
        if (crc & 0x8000) {
            crc = (crc << 1) ^ polynomial;
        } else {
            crc <<= 1;
        }
    }
    return crc;
}


static void CMD_RamFill(EmbeddedCli *cli, char *args, void *context) {
    const char *arg1 = embeddedCliGetToken(args, 1); // option (1, 2, 3)
    const char *arg2 = embeddedCliGetToken(args, 2); // size (byte)
    char buffer[100];
    uint16_t crc = 0x0000;

    if (arg1 == NULL || arg2 == NULL) {
        embeddedCliPrint(cli, "Usage: ram_fill <1|2|3> <size> (1: 0-255, 2: ASCII, 3: random, size: 1-200KB)");
        return;
    }

    int option = atoi(arg1);
    uint32_t size = (uint32_t)strtoul(arg2, NULL, 0);

    if (size < 1 || size > RAM_D2_200KB_SIZE) {
        snprintf(buffer, sizeof(buffer), "Invalid size. Must be 1 to %lu bytes.", (unsigned long)RAM_D2_200KB_SIZE);
        embeddedCliPrint(cli, buffer);
        return;
    }

    if (toCM4_GetState() != TOCM4_IDLE) {
        snprintf(buffer, sizeof(buffer), "Cannot fill RAM. Current state: %s",
                 toCM4_GetState() == TOCM4_BUSY ? "BUSY" :
                 toCM4_GetState() == TOCM4_READYSEND ? "READYSEND" : "ERROR");
        embeddedCliPrint(cli, buffer);
        return;
    }

    toCM4_SetState(TOCM4_BUSY);

    switch (option) {
        case 1:  // 0-255
            for (uint32_t i = 0; i < size; i++) {
                uint8_t value = (uint8_t)(i % 256);
                RAM_D2_200KB_START[i] = value;
                crc = UpdateCRC16_XMODEM(crc, value);
            }
            snprintf(buffer, sizeof(buffer), "Filled %lu bytes with pattern 0-255 repeating", (unsigned long)size);
            break;

        case 2:  // ASCII 0x20-0x7F
            for (uint32_t i = 0; i < size; i++) {
                uint8_t value = (uint8_t)(0x20 + (i % (0x7F - 0x20 + 1)));
                RAM_D2_200KB_START[i] = value;
                crc = UpdateCRC16_XMODEM(crc, value);
            }
            snprintf(buffer, sizeof(buffer), "Filled %lu bytes with ASCII pattern (0x20-0x7F)", (unsigned long)size);
            break;

        case 3:  // Random
            for (uint32_t i = 0; i < size; i++) {
                uint8_t value = (uint8_t)(rand() % 256);
                RAM_D2_200KB_START[i] = value;
                crc = UpdateCRC16_XMODEM(crc, value);
            }
            snprintf(buffer, sizeof(buffer), "Filled %lu bytes with random bytes", (unsigned long)size);
            break;

        default:
            embeddedCliPrint(cli, "Invalid option. Use: ram_fill <1|2|3> <size>");
            toCM4_SetState(TOCM4_IDLE);
            return;
    }

    vTaskDelay(pdMS_TO_TICKS(1000));

    toCM4_SetState(TOCM4_READYSEND);

    embeddedCliPrint(cli, buffer);
    snprintf(buffer, sizeof(buffer), "Start of RAM_D2 (0x%08lX): 0x%02X",
             (uint32_t)RAM_D2_200KB_START, RAM_D2_200KB_START[0]);
    embeddedCliPrint(cli, buffer);
    snprintf(buffer, sizeof(buffer), "End of RAM_D2 (0x%08lX): 0x%02X",
             (uint32_t)(RAM_D2_200KB_START + size - 1),
             RAM_D2_200KB_START[size - 1]);
    embeddedCliPrint(cli, buffer);
    snprintf(buffer, sizeof(buffer), "CRC16-XMODEM: 0x%04X", crc);
    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}

static void CMD_RamDump(EmbeddedCli *cli, char *args, void *context) {
    const char *arg1 = embeddedCliGetToken(args, 1); // size
    char buffer[100];
    const uint32_t bytes_per_line = 16;
    uint32_t byte_count = 0;
    uint16_t crc = 0x0000;

    if (arg1 == NULL) {
        embeddedCliPrint(cli, "Usage: ram_dump <size> (size: 1-200KB)");
        return;
    }

    uint32_t size = (uint32_t)strtoul(arg1, NULL, 0);

    snprintf(buffer, sizeof(buffer), "Dumping %lu bytes of RAM_D3 contents:", (unsigned long)size);
    embeddedCliPrint(cli, buffer);

    snprintf(buffer, sizeof(buffer),
             "Base address: 0x%08lX, data[0] @ %p",
             (unsigned long)(uintptr_t)g_simple_ram_d3_buffer,
             (void*)&g_simple_ram_d3_buffer[0]);
    embeddedCliPrint(cli, buffer);

    for (uint32_t i = 0; i < size; i += bytes_per_line) {
        snprintf(buffer, sizeof(buffer), "0x%08lX: ",
                 (unsigned long)((uintptr_t)g_simple_ram_d3_buffer + i));
        char *ptr = buffer + strlen(buffer);

        // In hex
        for (uint32_t j = 0; j < bytes_per_line && (i + j) < size; j++) {
            uint8_t value = g_simple_ram_d3_buffer[i + j];
            snprintf(ptr, sizeof(buffer) - (ptr - buffer), "%02X ", value);
            ptr += 3;
            byte_count++;
            crc = UpdateCRC16_XMODEM(crc, value);
        }

        *ptr++ = ' ';
        *ptr++ = '|';
        for (uint32_t j = 0; j < bytes_per_line && (i + j) < size; j++) {
            uint8_t c = g_simple_ram_d3_buffer[i + j];
            *ptr++ = (c >= 32 && c <= 126) ? c : '.';
        }
        *ptr = '\0';

        embeddedCliPrint(cli, buffer);
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    snprintf(buffer, sizeof(buffer),
             "Dump complete. Counted bytes: %lu, CRC16-XMODEM: 0x%04X",
             (unsigned long)byte_count, crc);
    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}


static void CMD_StateToCM4(EmbeddedCli *cli, char *args, void *context) {
    const char *arg1 = embeddedCliGetToken(args, 1);
    char buffer[100];

    if (arg1 == NULL) {
        embeddedCliPrint(cli, "Usage: state_tocm4 <get|reset>");
        return;
    }

    if (strcmp(arg1, "get") == 0) {
        toCM4_State_t state = toCM4_GetState();
        switch (state) {
            case TOCM4_ERROR:
                snprintf(buffer, sizeof(buffer), "toCM4 State: ERROR");
                break;
            case TOCM4_READYSEND:
                snprintf(buffer, sizeof(buffer), "toCM4 State: READYSEND");
                break;
            case TOCM4_BUSY:
                snprintf(buffer, sizeof(buffer), "toCM4 State: BUSY");
                break;
            case TOCM4_IDLE:
                snprintf(buffer, sizeof(buffer), "toCM4 State: IDLE");
                break;
            default:
                snprintf(buffer, sizeof(buffer), "toCM4 State: UNKNOWN");
                break;
        }
        embeddedCliPrint(cli, buffer);
    } else if (strcmp(arg1, "reset") == 0) {
        toCM4_SetState(TOCM4_IDLE);
        embeddedCliPrint(cli, "toCM4 State reset to IDLE");
    } else {
        embeddedCliPrint(cli, "Invalid option. Usage: state_tocm4 <get|reset>");
    }
    embeddedCliPrint(cli, "");
}

static void CMD_SPISlaveRST(EmbeddedCli *cli, char *args, void *context) {
    char buffer[100];

    SPI_SlaveDevice_t *device = SPI_SlaveDevice_GetHandle();
    if (!device->is_initialized) {
        embeddedCliPrint(cli, "SPI Slave Device not initialized");
        return;
    }

    Std_ReturnType ret = SPI_SlaveDevice_Disable();
    if (ret == E_OK) {
        snprintf(buffer, sizeof(buffer), "SPI Slave Device reset to IDLE state");
        embeddedCliPrint(cli, buffer);
    } else {
        snprintf(buffer, sizeof(buffer), "Failed to reset SPI Slave Device");
        embeddedCliPrint(cli, buffer);
    }

    embeddedCliPrint(cli, "");
}

static void CMD_CollectData(EmbeddedCli *cli, char *args, void *context) {
    const char *arg1 = embeddedCliGetToken(args, 1); // type
    const char *arg2 = embeddedCliGetToken(args, 2); // sample
    char buffer[100];

    if (arg1 == NULL || arg2 == NULL) {
        embeddedCliPrint(cli, "Usage: collect_data <type> <sample>");
        return;
    }

    uint8_t type = (uint8_t)atoi(arg1);
    uint32_t sample = (uint32_t)strtoul(arg2, NULL, 0);

    Std_ReturnType ret = SPI_SlaveDevice_CollectData(type, sample, (uint32_t)RAM_D2_200KB_START);
    if (ret == E_OK) {
        DataProcessContext_t ctx;
        if (SPI_SlaveDevice_GetDataInfo(&ctx) == E_OK) {
            snprintf(buffer, sizeof(buffer), "Collected %lu samples (type %d), size: %lu bytes, CRC: 0x%04X",
                     (unsigned long)ctx.sample, ctx.type, (unsigned long)ctx.data_size, ctx.crc);
            embeddedCliPrint(cli, buffer);
        }
    } else if (ret == E_BUSY) {
        snprintf(buffer, sizeof(buffer), "Type %d not implemented yet.", type);
        embeddedCliPrint(cli, buffer);
    } else {
        snprintf(buffer, sizeof(buffer), "Failed to collect data. Error code: %d", ret);
        embeddedCliPrint(cli, buffer);
    }

    embeddedCliPrint(cli, "");
}

static void CMD_PullData(EmbeddedCli *cli, char *args, void *context) {
    char buffer[100];
    toCM4_State_t state = SPI_SlaveDevice_GetCM4State();

    switch (state) {
        case TOCM4_IDLE:
            embeddedCliPrint(cli, "State: IDLE");
            break;
        case TOCM4_BUSY:
            embeddedCliPrint(cli, "State: BUSY");
            break;
        case TOCM4_READYSEND:
        {
            DataProcessContext_t ctx;
            if (SPI_SlaveDevice_GetDataInfo(&ctx) == E_OK) {
                snprintf(buffer, sizeof(buffer), "State: READYSEND, CRC: 0x%04X, Size: %lu bytes",
                         ctx.crc, (unsigned long)ctx.data_size);
                embeddedCliPrint(cli, buffer);
            } else {
                embeddedCliPrint(cli, "State: READYSEND, but no valid data context");
            }
            break;
        }
        case TOCM4_ERROR:
            embeddedCliPrint(cli, "State: ERROR");
            break;
        default:
            embeddedCliPrint(cli, "State: UNKNOWN");
            break;
    }

    embeddedCliPrint(cli, "");
}

#define READDONE_PIN_PORT				OBCOUT_EXPIN_READDONE_GPIO_Port
#define READDONE_PIN					OBCOUT_EXPIN_READDONE_Pin

static void CMD_MasterRead(EmbeddedCli *cli, char *args, void *context) {
    const char *arg1 = embeddedCliGetToken(args, 1); // size
    char buffer[100];

    if (arg1 == NULL) {
        embeddedCliPrint(cli, "Usage: master_read <size> (size: 1-200KB)");
        return;
    }

    uint32_t size = (uint32_t)strtoul(arg1, NULL, 0);

//    if (size < 1 || size > RAM_D2_200KB_SIZE) {
//        snprintf(buffer, sizeof(buffer), "Invalid size. Must be 1 to %lu bytes.", (unsigned long)RAM_D2_200KB_SIZE);
//        embeddedCliPrint(cli, buffer);
//        return;
//    }

//    if (toCM4_GetState() != TOCM4_IDLE) {
//        snprintf(buffer, sizeof(buffer), "Cannot read data. Current state: %s",
//                 toCM4_GetState() == TOCM4_BUSY ? "BUSY" :
//                 toCM4_GetState() == TOCM4_READYSEND ? "READYSEND" : "ERROR");
//        embeddedCliPrint(cli, buffer);
//        return;
//    }

    SPI_MasterDevice_t *device = SPI_MasterDevice_GetHandle();
    if (!device->is_initialized) {
        embeddedCliPrint(cli, "SPI Master Device not initialized");
        return;
    }

    Std_ReturnType ret = SPI_MasterDevice_ReadDMA((uint32_t)g_simple_ram_d3_buffer, size);

    LL_GPIO_SetOutputPin(READDONE_PIN_PORT, READDONE_PIN);

    if (ret == E_OK) {
        uint16_t crc = 0x0000;
        for (uint32_t i = 0; i < size; i++) {
            crc = UpdateCRC16_XMODEM(crc, g_simple_ram_d3_buffer[i]);
        }
        snprintf(buffer, sizeof(buffer), "Read %lu bytes via SPI6 Master, CRC: 0x%04X",
                 (unsigned long)size, crc);
        embeddedCliPrint(cli, buffer);
    } else {
        snprintf(buffer, sizeof(buffer), "Failed to read data. Error code: %d", ret);
        embeddedCliPrint(cli, buffer);
    }

    embeddedCliPrint(cli, "");
}

static void CMD_ClearCLI(EmbeddedCli *cli, char *args, void *context) {
    char buffer[10];

    LL_GPIO_ResetOutputPin(READDONE_PIN_PORT, READDONE_PIN);


    snprintf(buffer, sizeof(buffer), "\33[2J");
    embeddedCliPrint(cli, buffer);
}

static void CMD_RtcSet(EmbeddedCli *cli, char *args, void *context) {
    const char *arg1 = embeddedCliGetToken(args, 1); // hour
    const char *arg2 = embeddedCliGetToken(args, 2); // minute
    const char *arg3 = embeddedCliGetToken(args, 3); // second
    const char *arg4 = embeddedCliGetToken(args, 4); // day
    const char *arg5 = embeddedCliGetToken(args, 5); // month
    const char *arg6 = embeddedCliGetToken(args, 6); // year

    char buffer[100];
    if (arg1 == NULL || arg2 == NULL || arg3 == NULL ||
        arg4 == NULL || arg5 == NULL || arg6 == NULL) {
        snprintf(buffer, sizeof(buffer),
                 "Usage: rtc_set <hour> <minute> <second> <day> <month> <year>");
        embeddedCliPrint(cli, buffer);
        return;
    }

    int hour   = atoi(arg1);
    int minute = atoi(arg2);
    int second = atoi(arg3);
    int day    = atoi(arg4);
    int month  = atoi(arg5);
    int year   = atoi(arg6);

    if (hour < 0 || hour > 23) {
        embeddedCliPrint(cli, "Invalid hour (must be 0-23). Please enter again.");
        return;
    }
    if (minute < 0 || minute > 59) {
        embeddedCliPrint(cli, "Invalid minute (must be 0-59). Please enter again.");
        return;
    }
    if (second < 0 || second > 59) {
        embeddedCliPrint(cli, "Invalid second (must be 0-59). Please enter again.");
        return;
    }
    if (day < 1 || day > 31) {
        embeddedCliPrint(cli, "Invalid day (must be 1-31). Please enter again.");
        return;
    }
    if (month < 1 || month > 12) {
        embeddedCliPrint(cli, "Invalid month (must be 1-12). Please enter again.");
        return;
    }
    if (year < 0 || year > 99) {
        embeddedCliPrint(cli, "Invalid year (must be 2 digits, e.g., 25 for 2025). Please enter again.");
        return;
    }

    s_DateTime dt;
    dt.hour   = (uint8_t)hour;
    dt.minute = (uint8_t)minute;
    dt.second = (uint8_t)second;
    dt.day    = (uint8_t)day;
    dt.month  = (uint8_t)month;
    dt.year   = (uint8_t)year;

    Utils_SetRTC(&dt);
    RV3129_HandleTypeDef *hrtc = RV3129_GetHandle();
    RV3129_SetTime(hrtc, &dt);

    snprintf(buffer, sizeof(buffer),
             "--> RTC set to %02d:%02d:%02d, %02d/%02d/20%02d",
             dt.hour, dt.minute, dt.second, dt.day, dt.month, dt.year);
    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}

static void CMD_RtcGet(EmbeddedCli *cli, char *args, void *context) {
    const char *mode = embeddedCliGetToken(args, 1);
    char buffer[100];

    if (mode == NULL) {
        snprintf(buffer, sizeof(buffer), "Usage: rtc_get <hard|soft|work|epoch|all>");
        embeddedCliPrint(cli, buffer);
        return;
    }

    //Hard
    if (strcmp(mode, "hard") == 0) {
        s_DateTime currentTime;
        RV3129_HandleTypeDef *hrtc = RV3129_GetHandle();
        if (RV3129_GetTime(hrtc, &currentTime) == E_OK) {
            int16_t temp;
            RV3129_GetTemp(hrtc, &temp);
            snprintf(buffer, sizeof(buffer),
                     "--> Hard RTC: Time: %02d:%02d:%02d, Date: %02d/%02d/20%02d, Temp: %d",
                     currentTime.hour, currentTime.minute, currentTime.second,
                     currentTime.day, currentTime.month, currentTime.year, temp);
            embeddedCliPrint(cli, buffer);
        } else {
            embeddedCliPrint(cli, "Failed to get hard RTC\r\n");
        }
    } else if (strcmp(mode, "soft") == 0) {
        s_DateTime rtc;
        Utils_GetRTC(&rtc);
        snprintf(buffer, sizeof(buffer),
                 "--> Soft RTC: Time: %02d:%02d:%02d, Date: %02d/%02d/20%02d",
                 rtc.hour, rtc.minute, rtc.second,
                 rtc.day, rtc.month, rtc.year);
        embeddedCliPrint(cli, buffer);
    } else if (strcmp(mode, "work") == 0) {
        uint32_t days = 0;
        uint8_t hours = 0, minutes = 0, seconds = 0;
        Utils_GetWorkingTime(&days, &hours, &minutes, &seconds);
        snprintf(buffer, sizeof(buffer),
                        "--> Working Uptime: Time: %02d:%02d:%02d, Days: %d",
                        hours, minutes, seconds, (uint8_t)days);
        embeddedCliPrint(cli, buffer);
    }else if (strcmp(mode, "epoch") == 0) {
        uint32_t epoch = Utils_GetEpoch();
        snprintf(buffer, sizeof(buffer), "--> Epoch: %lu", (unsigned long)epoch);
        embeddedCliPrint(cli, buffer);
    } else if (strcmp(mode, "all") == 0) {
        s_DateTime currentTime;
        RV3129_HandleTypeDef *hrtc = RV3129_GetHandle();
        if (RV3129_GetTime(hrtc, &currentTime) == E_OK) {
            int16_t temp;
            RV3129_GetTemp(hrtc, &temp);
            snprintf(buffer, sizeof(buffer),
                     "--> Hard RTC: Time: %02d:%02d:%02d, Date: %02d/%02d/20%02d, Temp: %d",
                     currentTime.hour, currentTime.minute, currentTime.second,
                     currentTime.day, currentTime.month, currentTime.year, temp);
            embeddedCliPrint(cli, buffer);
        } else {
            embeddedCliPrint(cli, "Failed to get hard RTC");
        }
        // Soft RTC
        s_DateTime rtc;
        Utils_GetRTC(&rtc);
        snprintf(buffer, sizeof(buffer),
                 "--> Soft RTC: Time: %02d:%02d:%02d, Date: %02d/%02d/20%02d",
                 rtc.hour, rtc.minute, rtc.second,
                 rtc.day, rtc.month, rtc.year);
        embeddedCliPrint(cli, buffer);
        // Working uptime:
        uint32_t days = 0;
        uint8_t hours = 0, minutes = 0, seconds = 0;
        Utils_GetWorkingTime(&days, &hours, &minutes, &seconds);
        snprintf(buffer, sizeof(buffer),
                        "--> Working Uptime: Time: %02d:%02d:%02d, Days: %d",
                        hours, minutes, seconds, (uint8_t)days);
        embeddedCliPrint(cli, buffer);
        // Epoch
        uint32_t epoch = Utils_GetEpoch();
        snprintf(buffer, sizeof(buffer), "--> Epoch: %lu", (unsigned long)epoch);
        embeddedCliPrint(cli, buffer);
    } else {
        snprintf(buffer, sizeof(buffer), "Unknown mode. Use: rtc_get <hard|soft|work|epoch|all>");
        embeddedCliPrint(cli, buffer);
    }
    embeddedCliPrint(cli, "");
}

static void CMD_RtcSetEpoch(EmbeddedCli *cli, char *args, void *context) {
    const char *arg1 = embeddedCliGetToken(args, 1);
    char buffer[100];
    if (arg1 == NULL) {
        snprintf(buffer, sizeof(buffer), "Usage: rtc_setepoch <epoch>");
        embeddedCliPrint(cli, buffer);
        return;
    }

    uint32_t epoch = (uint32_t)strtoul(arg1, NULL, 0);

    if (epoch < EPOCH_OFFSET_UNIX) {
        snprintf(buffer, sizeof(buffer), "Invalid epoch. Must be >= %lu", (unsigned long)EPOCH_OFFSET_UNIX);
        embeddedCliPrint(cli, buffer);
        return;
    }

    Utils_SetEpoch(epoch);

    s_DateTime dt;
    EpochToDateTime(epoch - EPOCH_OFFSET_UNIX, &dt);
    RV3129_HandleTypeDef *hrtc = RV3129_GetHandle();
    RV3129_SetTime(hrtc, &dt);

    snprintf(buffer, sizeof(buffer),
             "--> RTC set to %02d:%02d:%02d, %02d/%02d/20%02d",
             dt.hour, dt.minute, dt.second, dt.day, dt.month, dt.year);
    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}


static void CMD_FramWrite(EmbeddedCli *cli, char *args, void *context) {
    const char *arg1 = embeddedCliGetToken(args, 1); // Address
    const char *arg2 = embeddedCliGetToken(args, 2); // Value

    char buffer[100];

    if (arg1 == NULL || arg2 == NULL) {
        snprintf(buffer, sizeof(buffer), "Usage: fram_write [address] [value]");
        embeddedCliPrint(cli, buffer);
        return;
    }

    uint16_t address = (uint16_t)strtol(arg1, NULL, 0);
    uint8_t value = (uint8_t)strtol(arg2, NULL, 0);

    FRAM_SPI_HandleTypeDef *hfram = FRAM_SPI_GetHandle();
    if (FRAM_SPI_WriteMem(hfram, address, &value, 1) == E_OK) {
        snprintf(buffer, sizeof(buffer), "Write OK: Addr 0x%04X = 0x%02X", address, value);
    } else {
        snprintf(buffer, sizeof(buffer), "FRAM Write Error at 0x%04X", address);
    }

    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}

static void CMD_FramRead(EmbeddedCli *cli, char *args, void *context) {
    const char *arg1 = embeddedCliGetToken(args, 1); // Address

    char buffer[100];

    if (arg1 == NULL) {
        snprintf(buffer, sizeof(buffer), "Usage: fram_read [address]");
        embeddedCliPrint(cli, buffer);
        return;
    }

    uint16_t address = (uint16_t)strtol(arg1, NULL, 0);
    uint8_t readData = 0;

    FRAM_SPI_HandleTypeDef *hfram = FRAM_SPI_GetHandle();
    if (FRAM_SPI_ReadMem(hfram, address, &readData, 1) == E_OK) {
        snprintf(buffer, sizeof(buffer), "Read OK: Addr 0x%04X = 0x%02X", address, readData);
    } else {
        snprintf(buffer, sizeof(buffer), "FRAM Read Error at 0x%04X", address);
    }

    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}

static void CMD_ls(EmbeddedCli *cli, char *args, void *context) {
    FS_ListFiles_path(cli);
    embeddedCliPrint(cli, "");
}

static void CMD_sd_lockin(EmbeddedCli *cli, char *args, void *context) {
	SD_Lockin();
	SDMMC1_Init();
    embeddedCliPrint(cli, "SD filesystem locked-in");
	Std_ReturnType ret = Link_SDFS_Driver();
	if(ret != E_OK){
        embeddedCliPrint(cli, "[Link FATFS Fail]");
	}else{
        embeddedCliPrint(cli, "[Link FATFS Successfully]");
	}
    embeddedCliPrint(cli, "");
}

static void CMD_sd_release(EmbeddedCli *cli, char *args, void *context) {
	SDMMC1_DeInit();
    SD_Release();
    embeddedCliPrint(cli, "SD filesystem released");
    embeddedCliPrint(cli, "");
}

static void CMD_vim_bypass(EmbeddedCli *cli, char *args, void *context) {
    const char *filename = embeddedCliGetToken(args, 1);
    const char *content = embeddedCliGetToken(args, 2);
    char buffer[128];

    if (filename == NULL || content == NULL) {
        embeddedCliPrint(cli, "Usage: vim <filename> \"content\"");
        return;
    }

    if (Vim_SDFS(cli, filename, content) == 0) {
        snprintf(buffer, sizeof(buffer), "Content written to %s", filename);
        embeddedCliPrint(cli, buffer);
    }
    embeddedCliPrint(cli, "");
}

static void CMD_vim(EmbeddedCli *cli, char *args, void *context) {
    const char *filename = embeddedCliGetToken(args, 1);
    const char *content = embeddedCliGetToken(args, 2);
    char buffer[128];

    if (filename == NULL || content == NULL) {
        embeddedCliPrint(cli, "Usage: vim <filename> \"content\"");
        return;
    }

    size_t content_len = strlen(content);
    if (content_len > (8 * 1024) ){
        embeddedCliPrint(cli, "Content exceeds 8KB limit");
        return;
    }

    if (FS_Write_Direct(filename, (uint8_t*)content, content_len) == E_OK) {
        snprintf(buffer, sizeof(buffer), "Content written to %s", filename);
        embeddedCliPrint(cli, buffer);
    } else {
        embeddedCliPrint(cli, "Failed to write to file");
    }
    embeddedCliPrint(cli, "");
}

static void CMD_cat(EmbeddedCli *cli, char *args, void *context) {
    const char *filename = embeddedCliGetToken(args, 1);

    if (filename == NULL) {
        embeddedCliPrint(cli, "Usage: cat <filename>");
        return;
    }

    Cat_SDFS(cli, filename);
    embeddedCliPrint(cli, "");
}

static void CMD_Cm4Rst(EmbeddedCli *cli, char *args, void *context) {
    GPIO_SetLow(CM4_RST_Port, CM4_RST_Pin);
    vTaskDelay(pdMS_TO_TICKS(100));
    GPIO_SetHigh(CM4_RST_Port, CM4_RST_Pin);
    embeddedCliPrint(cli, "CM4 reset pulse triggered.");
    embeddedCliPrint(cli, "");
}

static void CMD_Cm4Dis(EmbeddedCli *cli, char *args, void *context) {
    GPIO_SetHigh(CM4_ENA_Port, CM4_ENA_Pin);
    embeddedCliPrint(cli, "CM4 power disabled (enable driven low).");
    embeddedCliPrint(cli, "");
}

static void CMD_Cm4Ena(EmbeddedCli *cli, char *args, void *context) {
    GPIO_SetLow(CM4_ENA_Port, CM4_ENA_Pin);
    embeddedCliPrint(cli, "CM4 power enabled (enable driven high).");
    embeddedCliPrint(cli, "");
}

static void CMD_ExpForward(EmbeddedCli *cli, char *args, void *context) {
    const char *param = embeddedCliGetToken(args, 1);
    if (param == NULL) {
        embeddedCliPrint(cli, "Usage: exp_forward <cm4|usb|normal>");
        return;
    }

    if (strcmp(param, "cm4") == 0) {
        embeddedCliPrint(cli, "Forward mode enabled: CM4 <-> EXP forwarding.");
        ForwardMode_Set(FORWARD_MODE_UART);
    } else if (strcmp(param, "usb") == 0) {
        embeddedCliPrint(cli, "Forward mode enabled: CDC <-> EXP forwarding.");
        ForwardMode_Set(FORWARD_MODE_USB);
    } else if (strcmp(param, "normal") == 0) {
        embeddedCliPrint(cli, "Forward mode disabled. Operating in NORMAL mode.");
        ForwardMode_Set(FORWARD_MODE_NORMAL);
    } else {
        embeddedCliPrint(cli, "Invalid parameter. Usage: exp_forward <cm4|usb|normal>");
        return;
    }

    embeddedCliPrint(cli, "");
}

static void CMD_ExpListen(EmbeddedCli *cli, char *args, void *context) {
    const char *param = embeddedCliGetToken(args, 1);
    if (param == NULL) {
        embeddedCliPrint(cli, "Usage: exp_listen <cm4|usb|off>");
        return;
    }

    if (strcmp(param, "cm4") == 0) {
        ForwardMode_Set(FORWARD_MODE_LISTEN_CM4);
        embeddedCliPrint(cli, "Listen mode enabled: EXP data from UART7 will be sent to UART_DEBUG.");
    } else if (strcmp(param, "usb") == 0) {
        ForwardMode_Set(FORWARD_MODE_LISTEN_USB);
        embeddedCliPrint(cli, "Listen mode enabled: EXP data from UART7 will be sent to CDC.");
    } else if (strcmp(param, "off") == 0) {
        ForwardMode_Set(FORWARD_MODE_NORMAL);
        embeddedCliPrint(cli, "Listen mode disabled. Operating in NORMAL mode.");
    } else {
        embeddedCliPrint(cli, "Invalid parameter. Usage: exp_listen <cm4|usb|off>");
    }
    embeddedCliPrint(cli, "");
}

static void CMD_ExpSend(EmbeddedCli *cli, char *args, void *context) {
    const char *msg = embeddedCliGetToken(args, 1);
    if (msg == NULL) {
        embeddedCliPrint(cli, "Usage: exp_send \"message\"");
        return;
    }
    size_t len = strlen(msg);
    UART_Driver_Write(UART_EXP, '\r');
    for (size_t i = 0; i < len; i++) {
        UART_Driver_Write(UART_EXP, (uint8_t)msg[i]);
    }
    UART_Driver_Write(UART_EXP, '\r');
    embeddedCliPrint(cli, "Message sent to EXP via UART7.");
    embeddedCliPrint(cli, "");
}

void callback_every(void *context) {
    EmbeddedCli *cli = (EmbeddedCli *)context;
    embeddedCliPrint(cli, "Callback Every");
}

void callback_moment(void *context) {
    EmbeddedCli *cli = (EmbeddedCli *)context;
    embeddedCliPrint(cli, "Callback Moment");
}

void callback_countdown(void *context) {
    EmbeddedCli *cli = (EmbeddedCli *)context;
    embeddedCliPrint(cli, "Callback Countdown");
}

static void CMD_AliveCheck(EmbeddedCli *cli, char *args, void *context) {
//	Utils_Cronjob_SetEvery(EVERY_SECOND, 10, 0, callback_every, cli, 0);
//	Utils_Cronjob_SetMoment(10, 20, 30, 0, callback_moment, cli, 1);
//	Utils_Cronjob_SetCountdown(20, 0, callback_countdown, cli, 2);

    embeddedCliPrint(cli, "Hello from OBC-STM32. Status: OK");
    embeddedCliPrint(cli, "Sparrow call Eagle. Code: OK");
    embeddedCliPrint(cli, "Eagle clear, end. Code: OK");
    embeddedCliPrint(cli, "");
}

static void CMD_RtosCheck(EmbeddedCli *cli, char *args, void *context) {
    TaskStatus_t taskStatusArray[22];
    UBaseType_t arraySize = 22;
    UBaseType_t totalTasks;
    char buffer[256];

    UBaseType_t numTasks = uxTaskGetNumberOfTasks();
        snprintf(buffer, sizeof(buffer), "Number of tasks running: %lu", (unsigned long)numTasks);
        embeddedCliPrint(cli, buffer);

    totalTasks = uxTaskGetSystemState(taskStatusArray, arraySize, NULL);

    if (totalTasks == 0) {
        embeddedCliPrint(cli, "No tasks found or error occurred.");
        embeddedCliPrint(cli, "");
        return;
    }

    snprintf(buffer, sizeof(buffer), "Total Tasks: %lu", (unsigned long)totalTasks);
    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "Task Name          State     Prio  Stack Left");
    embeddedCliPrint(cli, "----------------------------------------");

	for (UBaseType_t i = 0; i < totalTasks; i++) {
		const char *stateStr;
		switch (taskStatusArray[i].eCurrentState) {
		case eRunning:
			stateStr = "Running";
			break;
		case eReady:
			stateStr = "Ready";
			break;
		case eBlocked:
			stateStr = "Blocked";
			break;
		case eSuspended:
			stateStr = "Suspend";
			break;
		case eDeleted:
			stateStr = "Deleted";
			break;
		default:
			stateStr = "Unknown";
			break;
		}

		snprintf(buffer, sizeof(buffer), "%-18s %-10s %-4lu %10lu",
				taskStatusArray[i].pcTaskName, stateStr,
				taskStatusArray[i].uxCurrentPriority,
				taskStatusArray[i].usStackHighWaterMark);
		embeddedCliPrint(cli, buffer);
	}

	embeddedCliPrint(cli, "----------------------------------------");
	embeddedCliPrint(cli, "");
}

static TimerHandle_t logoutTimer = NULL;
static void LogoutTimerCallback(TimerHandle_t xTimer) {
    Shield_Reset(&auth_usb);
}
static void CMD_LogOut(EmbeddedCli *cli, char *args, void *context) {
    embeddedCliPrint(cli, "Logging out...");
    if (logoutTimer == NULL) {
        logoutTimer = xTimerCreate("LogoutTimer", pdMS_TO_TICKS(100), pdFALSE, NULL, LogoutTimerCallback);
    }
    if (logoutTimer != NULL) {
        xTimerStart(logoutTimer, 0);
    }
}

static void CMD_PwdChange(EmbeddedCli *cli, char *args, void *context) {
    const char *new_password = embeddedCliGetToken(args, 1);
    if (new_password == NULL) {
        embeddedCliPrint(cli, "Usage: pwd_change <new_password>");
        return;
    }

	ShieldAuthState_t auth_state;
	auth_state = Shield_GetState(&auth_usb);

    if (auth_state == AUTH_ADMIN) {
    	size_t pwd_len = strlen(new_password);
        if (pwd_len > MAX_PASSWORD_LEN) {
        	embeddedCliPrint(cli, "Password too long (max 16 characters).");
            return;
        }
        FRAM_SPI_HandleTypeDef *hfram = FRAM_SPI_GetHandle();
        uint8_t len = (uint8_t)pwd_len;
        if (FRAM_SPI_WriteMem(hfram, FRAM_USER_PWD_LEN_ADDR, &len, 1) != E_OK) {
        	embeddedCliPrint(cli, "Failed to write password length to FRAM.\r\n");
            return;
        }
        if (pwd_len > 0) {
            if (FRAM_SPI_WriteMem(hfram, FRAM_USER_PWD_ADDR, (uint8_t *)new_password, pwd_len) != E_OK) {
            	embeddedCliPrint(cli, "Failed to write password to FRAM.\r\n");
                return;
            }
        }
        embeddedCliPrint(cli, "User password updated successfully.");
    } else {
        embeddedCliPrint(cli, "Must be logged in as admin to change password.");
        return;
    }
    embeddedCliPrint(cli, "");
}

static void CMD_Dmesg(EmbeddedCli *cli, char *args, void *context) {
    const char *arg1 = embeddedCliGetToken(args, 1);
    char buffer[64];
    embeddedCliPrint(cli, "Dmesg - Logger Message:");

    if (arg1 == NULL) {
        embeddedCliPrint(cli, "--> Oldest >>");
        Dmesg_GetLogs(cli);
        embeddedCliPrint(cli, "--> Latest <<");
    } else {
        size_t N = (size_t)strtoul(arg1, NULL, 10);
        snprintf(buffer, sizeof(buffer), "Latest %lu Logs:", (unsigned long)N);
        embeddedCliPrint(cli, buffer);
        Dmesg_GetLatestN(N, cli);
    }

    embeddedCliPrint(cli, "");
}
static void CMD_Reset(EmbeddedCli *cli, char *args, void *context) {
	NVIC_SystemReset();
    embeddedCliPrint(cli, "");
}

/*************************************************
 * Command Function "Dev"            *
 *************************************************/
static void CMD_DevEXPPleaseReset(EmbeddedCli *cli, char *args, void *context)
{
    char buffer[100];
    MIN_Send_PLEASE_RESET_CMD();
    snprintf(buffer, sizeof(buffer), "Sent EXP-PLEASE_RESET_CMD");
    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}


static void CMD_DevTestConnection(EmbeddedCli *cli, char *args, void *context)
{
    const char *arg1 = embeddedCliGetToken(args, 1);
    char buffer[100];
    if (arg1 == NULL) {
        snprintf(buffer, sizeof(buffer), "Usage: test_connection <value(32bit)>");
        embeddedCliPrint(cli, buffer);
        return;
    }
    uint32_t value = (uint32_t)strtoul(arg1, NULL, 0);
    MIN_Send_TEST_CONNECTION_CMD(value);
    snprintf(buffer, sizeof(buffer), "Sent TEST_CONNECTION_CMD with value: %lu", (unsigned long)value);
    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}

static void CMD_DevCM4TestConnection(EmbeddedCli *cli, char *args, void *context)
{
    const char *arg1 = embeddedCliGetToken(args, 1);
    char buffer[100];
    if (arg1 == NULL) {
        snprintf(buffer, sizeof(buffer), "Usage: cm4_test_connection <value(32bit)>");
        embeddedCliPrint(cli, buffer);
        return;
    }

    uint32_t value = (uint32_t)strtoul(arg1, NULL, 0);

	uint8_t trigger_data[4];
	trigger_data[0] = (uint8_t)((value >> 24) & 0xFF);
	trigger_data[1] = (uint8_t)((value >> 16) & 0xFF);
	trigger_data[2] = (uint8_t)((value >> 8) & 0xFF);
	trigger_data[3] = (uint8_t)(value & 0xFF);

	MODFSP_Send(&cm4_protocol, 0x99,
					 trigger_data, sizeof(trigger_data));

    snprintf(buffer, sizeof(buffer), "Sent CM4_TEST_CONNECTION_CMD with value: %lu", (unsigned long)value);
    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}


static void CMD_DevEXPSetRTC(EmbeddedCli *cli, char *args, void *context)
{
    char buffer[100];
    MIN_Send_SET_WORKING_RTC_CMD();
    s_DateTime now;
    Utils_GetRTC(&now);

    snprintf(buffer, sizeof(buffer), "Sent SET_WORKING_RTC_CMD with %u-%u:%u:%u", now.day, now.hour, now.minute, now.second);
    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}

static void CMD_DevNTCSetControl(EmbeddedCli *cli, char *args, void *context)
{
    char buffer[150];
    uint8_t ntc_control_byte = 0;

    const char *arg[8];
    for (int i = 0; i < 8; i++) {
        arg[i] = embeddedCliGetToken(args, i + 1);
    }

    for (int i = 0; i < 8; i++) {
        if (!arg[i]) {
            snprintf(buffer, sizeof(buffer), "Usage: set_ntc_control <ntc0[0:off/1:on]> <ntc1> ... <ntc7>");
            embeddedCliPrint(cli, buffer);
            return;
        }
    }

    for (int i = 0; i < 8; i++) {
        uint8_t enable_flag = (uint8_t)strtoul(arg[i], NULL, 0);
        if (enable_flag > 1) {
            snprintf(buffer, sizeof(buffer), "Invalid argument for ntc%d. Please use 0 (off) or 1 (on).", i);
            embeddedCliPrint(cli, buffer);
            return;
        }
        ntc_control_byte |= (enable_flag << i);
    }

    MIN_Send_SET_NTC_CONTROL_CMD(ntc_control_byte);

    snprintf(buffer, sizeof(buffer), "Sent SET_NTC_CONTROL_CMD with Data Hex: 0x%02X, NTC0: %d, NTC1: %d, NTC2: %d, NTC3: %d, NTC4: %d, NTC5: %d, NTC6: %d, NTC7: %d",
             ntc_control_byte,
             (ntc_control_byte >> 0) & 0x01,
             (ntc_control_byte >> 1) & 0x01,
             (ntc_control_byte >> 2) & 0x01,
             (ntc_control_byte >> 3) & 0x01,
             (ntc_control_byte >> 4) & 0x01,
             (ntc_control_byte >> 5) & 0x01,
             (ntc_control_byte >> 6) & 0x01,
             (ntc_control_byte >> 7) & 0x01);
    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}

static void CMD_DevSetTempProfile(EmbeddedCli *cli, char *args, void *context)
{
    char buffer[256];
    const char *arg1 = embeddedCliGetToken(args, 1);  // target_temp
    const char *arg2 = embeddedCliGetToken(args, 2);  // min_temp
    const char *arg3 = embeddedCliGetToken(args, 3);  // max_temp
    const char *arg4 = embeddedCliGetToken(args, 4);  // ntc_pri
    const char *arg5 = embeddedCliGetToken(args, 5);  // ntc_sec
    const char *arg6 = embeddedCliGetToken(args, 6);  // auto_recover
    const char *arg7 = embeddedCliGetToken(args, 7);  // tec_positions
    const char *arg8 = embeddedCliGetToken(args, 8);  // heater_positions
    const char *arg9 = embeddedCliGetToken(args, 9);  // tec_vol
    const char *arg10 = embeddedCliGetToken(args, 10); // heater_duty_cycle

    if (!arg1 || !arg2 || !arg3 || !arg4 || !arg5 || !arg6 || !arg7 || !arg8 || !arg9 || !arg10) {
        snprintf(buffer, sizeof(buffer), "Usage: set_temp_profile <ref_t> <min_t> <max_t> <ntc_pri> <ntc_sec> <auto_rcv> <tec_mask> <heater_mask> <tec_vol> <heater_duty>");
        embeddedCliPrint(cli, buffer);
        return;
    }

    uint16_t target_temp       = (uint16_t)strtoul(arg1, NULL, 0);
    uint16_t min_temp          = (uint16_t)strtoul(arg2, NULL, 0);
    uint16_t max_temp          = (uint16_t)strtoul(arg3, NULL, 0);
    uint8_t  ntc_pri           = (uint8_t)strtoul(arg4, NULL, 0);
    uint8_t  ntc_sec           = (uint8_t)strtoul(arg5, NULL, 0);
    uint8_t  auto_recover      = (uint8_t)strtoul(arg6, NULL, 0);
    uint8_t  tec_positions     = (uint8_t)strtoul(arg7, NULL, 0);
    uint8_t  heater_positions  = (uint8_t)strtoul(arg8, NULL, 0);
    uint16_t tec_vol           = (uint16_t)strtoul(arg9, NULL, 0);
    uint8_t  heater_duty_cycle = (uint8_t)strtoul(arg10, NULL, 0);

    MIN_Send_SET_TEMP_PROFILE_CMD(target_temp, min_temp, max_temp,
            ntc_pri, ntc_sec, auto_recover,
            tec_positions, heater_positions,
            tec_vol, heater_duty_cycle
        );

    snprintf(buffer, sizeof(buffer), "Sent SET_TEMP_PROFILE_CMD with params:%u, %u, %u, %u, %u, %u, %u, %u, %u, %u",
    		target_temp, min_temp, max_temp, ntc_pri, ntc_sec, auto_recover, tec_positions, heater_positions, tec_vol, heater_duty_cycle);
    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}

static void CMD_DevStartTempProfile(EmbeddedCli *cli, char *args, void *context)
{
    (void)args;
    (void)context;
    MIN_Send_START_TEMP_PROFILE_CMD();
    embeddedCliPrint(cli, "Sent START_TEMP_PROFILE_CMD");
    embeddedCliPrint(cli, "");
}

static void CMD_DevStopTempProfile(EmbeddedCli *cli, char *args, void *context)
{
    (void)args;
    (void)context;
    MIN_Send_STOP_TEMP_PROFILE_CMD();
    embeddedCliPrint(cli, "Sent STOP_TEMP_PROFILE_CMD");
    embeddedCliPrint(cli, "");
}

static void CMD_DevSetOverrideTecProfile(EmbeddedCli *cli, char *args, void *context)
{
    char buffer[100];
    const char *arg1 = embeddedCliGetToken(args, 1);
    const char *arg2 = embeddedCliGetToken(args, 2);
    const char *arg3 = embeddedCliGetToken(args, 3);

    if (!arg1 || !arg2 || !arg3) {
        snprintf(buffer, sizeof(buffer), "Usage: set_override_tec_profile <interval> <tec_index> <tec_vol>");
        embeddedCliPrint(cli, buffer);
        return;
    }

    uint16_t interval  = (uint16_t)strtoul(arg1, NULL, 0);
    uint8_t ovr_tec_index = (uint8_t)strtoul(arg2, NULL, 0);
    uint16_t ovr_tec_vol  = (uint16_t)strtoul(arg3, NULL, 0);

    MIN_Send_SET_OVERRIDE_TEC_PROFILE_CMD(interval, ovr_tec_index, ovr_tec_vol);
    snprintf(buffer, sizeof(buffer), "Sent SET_OVERRIDE_TEC_PROFILE_CMD with params: %u, %u, %u",
             interval, ovr_tec_index, ovr_tec_vol);
    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}

static void CMD_DevStartOverrideTecProfile(EmbeddedCli *cli, char *args, void *context)
{
    (void)args;
    (void)context;
    MIN_Send_START_OVERRIDE_TEC_PROFILE_CMD();
    embeddedCliPrint(cli, "Sent START_OVERRIDE_TEC_PROFILE_CMD");
    embeddedCliPrint(cli, "");
}

static void CMD_DevStopOverrideTecProfile(EmbeddedCli *cli, char *args, void *context)
{
    (void)args;
    (void)context;
    MIN_Send_STOP_OVERRIDE_TEC_PROFILE_CMD();
    embeddedCliPrint(cli, "Sent STOP_OVERRIDE_TEC_PROFILE_CMD");
    embeddedCliPrint(cli, "");
}

static void CMD_DevSetSamplingProfile(EmbeddedCli *cli, char *args, void *context)
{
    char buffer[128];
    const char *arg1 = embeddedCliGetToken(args, 1);
    const char *arg2 = embeddedCliGetToken(args, 2);
    const char *arg3 = embeddedCliGetToken(args, 3);
    const char *arg4 = embeddedCliGetToken(args, 4);

    if (!arg1 || !arg2 || !arg3 || !arg4) {
        snprintf(buffer, sizeof(buffer), "Usage: set_sampling_profile <rate> <pre> <in> <post>");
        embeddedCliPrint(cli, buffer);
        return;
    }

    uint32_t sample_rate = (uint32_t)strtoul(arg1, NULL, 0);
    uint16_t pre         = (uint16_t)strtoul(arg2, NULL, 0);
    uint16_t in          = (uint16_t)strtoul(arg3, NULL, 0);
    uint16_t post        = (uint16_t)strtoul(arg4, NULL, 0);


    MIN_Send_SET_PDA_PROFILE_CMD(sample_rate, pre, in, post);
    snprintf(buffer, sizeof(buffer), "Sent SET_SAMPLING_PROFILE_CMD with params:rate=%lu, pre=%u, in=%u, post=%u",
    		(unsigned long)sample_rate, pre, in, post );
    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}

static void CMD_DevSetLaserIntensity(EmbeddedCli *cli, char *args, void *context)
{
    char buffer[100];
    const char *arg1 = embeddedCliGetToken(args, 1);
    if (!arg1) {
        snprintf(buffer, sizeof(buffer), "Usage: set_laser_intensity <intensity>");
        embeddedCliPrint(cli, buffer);
        return;
    }
    uint8_t intensity = (uint8_t)strtoul(arg1, NULL, 0);
    MIN_Send_SET_LASER_INTENSITY_CMD(intensity);
    snprintf(buffer, sizeof(buffer), "Sent SET_LASER_INTENSITY_CMD with intensity: %u", intensity);
    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}

static void CMD_DevSetPosition(EmbeddedCli *cli, char *args, void *context)
{
    char buffer[100];
    const char *arg1 = embeddedCliGetToken(args, 1);
    if (!arg1) {
        snprintf(buffer, sizeof(buffer), "Usage: set_position <position>");
        embeddedCliPrint(cli, buffer);
        return;
    }
    uint8_t position = (uint8_t)strtoul(arg1, NULL, 0);
    MIN_Send_SET_POSITION_CMD(position);
    snprintf(buffer, sizeof(buffer), "Sent SET_POSITION_CMD with position: %u", position);
    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}

static void CMD_DevStartSamplingCycle(EmbeddedCli *cli, char *args, void *context)
{
    (void)args;
    (void)context;
    MIN_Send_START_SAMPLE_CYCLE_CMD();
    embeddedCliPrint(cli, "Sent START_SAMPLING_CYCLE_CMD");
    embeddedCliPrint(cli, "");
}

static void CMD_DevGetInfoSample(EmbeddedCli *cli, char *args, void *context)
{
    (void)args;
    (void)context;
    MIN_Send_GET_INFO_SAMPLE_CMD();
    embeddedCliPrint(cli, "Sent GET_INFO_SAMPLE_CMD");
    embeddedCliPrint(cli, "");
}

static void CMD_DevGetChunk(EmbeddedCli *cli, char *args, void *context)
{
    char buffer[100];
    const char *arg1 = embeddedCliGetToken(args, 1);
    if (!arg1) {
        snprintf(buffer, sizeof(buffer), "Usage: get_chunk <num_chunk>");
        embeddedCliPrint(cli, buffer);
        return;
    }
    uint8_t noChunk = (uint8_t)strtoul(arg1, NULL, 0);
    MIN_Send_GET_CHUNK_CMD(noChunk);
    snprintf(buffer, sizeof(buffer), "Sent GET_CHUNK_CMD for chunk: %u", noChunk);
    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}

static void CMD_DevGetCRCChunk(EmbeddedCli *cli, char *args, void *context)
{
    char buffer[100];
    const char *arg1 = embeddedCliGetToken(args, 1);
    if (!arg1) {
        snprintf(buffer, sizeof(buffer), "Usage: get_chunk_crc <num_chunk>");
        embeddedCliPrint(cli, buffer);
        return;
    }
    uint8_t noChunk = (uint8_t)strtoul(arg1, NULL, 0);
    MIN_Send_GET_CHUNK_CRC_CMD(noChunk);
    snprintf(buffer, sizeof(buffer), "Sent GET_CHUNK_CMD for chunk: %u", noChunk);
    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}

static void CMD_DevSetExtLaserProfile(EmbeddedCli *cli, char *args, void *context)
{
    char buffer[100];
    const char *arg1 = embeddedCliGetToken(args, 1);
    if (!arg1) {
        snprintf(buffer, sizeof(buffer), "Usage: set_ext_laser_profile <intensity>");
        embeddedCliPrint(cli, buffer);
        return;
    }
    uint8_t intensity = (uint8_t)strtoul(arg1, NULL, 0);
    MIN_Send_SET_EXT_LASER_INTENSITY_CMD(intensity);
    snprintf(buffer, sizeof(buffer), "Sent SET_EXT_LASER_PROFILE_CMD with intensity: %u", intensity);
    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}

static void CMD_DevTurnOnExtLaser(EmbeddedCli *cli, char *args, void *context)
{
    char buffer[100];
    const char *arg1 = embeddedCliGetToken(args, 1);
    if (!arg1) {
        snprintf(buffer, sizeof(buffer), "Usage: turn_on_ext_laser <position>");
        embeddedCliPrint(cli, buffer);
        return;
    }
    uint8_t position = (uint8_t)strtoul(arg1, NULL, 0);
    MIN_Send_TURN_ON_EXT_LASER_CMD(position);
    snprintf(buffer, sizeof(buffer), "Sent TURN_ON_EXT_LASER_CMD for position: %u", position);
    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}

static void CMD_DevTurnOffExtLaser(EmbeddedCli *cli, char *args, void *context)
{
    (void)args;
    (void)context;
    MIN_Send_TURN_OFF_EXT_LASER_CMD();
    embeddedCliPrint(cli, "Sent TURN_OFF_EXT_LASER_CMD");
    embeddedCliPrint(cli, "");
}

static void CMD_DevSetLaserInt(EmbeddedCli *cli, char *args, void *context)
{
    char buffer[100];
    const char *arg1 = embeddedCliGetToken(args, 1); // pos
    const char *arg2 = embeddedCliGetToken(args, 2); // percent

    if (!arg1 || !arg2) {
        snprintf(buffer, sizeof(buffer), "Usage: set_laser_int <pos> <percent>");
        embeddedCliPrint(cli, buffer);
        return;
    }

    uint8_t pos = (uint8_t)strtoul(arg1, NULL, 0);
    uint8_t percent = (uint8_t)strtoul(arg2, NULL, 0);

    MIN_Send_SET_LASER_INT_CMD(pos, percent);

    snprintf(buffer, sizeof(buffer), "Sent SET_LASER_INT_CMD with Position: %u, Percent: %u", pos, percent);
    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}

static void CMD_DevSetLaserExt(EmbeddedCli *cli, char *args, void *context)
{
    char buffer[100];
    const char *arg1 = embeddedCliGetToken(args, 1);
    const char *arg2 = embeddedCliGetToken(args, 2);

    if (!arg1 || !arg2) {
        snprintf(buffer, sizeof(buffer), "Usage: set_laser_ext <pos> <percent>");
        embeddedCliPrint(cli, buffer);
        return;
    }

    uint8_t pos = (uint8_t)strtoul(arg1, NULL, 0);
    uint8_t percent = (uint8_t)strtoul(arg2, NULL, 0);

    MIN_Send_SET_LASER_EXT_CMD(pos, percent);

    snprintf(buffer, sizeof(buffer), "Sent SET_LASER_EXT_CMD with Position: %u, Percent: %u", pos, percent);
    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}

static void CMD_DevGetCurrentLaser(EmbeddedCli *cli, char *args, void *context)
{
    char buffer[100];

    MIN_Send_GET_CURRENT_CMD();

    snprintf(buffer, sizeof(buffer), "Sent GET_CURRENT_CMD");
    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}

static void CMD_DevCustomCommand(EmbeddedCli *cli, char *args, void *context)
{
    char buffer[100];
    if (args == NULL || *args == '\0') {
        snprintf(buffer, sizeof(buffer), "Usage: custom_cmd <string>");
        embeddedCliPrint(cli, buffer);
        return;
    }
    MIN_Send_CUSTOM_COMMAND_CMD(args, strlen(args));
    snprintf(buffer, sizeof(buffer), "Sent CUSTOM_COMMAND with string: \"%s\"", args);
    embeddedCliPrint(cli, buffer);
    embeddedCliPrint(cli, "");
}


static void CMD_DevScriptManager(EmbeddedCli *cli, char *args, void *context)
{
	UserActivityTrigger();
    ScriptManager_PrintStatus();

    embeddedCliPrint(cli, "");
}

static void CMD_DevEraseScript(EmbeddedCli *cli, char *args, void *context)
{
	ScriptManager_EraseAllScriptsFromFRAM();

    embeddedCliPrint(cli, "");
}

static void CMD_DevLogManagerDebug(EmbeddedCli *cli, char *args, void *context)
{

	for (LogSource_TypeDef source = 0; source < LOG_SOURCE_COUNT; source++) {
		LogManager_DebugInfo(source);
		LogManager_DumpBuffer(source, LOG_BUFFER_LEFT);
		LogManager_DumpBuffer(source, LOG_BUFFER_RIGHT);
	}

    embeddedCliPrint(cli, "");
}


static void CMD_DevLogManagerLog(EmbeddedCli *cli, char *args, void *context)
{
    s_DateTime now;
    Utils_GetRTC(&now);

    LWL_Log(OBC_STM32_LOGTEST, now.day, now.month, now.year, now.hour, now.minute, now.second);

    char log_msg[64];
    snprintf(log_msg, sizeof(log_msg),
             "Time logged: %02d/%02d/20%02d %02d:%02d:%02d\r\n",
             now.day, now.month, now.year,
             now.hour, now.minute, now.second);

    embeddedCliPrint(cli, log_msg);
    embeddedCliPrint(cli, "");
}

static void CMD_DevCM4KeepAliveStatus(EmbeddedCli *cli, char *args, void *context)
{
    char status[64];
    snprintf(status, sizeof(status), "CM4 Miss Count: %u / %d\r\n",
             CM4_GetMissCount(), MAX_RETRY_COUNT);
    embeddedCliPrint(cli, status);
    embeddedCliPrint(cli, "");
}



static uint8_t current_step = 0;
static int mode = -1;

static void CMD_Testcase(EmbeddedCli *cli, char *args, void *context)
{
    if (args != NULL && strlen(args) > 0) {
        const char *arg1 = embeddedCliGetToken(args, 1);

        if (arg1 == NULL) {
            embeddedCliPrint(cli, "Usage: testcase <mode|reset>");
            return;
        }

        if (strcmp(arg1, "reset") == 0) {
            mode = -1;
            current_step = 0;
            embeddedCliPrint(cli, "Testcase state reset.");
            return;
        }
        mode = atoi(arg1);
        if(current_step == 0){
            char msg[64];
            snprintf(msg, sizeof(msg), "Start Testcase Mode %d", mode);
            embeddedCliPrint(cli, msg);
        }
    }

    if (mode == 0) {
        switch (current_step) {
            case 0:
                embeddedCliPrint(cli, "-> Step 1: set_sampling_profile 100000 100 4000 100");
                CMD_DevSetSamplingProfile(cli, " 100000 100 4000 100", context);
                break;
            case 1:
                embeddedCliPrint(cli, "-> Step 2: set_position 1");
                CMD_DevSetPosition(cli, " 1", context);
                break;
            case 2:
                embeddedCliPrint(cli, "-> Step 3: set_laser_intensity 50");
                CMD_DevSetLaserIntensity(cli, " 50", context);
                break;
            case 3:
                embeddedCliPrint(cli, "-> Step 4: start_sampling_cycle");
                CMD_DevStartSamplingCycle(cli, "", context);
                break;
            case 4:
                embeddedCliPrint(cli, "-> Step 5: get_chunk 0");
                CMD_DevGetChunk(cli, " 0", context);
                break;
            default:
                embeddedCliPrint(cli, "[v] Testcase 0 completed.");
                mode = -1;
                current_step = 0;
                return;
        }
        current_step++;
    } else if (mode == 2){
		uint8_t trigger_data[4];
		trigger_data[0] = (uint8_t)((0x11 >> 8) & 0xFF);
		trigger_data[1] = (uint8_t)(0x22 & 0xFF);
		trigger_data[2] = (uint8_t)((0x33 >> 8) & 0xFF);
		trigger_data[3] = (uint8_t)(0x44 & 0xFF);
		if (MODFSP_Send(&cm4_protocol, 0x21,
						 trigger_data, sizeof(trigger_data)) != MODFSP_OK) {
			embeddedCliPrint(cli, "Send to CM4 Fail!");
		}
		embeddedCliPrint(cli, "Send to CM4 OK!");
    }

    else{
        embeddedCliPrint(cli, "Unsupported mode. Only '0' is implemented.");
    }
}


static void CMD_FormatSD(EmbeddedCli *cli, char *args, void *context) {
    Std_ReturnType ret = PerformCleanup();
    if (ret == E_OK) {
        embeddedCliPrint(cli, "Filesystem formatted successfully.");
    } else if (ret == E_BUSY) {
        embeddedCliPrint(cli, "Filesystem busy. Try again later.");
    } else {
        embeddedCliPrint(cli, "Filesystem format failed.");
    }
    embeddedCliPrint(cli, "");
}



/*************************************************
 *                  End CMD List                 *
 *************************************************/

/*************************************************
 *                Getter - Helper                *
 *************************************************/
const CliCommandBinding *getCliStaticBindings(void) {
    return cliStaticBindings_internal;
}

uint16_t getCliStaticBindingCount(void) {
    return sizeof(cliStaticBindings_internal) / sizeof(cliStaticBindings_internal[0]);
}


