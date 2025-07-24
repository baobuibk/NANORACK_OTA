#include "min_app.h"
#include "min_command.h"
#include <string.h>

#ifdef TEST_DEMO_MIN
#include "main.h"
extern UART_HandleTypeDef huart6;
#endif

#ifdef MIN_DEBUG_PRINTING
#include "stdio.h"
#include "stdarg.h"
void min_debug_print(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    printf("\r");
    va_end(args);
}
#endif

#include "board.h"
#include "uart_driver_dma.h"

static MIN_Context_t *registered_contexts[MAX_MIN_CONTEXTS] = {0};

uint16_t min_tx_space(uint8_t port)
{
    return UART_DMA_Driver_TXNumFreeSlots(UART_EXP);
/*
 * Since the implementation uses UART TX in Blocking/IRQ mode, we assume that
 * the buffer always has enough space, so this function returns a constant value.
 */
}

void min_tx_byte(uint8_t port, uint8_t byte)
{
    if (port == EXP_PORT)
    {
    	UART_Driver_Write(UART_EXP, byte);
#ifdef TEST_DEMO_MIN
        HAL_UART_Transmit(&huart6, &byte, 1, 10);
#endif
    }
}

/**
 * @brief Callback triggered before starting a transmission.
 *
 * This function is called before sending data. It can be used to
 * perform actions such as disabling TX interrupts or preparing
 * the hardware for transmission.
 */
void min_tx_start(uint8_t port)
{
    (void)port;
    // Example: Disable TX interrupt if needed (not required in this case)
}

/**
 * @brief Callback triggered after transmission is complete.
 *
 * This function is called when data transmission is finished.
 * It can be used to restore the previous state if any changes
 * were made in `min_tx_start()`.
 */
void min_tx_finished(uint8_t port)
{
    (void)port;
    // Example: Restore TX interrupt state if modified earlier
}

/**
 * @brief Returns the current system time in milliseconds.
 *
 * This function retrieves the system tick count and provides
 * a timestamp for timeout handling and scheduling.
 *
 * @return uint32_t The current system time in milliseconds.
 */
uint32_t min_time_ms(void)
{
    return HAL_GetTick(); // Uses HAL function to get system uptime
}

void MIN_Context_Init(MIN_Context_t *ctx, uint8_t port) {
    min_init_context(&ctx->min_ctx, port);
    min_transport_reset(&ctx->min_ctx, true);
    ctx->last_poll_time = min_time_ms();
    ctx->timeout_triggered = false;
#ifdef AUTO_REINIT_ON_TIMEOUT
    ctx->auto_reinit = true;
#endif
    if (port < MAX_MIN_CONTEXTS) {
        registered_contexts[port] = ctx;
    }
}

void MIN_ReInit(MIN_Context_t *ctx) {
    min_init_context(&ctx->min_ctx, ctx->min_ctx.port);
    min_transport_reset(&ctx->min_ctx, true);
    ctx->last_poll_time = min_time_ms();
    ctx->timeout_triggered = false;
}

void MIN_RegisterTimeoutCallback(MIN_Context_t *ctx, void (*callback)(MIN_Context_t *ctx)) {
    ctx->timeout_callback = callback;
}

void MIN_App_Poll(MIN_Context_t *ctx, const uint8_t *rx_data, uint32_t rx_len) {
    min_poll(&ctx->min_ctx, rx_data, rx_len);
    uint32_t now = min_time_ms();
    if (ctx->min_ctx.transport_fifo.n_frames > 0) {
        struct transport_frame *oldest = &ctx->min_ctx.transport_fifo.frames[ctx->min_ctx.transport_fifo.head_idx];
        if ((now - oldest->last_sent_time_ms) > MIN_FRAME_TIMEOUT_MS) {
            if (!ctx->timeout_triggered) {
                if (ctx->timeout_callback) {
                    ctx->timeout_callback(ctx);
                }
                ctx->timeout_triggered = true;
#ifdef AUTO_REINIT_ON_TIMEOUT
                if (ctx->auto_reinit) {
                    MIN_ReInit(ctx);
                }
#endif
            }
        } else {
            ctx->timeout_triggered = false;
        }
    }
    ctx->last_poll_time = now;
}
void MIN_Send(MIN_Context_t *ctx, uint8_t min_id, const uint8_t *payload, uint8_t len) {
    if (min_queue_has_space_for_frame(&ctx->min_ctx, len)) {
        min_queue_frame(&ctx->min_ctx, min_id, payload, len);
    }
}


/* ==================== MIN Library Callback ==================== */

/**
 * @brief Callback function triggered when a valid MIN frame is received.
 *
 * This function is called by the MIN protocol stack when a complete
 * and valid frame is received. It determines the corresponding
 * communication context based on the port and processes the message
 * using the predefined command table.
 *
 * @param min_id The MIN message ID.
 * @param min_payload Pointer to the received payload data.
 * @param len_payload Length of the received payload.
 * @param port The communication port from which the frame was received.
 *
 * @note This function is responsible for dispatching commands
 *       to the appropriate handlers based on `min_id`.
 */

static MIN_ResponseHandler response_handler = NULL;

void MIN_RegisterResponseHandler(MIN_ResponseHandler handler) {
    response_handler = handler;
}

void min_application_handler(uint8_t min_id, const uint8_t *min_payload, uint8_t len_payload, uint8_t port) {
    if (port >= MAX_MIN_CONTEXTS) {
        return;
    }
    MIN_Context_t *ctx = registered_contexts[port];
    if (ctx == NULL) {
        return;
    }

    if (response_handler != NULL) {
            response_handler(min_id, min_payload, len_payload);
    }

    const MIN_Command_t *command_table = MIN_GetCommandTable();
    int table_size = MIN_GetCommandTableSize();
    for (int i = 0; i < table_size; i++) {
        if (command_table[i].id == min_id) {
            command_table[i].handler(ctx, min_payload, len_payload);
            return;
        }
    }
    // Optional: Add default handler for unmatched commands
}
