/************************************************
 *  @file     : simple_datatrans.h
 *  @date     : Jul 23, 2025
 *  @author   : CAO HIEU
 *-----------------------------------------------
 *  Description :
 *    [-]
 ************************************************/

#ifndef M2_SYSTEM_SIMPLEDATATRANSFER_SIMPLE_DATATRANS_H_
#define M2_SYSTEM_SIMPLEDATATRANSFER_SIMPLE_DATATRANS_H_

#include "utils.h"
#include <stdbool.h>

/*************************************************
 *                 DEFINITIONS                   *
 *************************************************/
#define DATA_CHUNK_SIZE         (32 * 1024)    // 32KB per chunk
#define RAM_D3_BASE_ADDR        0x38000000      // RAM D3 base address
#define MAX_CRC_RETRY_COUNT     2               // Maximum CRC retry attempts
#define MAX_MASTER_RETRIES  	3
#define TRANSFER_TIMEOUT_MS     3000            // 3 second timeout

#define MODFSP_MASTER_ACK   			0x31
#define MODFSP_MASTER_NAK        		0x32

typedef enum {
    DATA_TYPE_CHUNK = 0,    // Chunk data (.dat files)
    DATA_TYPE_CURRENT = 1,  // Current data (.cdat files)
    DATA_TYPE_LOG = 2       // Log data (.log files)
} SimpleDataType_t;


typedef enum {
    SIMPLE_TRANSFER_SUCCESS = 0,
    SIMPLE_TRANSFER_ERROR_DATAREADY_TIMEOUT,
    SIMPLE_TRANSFER_ERROR_MINBUSY_TIMEOUT,
    SIMPLE_TRANSFER_ERROR_SPI_READ_FAILED,
    SIMPLE_TRANSFER_ERROR_CRC_MISMATCH,
    SIMPLE_TRANSFER_ERROR_FILE_SAVE_FAILED,
    SIMPLE_TRANSFER_ERROR_MASTER_TRIGGER_FAILED,
    SIMPLE_TRANSFER_ERROR_MASTER_ACK_TIMEOUT,
    SIMPLE_TRANSFER_ERROR_INVALID_PARAMS
} SimpleTransferResult_t;

/**
 * @brief Initialize Simple Data Transfer system
 * @return E_OK on success, E_ERROR on failure
 */
Std_ReturnType SimpleDataTransfer_Init(void);

/**
 * @brief Transfer single chunk of data (BLOCKING CALL)
 * @param data_type Type of data (CHUNK/CURRENT/LOG)
 * @param chunk_id Chunk ID for naming
 * @param base_filename Base filename without extension
 * @return Transfer result
 */
SimpleTransferResult_t SimpleDataTransfer_ExecuteTransfer(SimpleDataType_t data_type,
                                                         uint16_t chunk_id,
                                                         const char* base_filename,
                                                         uint8_t year,
                                                         uint8_t month,
                                                         uint8_t day,
                                                         uint8_t hour,
                                                         uint8_t minute,
                                                         uint8_t second
														 );

/**
 * @brief Transfer multiple chunks (BLOCKING CALL)
 * @param data_type Type of data
 * @param total_chunks Number of chunks to transfer
 * @param base_filename Base filename
 * @return Transfer result (fails on first chunk failure)
 */
//SimpleTransferResult_t SimpleDataTransfer_ExecuteMultipleChunks(SimpleDataType_t data_type,
//                                                               uint16_t total_chunks,
//                                                               const char* base_filename);

/**
 * @brief Get string representation of transfer result
 * @param result Transfer result
 * @return String description
 */
const char* SimpleDataTransfer_GetResultString(SimpleTransferResult_t result);


/**
 * @brief Get string representation of data type
 * @param type Data type
 * @return String description
 */
const char* SimpleDataTransfer_GetTypeString(SimpleDataType_t type);


/**
 * @brief Get transfer statistics
 * @param successful_transfers Pointer to store successful count
 * @param failed_transfers Pointer to store failed count
 * @param crc_errors Pointer to store CRC error count
 */
void SimpleDataTransfer_GetStatistics(uint32_t* successful_transfers,
                                     uint32_t* failed_transfers,
                                     uint32_t* crc_errors);


/**
 * @brief Reset transfer statistics
 */
void SimpleDataTransfer_ResetStatistics(void);

/**
 * @brief Print current statistics
 */
void SimpleDataTransfer_PrintStatistics(void);

/*************************************************
 *              UTILITY FUNCTIONS                *
 *************************************************/
void SimpleDataTransfer_HandleMasterAck(uint8_t frame_id, const uint8_t* data, uint32_t length);

/**
 * @brief Check if DATAREADY pin is active
 * @return true if busy, false if ready
 */
bool SimpleDataTransfer_IsDataReady(void);

/**
 * @brief Wait for DATAREADY to become ready
 * @param timeout_ms Timeout in milliseconds
 * @return E_OK if ready, E_TIMEOUT if timeout
 */
Std_ReturnType SimpleDataTransfer_WaitDataReady(uint32_t timeout_ms);

/**
 * @brief Calculate CRC16 XMODEM
 * @param data Pointer to data
 * @param size Data size
 * @return CRC16 value
 */
uint16_t SimpleDataTransfer_CalculateCRC16(const uint8_t* data, uint32_t size);

/**
 * @brief Generate filename based on type and chunk
 * @param data_type Data type
 * @param base_filename Base filename
 * @param chunk_id Chunk ID
 * @param output_filename Output buffer (must be at least 64 chars)
 */
void SimpleDataTransfer_GenerateFilename(SimpleDataType_t data_type,
                                        const char* base_filename,
                                        uint16_t chunk_id,
                                        char* output_filename);

SimpleTransferResult_t SimpleDataTransfer_ExecuteLogTransfer(const char* base_filename, uint8_t* log_data_ptr, uint32_t log_data_size,                                                          uint8_t year,
        uint8_t month,
        uint8_t day,
        uint8_t hour,
        uint8_t minute,
        uint8_t second);

bool SimpleDataTransfer_IsFatfsOk(void);
void SimpleDataTransfer_SetFatfsOk(bool status);


#endif /* M2_SYSTEM_SIMPLEDATATRANSFER_SIMPLE_DATATRANS_H_ */
