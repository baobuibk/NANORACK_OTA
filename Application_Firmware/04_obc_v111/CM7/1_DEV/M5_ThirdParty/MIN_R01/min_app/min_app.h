#ifndef MIN_APP_H
#define MIN_APP_H

#include "../min/min.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define TEST_DEMO_MIN

#define AUTO_REINIT_ON_TIMEOUT

#define MIN_FRAME_TIMEOUT_MS 3000U

typedef void (*MIN_ResponseHandler)(uint8_t min_id, const uint8_t *payload, uint8_t len);
void MIN_RegisterResponseHandler(MIN_ResponseHandler handler);

/**
 * @brief Maximum number of MIN contexts that can be registered.
 *
 * This defines the number of independent MIN communication contexts that can be managed.
 * Each context represents a separate communication channel, typically associated with
 * a specific hardware port (e.g., different UART peripherals or logical connections).
 *
 * - The value 4 is chosen as a reasonable limit to support multiple connections
 *   while keeping memory usage optimized for embedded systems.
 * - Contexts are stored in an array, where each index corresponds to a specific port.
 * - If more than 4 contexts are needed, this value should be increased accordingly.
 */
#define MAX_MIN_CONTEXTS 4

#define EXP_PORT 0

/**
 * @brief Application-specific MIN context structure.
 *        Allows handling multiple independent MIN contexts.
 */
typedef struct MIN_Context_t {
    struct min_context min_ctx;  						 ///< Core MIN context from the MIN library.
    uint32_t last_poll_time;      						 ///< Timestamp of the last poll operation (in ms).
    void (*timeout_callback)(struct MIN_Context_t *ctx); ///< Callback function for timeout events.
    _Bool timeout_triggered;       						 ///< Flag indicating whether the timeout callback has been triggered.
#ifdef AUTO_REINIT_ON_TIMEOUT
    _Bool auto_reinit;             						 ///< Option to automatically reinitialize after a timeout.
#endif
} MIN_Context_t;

/**
 * @brief Initializes a MIN context and associates it with a specific port.
 *        This wraps `min_init_context` for easier usage.
 *
 * @param ctx Pointer to the MIN context to initialize.
 * @param port Communication port to associate with the context.
 */
void MIN_Context_Init(MIN_Context_t *ctx, uint8_t port);

/**
 * @brief Reinitializes a MIN context, resetting its state, sequence,
 *        and clearing previous timeout flags.
 *
 * @param ctx Pointer to the MIN context to reinitialize.
 */
void MIN_ReInit(MIN_Context_t *ctx);

/**
 * @brief Registers a timeout callback for a given MIN context.
 *
 * @param ctx Pointer to the MIN context.
 * @param callback Function to be called on timeout.
 */
void MIN_RegisterTimeoutCallback(MIN_Context_t *ctx, void (*callback)(MIN_Context_t *ctx));

/**
 * @brief Processes incoming data and checks for timeouts.
 *        This function should be called periodically.
 *
 * @param ctx Pointer to the MIN context.
 * @param rx_data Received data buffer (can be NULL if no new data).
 * @param rx_len Length of received data.
 *
 * @note This function should be called from a UART Rx callback or a main loop.
 */
void MIN_App_Poll(MIN_Context_t *ctx, const uint8_t *rx_data, uint32_t rx_len);

/**
 * @brief Sends a frame using the transport queue.
 *        This wraps `min_queue_frame` for simplified use.
 *
 * @param ctx Pointer to the MIN context.
 * @param min_id Message ID to send.
 * @param payload Pointer to the data payload.
 * @param len Length of the payload.
 *
 * @note Ensure there is enough space in the queue before sending.
 */
void MIN_Send(MIN_Context_t *ctx, uint8_t min_id, const uint8_t *payload, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif // MIN_APP_H
