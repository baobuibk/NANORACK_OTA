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
#include "bsp_system.h"
#include "AliveCM4/alive_cm4.h"

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
static void CMD_Boot_Reset(EmbeddedCli *cli, char *args, void *context);

/*************************************************
 * Command Define "Dev"              *
 *************************************************/
static void CMD_DevTestConnection(EmbeddedCli *cli, char *args, void *context);
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
static void CMD_DevSetExtLaserProfile(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevTurnOnExtLaser(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevTurnOffExtLaser(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevCustomCommand(EmbeddedCli *cli, char *args, void *context);

static void CMD_DevScriptManager(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevEraseScript(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevLogManagerDebug(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevLogManagerLog(EmbeddedCli *cli, char *args, void *context);
static void CMD_DevCM4KeepAliveStatus(EmbeddedCli *cli, char *args, void *context);
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

	{ "Dev", "test_connection", 			"Send TEST_CONNECTION_CMD with a 32-bit value", 		true, 	NULL, CMD_DevTestConnection },
	{ "Dev", "set_temp_profile", 			"set_temp_profile <ntc_index> <tec_pos> <heater_pos> <tec_vol> <heater_duty> <target_temp>",
																									true, 	NULL, CMD_DevSetTempProfile },
	{ "Dev", "start_temp_profile", 			"Send START_TEMP_PROFILE_CMD", 							false, 	NULL, CMD_DevStartTempProfile },
	{ "Dev", "stop_temp_profile", 			"Send STOP_TEMP_PROFILE_CMD", 							false, 	NULL, CMD_DevStopTempProfile },
	{ "Dev", "set_override_tec_profile", 	"set_override_tec_profile <tec_index> <tec_vol>", 		true, 	NULL, CMD_DevSetOverrideTecProfile },
	{ "Dev", "start_override_tec_profile", 	"Send START_OVERRIDE_TEC_PROFILE_CMD", 					false, 	NULL, CMD_DevStartOverrideTecProfile },
	{ "Dev", "stop_override_tec_profile", 	"Send STOP_OVERRIDE_TEC_PROFILE_CMD", 					false, 	NULL, CMD_DevStopOverrideTecProfile },
	{ "Dev", "set_sampling_profile", 		"set_sampling_profile <pre> <in> <post> <sample_rate>", true, 	NULL, CMD_DevSetSamplingProfile },
	{ "Dev", "set_laser_intensity", 		"set_laser_intensity <intensity>", 						true, 	NULL, CMD_DevSetLaserIntensity },
	{ "Dev", "set_position", 				"set_position <position>", 								true, 	NULL, CMD_DevSetPosition },
	{ "Dev", "start_sampling_cycle", 		"Send START_SAMPLING_CYCLE_CMD", 						false, 	NULL, CMD_DevStartSamplingCycle },
	{ "Dev", "get_info_sample", 			"Send GET_INFO_SAMPLE_CMD", 							false, 	NULL, CMD_DevGetInfoSample },
	{ "Dev", "get_chunk", 					"get_chunk <num_chunk>", 								true,	NULL, CMD_DevGetChunk },
	{ "Dev", "set_ext_laser_profile", 		"set_ext_laser_profile <intensity>", 					true, 	NULL, CMD_DevSetExtLaserProfile },
	{ "Dev", "turn_on_ext_laser", 			"turn_on_ext_laser <position>", 						true, 	NULL, CMD_DevTurnOnExtLaser },
	{ "Dev", "turn_off_ext_laser", 			"Send TURN_OFF_EXT_LASER_CMD", 							false, 	NULL, CMD_DevTurnOffExtLaser },
	{ "Dev", "custom_cmd", 					"send_custom_cmd <string>", 							true, 	NULL, CMD_DevCustomCommand },

	{ "Dev", "script_manager", 				"-", 													false, 	NULL, CMD_DevScriptManager},
	{ "Dev", "erase_script", 				"-", 													false, 	NULL, CMD_DevEraseScript},
	{ "Dev", "lwl_debug", 					"-", 													false, 	NULL, CMD_DevLogManagerDebug},
	{ "Dev", "lwl_log", 					"-", 													false, 	NULL, CMD_DevLogManagerLog},
	{ "Dev", "alivecm4_log", 				"-", 													false, 	NULL, CMD_DevCM4KeepAliveStatus},
	{ "Dev", "obc_rst", 				"-", 													false, 	NULL, CMD_Boot_Reset},

};
/*************************************************
 *                 External Declarations         *
 *************************************************/

/*************************************************
 *             Command List Function             *
 *************************************************/
extern uint32_t _scustom_data;
extern uint32_t _ecustom_data;
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

    if (size < 1 || size > RAM_D2_200KB_SIZE) {
        snprintf(buffer, sizeof(buffer), "Invalid size. Must be 1 to %lu bytes.", (unsigned long)RAM_D2_200KB_SIZE);
        embeddedCliPrint(cli, buffer);
        return;
    }

    snprintf(buffer, sizeof(buffer), "Dumping %lu bytes of RAM_D3 contents:", (unsigned long)size);
    embeddedCliPrint(cli, buffer);

    for (uint32_t i = 0; i < size; i += bytes_per_line) {
        snprintf(buffer, sizeof(buffer), "0x%08lX: ", (uint32_t)(0x38000000 + i));
        char *ptr = buffer + strlen(buffer);

        for (uint32_t j = 0; j < bytes_per_line && (i + j) < size; j++) {
            uint8_t value = RAM_D3_START[i + j];
            snprintf(ptr, sizeof(buffer) - (ptr - buffer), "%02X ", value);
            ptr += 3;
            byte_count++;
            crc = UpdateCRC16_XMODEM(crc, value);
        }

        *ptr++ = ' ';
        *ptr++ = '|';
        for (uint32_t j = 0; j < bytes_per_line && (i + j) < size; j++) {
            uint8_t c = RAM_D3_START[i + j];
            *ptr++ = (c >= 32 && c <= 126) ? c : '.';
        }
        *ptr = '\0';

        embeddedCliPrint(cli, buffer);

        vTaskDelay(pdMS_TO_TICKS(1));
    }

    snprintf(buffer, sizeof(buffer), "Dump complete. Counted bytes: %lu, CRC16-XMODEM: 0x%04X",
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

static void CMD_MasterRead(EmbeddedCli *cli, char *args, void *context) {
    const char *arg1 = embeddedCliGetToken(args, 1); // size
    char buffer[100];

    if (arg1 == NULL) {
        embeddedCliPrint(cli, "Usage: master_read <size> (size: 1-200KB)");
        return;
    }

    uint32_t size = (uint32_t)strtoul(arg1, NULL, 0);

    if (size < 1 || size > RAM_D2_200KB_SIZE) {
        snprintf(buffer, sizeof(buffer), "Invalid size. Must be 1 to %lu bytes.", (unsigned long)RAM_D2_200KB_SIZE);
        embeddedCliPrint(cli, buffer);
        return;
    }

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

    Std_ReturnType ret = SPI_MasterDevice_ReadDMA(0x38000000, size);
    if (ret == E_OK) {
        uint16_t crc = 0x0000;
        for (uint32_t i = 0; i < size; i++) {
            crc = UpdateCRC16_XMODEM(crc, RAM_D3_START[i]);
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

    if (FS_Request_Write(filename, (uint8_t*)content, content_len) == E_OK) {
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

static void CMD_DevSetTempProfile(EmbeddedCli *cli, char *args, void *context)
{
    char buffer[256];
    const char *arg1 = embeddedCliGetToken(args, 1);
    const char *arg2 = embeddedCliGetToken(args, 2);
    const char *arg3 = embeddedCliGetToken(args, 3);
    const char *arg4 = embeddedCliGetToken(args, 4);
    const char *arg5 = embeddedCliGetToken(args, 5);
    const char *arg6 = embeddedCliGetToken(args, 6);

    if (!arg1 || !arg2 || !arg3 || !arg4 || !arg5 || !arg6) {
        snprintf(buffer, sizeof(buffer), "Usage: set_temp_profile <ntc_index> <tec_pos> <heater_pos> <tec_vol> <heater_duty> <target_temp>");
        embeddedCliPrint(cli, buffer);
        return;
    }

    uint8_t ntc_index         = (uint8_t)strtoul(arg1, NULL, 0);
    uint8_t tec_positions     = (uint8_t)strtoul(arg2, NULL, 0);
    uint8_t heater_positions  = (uint8_t)strtoul(arg3, NULL, 0);
    uint16_t tec_vol          = (uint16_t)strtoul(arg4, NULL, 0);
    uint8_t heater_duty_cycle = (uint8_t)strtoul(arg5, NULL, 0);
    uint16_t target_temp      = (uint16_t)strtoul(arg6, NULL, 0);

    MIN_Send_SET_TEMP_PROFILE_CMD(ntc_index, tec_positions, heater_positions, tec_vol, heater_duty_cycle, target_temp);
    snprintf(buffer, sizeof(buffer), "Sent SET_TEMP_PROFILE_CMD with params: %u, %u, %u, %u, %u, %u",
             ntc_index, tec_positions, heater_positions, tec_vol, heater_duty_cycle, target_temp);
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

    if (!arg1 || !arg2) {
        snprintf(buffer, sizeof(buffer), "Usage: set_override_tec_profile <tec_index> <tec_vol>");
        embeddedCliPrint(cli, buffer);
        return;
    }

    uint8_t ovr_tec_index = (uint8_t)strtoul(arg1, NULL, 0);
    uint16_t ovr_tec_vol  = (uint16_t)strtoul(arg2, NULL, 0);

    MIN_Send_SET_OVERRIDE_TEC_PROFILE_CMD(ovr_tec_index, ovr_tec_vol);
    snprintf(buffer, sizeof(buffer), "Sent SET_OVERRIDE_TEC_PROFILE_CMD with params: %u, %u",
             ovr_tec_index, ovr_tec_vol);
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
        snprintf(buffer, sizeof(buffer), "Usage: set_sampling_profile <pre> <in> <post> <sample_rate>");
        embeddedCliPrint(cli, buffer);
        return;
    }

    uint32_t pre         = (uint32_t)strtoul(arg1, NULL, 0);
    uint32_t in          = (uint32_t)strtoul(arg2, NULL, 0);
    uint32_t post        = (uint32_t)strtoul(arg3, NULL, 0);
    uint16_t sample_rate = (uint16_t)strtoul(arg4, NULL, 0);

    MIN_Send_SET_PDA_PROFILE_CMD(pre, in, post, sample_rate);
    snprintf(buffer, sizeof(buffer), "Sent SET_SAMPLING_PROFILE_CMD with params: pre=%lu, in=%lu, post=%lu, sample_rate=%u",
             (unsigned long)pre, (unsigned long)in, (unsigned long)post, sample_rate);
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

static void CMD_Boot_Reset(EmbeddedCli *cli, char *args, void *context)
{
	embeddedCliPrint(cli, "bootloader");
	vTaskDelay(1000);
	System_On_Bootloader_Reset();
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


