/*
 * cli_setup.c
 *
 *  Created on: Feb 27, 2025
 *      Author: CAO HIEU
 */

#include "cli_setup.h"
#include "uart_driver_dma.h"
#include "board.h"
#include "CLI_Terminal/CLI_Command/cli_command.h"

// CLI buffer
/*************************************************
 *           CLI Static Buffer Define            *
 *************************************************/

#define USBCDC_CLI_BUFFER_SIZE 2048
static  CLI_UINT usbcdc_cliStaticBuffer[BYTES_TO_CLI_UINTS(USBCDC_CLI_BUFFER_SIZE)];
//#define UARTCM4_CLI_BUFFER_SIZE 2048
//static  CLI_UINT uartcm4_cliStaticBuffer[BYTES_TO_CLI_UINTS(UARTCM4_CLI_BUFFER_SIZE)];

/*************************************************
 *             ----------------------            *
 *************************************************/

static EmbeddedCli *cli_usbcdc;
//static EmbeddedCli *cli_uartcm4;

// Bool to disable the interrupts, if CLI is not yet ready.
static _Bool cliIsReady = false;

/*************************************************
 *          Tx Transmit CLI Byte Buffer          *
 *************************************************/

static void writeCharToCli_USBCDC(EmbeddedCli *embeddedCli, char c) {
    uint8_t c_to_send = c;
    UART_Driver_Write(UART_USB, c_to_send);
}

//static void writeCharToCli_UARTCM4(EmbeddedCli *embeddedCli, char c) {
//    uint8_t c_to_send = c;
//    UART_Driver_Write(UART_DEBUG, c_to_send);
//}

//Call before FREERTOS be initialized
//Call After UART Driver Init (or Peripheral use CLI)

Std_ReturnType SystemCLI_Init() {
    // Initialize the CLI configuration settings
    // Initialize USB CDC CLI
    EmbeddedCliConfig *usbcdc_config = embeddedCliDefaultConfig();
    usbcdc_config->cliBuffer = usbcdc_cliStaticBuffer;
    usbcdc_config->cliBufferSize = USBCDC_CLI_BUFFER_SIZE;
    usbcdc_config->rxBufferSize = CLI_RX_BUFFER_SIZE;
    usbcdc_config->cmdBufferSize = CLI_CMD_BUFFER_SIZE;
    usbcdc_config->historyBufferSize = CLI_HISTORY_SIZE;
    usbcdc_config->maxBindingCount = CLI_MAX_BINDING_COUNT;
    usbcdc_config->enableAutoComplete = CLI_AUTO_COMPLETE;
    usbcdc_config->invitation = CLI_INITATION_USB;
    usbcdc_config->staticBindings = getCliStaticBindings();
    usbcdc_config->staticBindingCount = getCliStaticBindingCount();

    cli_usbcdc = embeddedCliNew(usbcdc_config);
    if (cli_usbcdc == NULL) {
        return E_ERROR;
    }
    cli_usbcdc->writeChar = writeCharToCli_USBCDC;

    // Initialize UART CM4 CLI
//    EmbeddedCliConfig *uartcm4_config = embeddedCliDefaultConfig();
//    uartcm4_config->cliBuffer = uartcm4_cliStaticBuffer;
//    uartcm4_config->cliBufferSize = UARTCM4_CLI_BUFFER_SIZE;
//    uartcm4_config->rxBufferSize = CLI_RX_BUFFER_SIZE;
//    uartcm4_config->cmdBufferSize = CLI_CMD_BUFFER_SIZE;
//    uartcm4_config->historyBufferSize = CLI_HISTORY_SIZE;
//    uartcm4_config->maxBindingCount = CLI_MAX_BINDING_COUNT;
//    uartcm4_config->enableAutoComplete = CLI_AUTO_COMPLETE;
//    uartcm4_config->invitation = CLI_INITATION_CM4;
//    uartcm4_config->staticBindings = getCliStaticBindings();
//    uartcm4_config->staticBindingCount = getCliStaticBindingCount();
//
//    cli_uartcm4 = embeddedCliNew(uartcm4_config);
//    if (cli_uartcm4 == NULL) {
//        return E_ERROR;
//    }
//    cli_uartcm4->writeChar = writeCharToCli_UARTCM4;

    // Init the CLI with blank screen
//    onClearCLI(cli, NULL, NULL);

    // CLI has now been initialized, set bool to true to enable interrupts.
    cliIsReady = true;

    return E_OK;
}


/*************************************************
 *             Get CLI Pointers                  *
 *************************************************/
EmbeddedCli *getUsbCdcCliPointer() {
    return cli_usbcdc;
}

//EmbeddedCli *getUartCm4CliPointer() {
//    return cli_uartcm4;
//}

