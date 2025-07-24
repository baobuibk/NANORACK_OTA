/*************************************************
 * @file     : script_storage.c
 * @date     : Jul 21, 2025
 * @author   : CAO HIEU
 *-----------------------------------------------
 * Description :
 *   Script Storage Manager for FRAM persistence
 *   Handles saving/loading scripts with CRC validation
 ************************************************/

#include "script_storage.h"

#include "logger/bscript_logger.h"
#include "port/bscript_port.h"
#include <string.h>

/*************************************************
 *               CONSTANTS                       *
 *************************************************/

// FRAM memory layout (using last 16KB of 256KB FRAM)
#define FRAM_TOTAL_SIZE         (256 * 1024)    // 256KB total
#define FRAM_SCRIPT_SECTION_SIZE (16 * 1024)    // 16KB for scripts
#define FRAM_SCRIPT_BASE_ADDR   (FRAM_TOTAL_SIZE - FRAM_SCRIPT_SECTION_SIZE)

// Each script slot is 5KB
#define SCRIPT_SLOT_SIZE        (5 * 1024)      // 5KB per script

// Script storage addresses
#define FRAM_INIT_SCRIPT_ADDR   (FRAM_SCRIPT_BASE_ADDR + 0 * SCRIPT_SLOT_SIZE)
#define FRAM_DLS_SCRIPT_ADDR    (FRAM_SCRIPT_BASE_ADDR + 1 * SCRIPT_SLOT_SIZE)
#define FRAM_CAM_SCRIPT_ADDR    (FRAM_SCRIPT_BASE_ADDR + 2 * SCRIPT_SLOT_SIZE)

// Magic codes for each script type
#define MAGIC_CODE_INIT         0x494E  // "IN"
#define MAGIC_CODE_DLS          0x444C  // "DL" 
#define MAGIC_CODE_CAM          0x4341  // "CA"

// Header structure
#define HEADER_MAGIC_OFFSET     0
#define HEADER_LENGTH_OFFSET    2
#define HEADER_DATA_OFFSET      6
#define HEADER_SIZE             6

// Constraints
#define MIN_SCRIPT_SIZE         10
#define MAX_SCRIPT_DATA_SIZE    (SCRIPT_SLOT_SIZE - HEADER_SIZE - 2) // 2 bytes for CRC
#define CRC_SIZE                2

// Auto-load timing
#define AUTO_LOAD_DELAY_SEC     0

/*************************************************
 *               PRIVATE VARIABLES               *
 *************************************************/

static ScriptStorageManager_t g_storage_manager;

/*************************************************
 *               CRC16 FUNCTIONS                 *
 *************************************************/

/**
 * @brief Calculate CRC16 X-Modem
 * @param data Pointer to data
 * @param length Data length
 * @return CRC16 value
 */
static uint16_t ScriptStorage_CalculateCRC16(const uint8_t* data, uint32_t length)
{
    uint16_t crc = 0x0000;  // X-Modem initial value
    uint16_t polynomial = 0x1021;  // X-Modem polynomial

    for (uint32_t i = 0; i < length; i++) {
        crc ^= (uint16_t)(data[i] << 8);
        
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ polynomial;
            } else {
                crc <<= 1;
            }
        }
    }
    
    return crc;
}

/*************************************************
 *               HELPER FUNCTIONS                *
 *************************************************/

/**
 * @brief Get FRAM address for script type
 * @param type Script type
 * @return FRAM address or 0 if invalid
 */
static uint32_t ScriptStorage_GetFramAddress(ScriptType_t type)
{
    switch (type) {
        case SCRIPT_TYPE_INIT:
            return FRAM_INIT_SCRIPT_ADDR;
        case SCRIPT_TYPE_DLS_ROUTINE:
            return FRAM_DLS_SCRIPT_ADDR;
        case SCRIPT_TYPE_CAM_ROUTINE:
            return FRAM_CAM_SCRIPT_ADDR;
        default:
            return 0;
    }
}

/**
 * @brief Get magic code for script type
 * @param type Script type
 * @return Magic code or 0 if invalid
 */
static uint16_t ScriptStorage_GetMagicCode(ScriptType_t type)
{
    switch (type) {
        case SCRIPT_TYPE_INIT:
            return MAGIC_CODE_INIT;
        case SCRIPT_TYPE_DLS_ROUTINE:
            return MAGIC_CODE_DLS;
        case SCRIPT_TYPE_CAM_ROUTINE:
            return MAGIC_CODE_CAM;
        default:
            return 0;
    }
}

/**
 * @brief Get script type name for logging
 * @param type Script type
 * @return Script type name
 */
static const char* ScriptStorage_GetTypeName(ScriptType_t type)
{
    switch (type) {
        case SCRIPT_TYPE_INIT:
            return "INIT";
        case SCRIPT_TYPE_DLS_ROUTINE:
            return "DLS";
        case SCRIPT_TYPE_CAM_ROUTINE:
            return "CAM";
        default:
            return "UNKNOWN";
    }
}

/*************************************************
 *               PUBLIC FUNCTIONS                *
 *************************************************/

/**
 * @brief Initialize script storage manager
 * @return true if successful, false otherwise
 */
_Bool ScriptStorage_Init(void)
{
    memset(&g_storage_manager, 0, sizeof(ScriptStorageManager_t));
    
    // Get FRAM handle
    g_storage_manager.fram_handle = FRAM_SPI_GetHandle();
    if (!g_storage_manager.fram_handle) {
        BScript_Log("[ScriptStorage] Error: Failed to get FRAM handle");
        return false;
    }
    
    g_storage_manager.is_initialized = true;
    g_storage_manager.auto_load_enabled = true;
    g_storage_manager.auto_load_delay_sec = AUTO_LOAD_DELAY_SEC;
    
    BScript_Log("[ScriptStorage] Initialized successfully");
    BScript_Log("[ScriptStorage] FRAM script storage:");
    BScript_Log("  - Base address: 0x%06X", FRAM_SCRIPT_BASE_ADDR);
    BScript_Log("  - INIT slot: 0x%06X - 0x%06X (%d bytes)", 
               FRAM_INIT_SCRIPT_ADDR, FRAM_INIT_SCRIPT_ADDR + SCRIPT_SLOT_SIZE - 1, SCRIPT_SLOT_SIZE);
    BScript_Log("  - DLS slot:  0x%06X - 0x%06X (%d bytes)", 
               FRAM_DLS_SCRIPT_ADDR, FRAM_DLS_SCRIPT_ADDR + SCRIPT_SLOT_SIZE - 1, SCRIPT_SLOT_SIZE);
    BScript_Log("  - CAM slot:  0x%06X - 0x%06X (%d bytes)", 
               FRAM_CAM_SCRIPT_ADDR, FRAM_CAM_SCRIPT_ADDR + SCRIPT_SLOT_SIZE - 1, SCRIPT_SLOT_SIZE);
    
    return true;
}

/**
 * @brief Save script to FRAM
 * @param type Script type
 * @param binary_data Binary script data
 * @param size Data size
 * @return Storage result
 */
ScriptStorageResult_t ScriptStorage_SaveScript(ScriptType_t type, const uint8_t* binary_data, uint32_t size)
{
    if (!g_storage_manager.is_initialized) {
        BScript_Log("[ScriptStorage] Error: Storage not initialized");
        return STORAGE_ERROR_NOT_INITIALIZED;
    }
    
    if (type >= SCRIPT_TYPE_COUNT || !binary_data) {
        BScript_Log("[ScriptStorage] Error: Invalid parameters");
        return STORAGE_ERROR_INVALID_PARAMS;
    }
    
    if (size < MIN_SCRIPT_SIZE || size > MAX_SCRIPT_DATA_SIZE) {
        BScript_Log("[ScriptStorage] Error: Invalid script size %u (must be %d-%d bytes)", 
                   size, MIN_SCRIPT_SIZE, MAX_SCRIPT_DATA_SIZE);
        return STORAGE_ERROR_INVALID_SIZE;
    }
    
    uint32_t fram_addr = ScriptStorage_GetFramAddress(type);
    uint16_t magic_code = ScriptStorage_GetMagicCode(type);
    const char* type_name = ScriptStorage_GetTypeName(type);
    
    BScript_Log("[ScriptStorage] Saving %s script to FRAM (size: %u bytes)", type_name, size);
    
    // Prepare header: [Magic Code][Length][Data][CRC16]
    uint8_t header[HEADER_SIZE];
    header[0] = (magic_code >> 8) & 0xFF;  // Magic code high byte
    header[1] = magic_code & 0xFF;         // Magic code low byte
    header[2] = (size >> 24) & 0xFF;       // Length byte 3 (MSB)
    header[3] = (size >> 16) & 0xFF;       // Length byte 2
    header[4] = (size >> 8) & 0xFF;        // Length byte 1
    header[5] = size & 0xFF;               // Length byte 0 (LSB)
    
    // Calculate CRC for header + data
    uint16_t crc = ScriptStorage_CalculateCRC16(header, HEADER_SIZE);
    crc = ScriptStorage_CalculateCRC16(binary_data, size) ^ crc;  // XOR with data CRC
    
    uint8_t crc_bytes[CRC_SIZE];
    crc_bytes[0] = (crc >> 8) & 0xFF;  // CRC high byte
    crc_bytes[1] = crc & 0xFF;         // CRC low byte
    
    BScript_Log("[ScriptStorage] %s script details:", type_name);
    BScript_Log("  - FRAM address: 0x%06X", fram_addr);
    BScript_Log("  - Magic code: 0x%04X", magic_code);
    BScript_Log("  - Data size: %u bytes", size);
    BScript_Log("  - CRC16: 0x%04X", crc);
    
    // Write to FRAM: Header
    Std_ReturnType status = FRAM_SPI_WriteMem(g_storage_manager.fram_handle, fram_addr, header, HEADER_SIZE);
    if (status != E_OK) {
        BScript_Log("[ScriptStorage] Error: Failed to write header to FRAM (status: %d)", status);
        return STORAGE_ERROR_FRAM_WRITE;
    }
    
    // Write to FRAM: Data
    status = FRAM_SPI_WriteMem(g_storage_manager.fram_handle, fram_addr + HEADER_DATA_OFFSET, 
                               (uint8_t*)binary_data, size);
    if (status != E_OK) {
        BScript_Log("[ScriptStorage] Error: Failed to write data to FRAM (status: %d)", status);
        return STORAGE_ERROR_FRAM_WRITE;
    }
    
    // Write to FRAM: CRC
    status = FRAM_SPI_WriteMem(g_storage_manager.fram_handle, fram_addr + HEADER_DATA_OFFSET + size, 
                               crc_bytes, CRC_SIZE);
    if (status != E_OK) {
        BScript_Log("[ScriptStorage] Error: Failed to write CRC to FRAM (status: %d)", status);
        return STORAGE_ERROR_FRAM_WRITE;
    }
    
    BScript_Log("[ScriptStorage] %s script saved successfully to FRAM", type_name);
    return STORAGE_SUCCESS;
}

/**
 * @brief Load script from FRAM
 * @param type Script type
 * @param binary_data Output buffer for binary data
 * @param max_size Maximum buffer size
 * @param actual_size Actual loaded size
 * @return Storage result
 */
ScriptStorageResult_t ScriptStorage_LoadScript(ScriptType_t type, uint8_t* binary_data, 
                                             uint32_t max_size, uint32_t* actual_size)
{
    if (!g_storage_manager.is_initialized) {
        BScript_Log("[ScriptStorage] Error: Storage not initialized");
        return STORAGE_ERROR_NOT_INITIALIZED;
    }
    
    if (type >= SCRIPT_TYPE_COUNT || !binary_data || !actual_size) {
        BScript_Log("[ScriptStorage] Error: Invalid parameters");
        return STORAGE_ERROR_INVALID_PARAMS;
    }
    
    uint32_t fram_addr = ScriptStorage_GetFramAddress(type);
    uint16_t expected_magic = ScriptStorage_GetMagicCode(type);
    const char* type_name = ScriptStorage_GetTypeName(type);
    
    BScript_Log("[ScriptStorage] Loading %s script from FRAM (address: 0x%06X)", type_name, fram_addr);
    
    // Read header
    uint8_t header[HEADER_SIZE];
    Std_ReturnType status = FRAM_SPI_ReadMem(g_storage_manager.fram_handle, fram_addr, header, HEADER_SIZE);
    if (status != E_OK) {
        BScript_Log("[ScriptStorage] Error: Failed to read header from FRAM (status: %d)", status);
        return STORAGE_ERROR_FRAM_READ;
    }
    
    // Validate magic code
    uint16_t stored_magic = (header[0] << 8) | header[1];
    if (stored_magic != expected_magic) {
        BScript_Log("[ScriptStorage] %s script: Invalid magic code 0x%04X (expected 0x%04X)", 
                   type_name, stored_magic, expected_magic);
        return STORAGE_ERROR_INVALID_MAGIC;
    }
    
    // Extract data size
    uint32_t data_size = (header[2] << 24) | (header[3] << 16) | (header[4] << 8) | header[5];
    
    // Validate data size
    if (data_size < MIN_SCRIPT_SIZE || data_size > MAX_SCRIPT_DATA_SIZE) {
        BScript_Log("[ScriptStorage] %s script: Invalid data size %u (must be %d-%d)", 
                   type_name, data_size, MIN_SCRIPT_SIZE, MAX_SCRIPT_DATA_SIZE);
        return STORAGE_ERROR_INVALID_SIZE;
    }
    
    if (data_size > max_size) {
        BScript_Log("[ScriptStorage] %s script: Data size %u exceeds buffer size %u", 
                   type_name, data_size, max_size);
        return STORAGE_ERROR_BUFFER_TOO_SMALL;
    }
    
    BScript_Log("[ScriptStorage] %s script header valid:", type_name);
    BScript_Log("  - Magic code: 0x%04X", stored_magic);
    BScript_Log("  - Data size: %u bytes", data_size);
    
    // Read script data
    status = FRAM_SPI_ReadMem(g_storage_manager.fram_handle, fram_addr + HEADER_DATA_OFFSET, 
                              binary_data, data_size);
    if (status != E_OK) {
        BScript_Log("[ScriptStorage] Error: Failed to read script data from FRAM (status: %d)", status);
        return STORAGE_ERROR_FRAM_READ;
    }
    
    // Read stored CRC
    uint8_t crc_bytes[CRC_SIZE];
    status = FRAM_SPI_ReadMem(g_storage_manager.fram_handle, fram_addr + HEADER_DATA_OFFSET + data_size, 
                              crc_bytes, CRC_SIZE);
    if (status != E_OK) {
        BScript_Log("[ScriptStorage] Error: Failed to read CRC from FRAM (status: %d)", status);
        return STORAGE_ERROR_FRAM_READ;
    }
    
    uint16_t stored_crc = (crc_bytes[0] << 8) | crc_bytes[1];
    
    // Calculate and verify CRC
    uint16_t calculated_crc = ScriptStorage_CalculateCRC16(header, HEADER_SIZE);
    calculated_crc = ScriptStorage_CalculateCRC16(binary_data, data_size) ^ calculated_crc;
    
    if (stored_crc != calculated_crc) {
        BScript_Log("[ScriptStorage] %s script: CRC mismatch! Stored: 0x%04X, Calculated: 0x%04X", 
                   type_name, stored_crc, calculated_crc);
        return STORAGE_ERROR_CRC_MISMATCH;
    }
    
    *actual_size = data_size;
    
    BScript_Log("[ScriptStorage] %s script loaded successfully:", type_name);
    BScript_Log("  - Data size: %u bytes", data_size);
    BScript_Log("  - CRC16: 0x%04X (verified)", stored_crc);
    
    return STORAGE_SUCCESS;
}

/**
 * @brief Check if script exists in FRAM
 * @param type Script type
 * @return true if script exists and is valid, false otherwise
 */
_Bool ScriptStorage_ScriptExists(ScriptType_t type)
{
    if (!g_storage_manager.is_initialized || type >= SCRIPT_TYPE_COUNT) {
        return false;
    }
    
    uint32_t fram_addr = ScriptStorage_GetFramAddress(type);
    uint16_t expected_magic = ScriptStorage_GetMagicCode(type);
    
    // Read header
    uint8_t header[HEADER_SIZE];
    Std_ReturnType status = FRAM_SPI_ReadMem(g_storage_manager.fram_handle, fram_addr, header, HEADER_SIZE);
    if (status != E_OK) {
        return false;
    }
    
    // Check magic code
    uint16_t stored_magic = (header[0] << 8) | header[1];
    if (stored_magic != expected_magic) {
        return false;
    }
    
    // Check data size
    uint32_t data_size = (header[2] << 24) | (header[3] << 16) | (header[4] << 8) | header[5];
    if (data_size < MIN_SCRIPT_SIZE || data_size > MAX_SCRIPT_DATA_SIZE) {
        return false;
    }
    
    return true;
}

/**
 * @brief Erase script from FRAM
 * @param type Script type
 * @return Storage result
 */
ScriptStorageResult_t ScriptStorage_EraseScript(ScriptType_t type)
{
    if (!g_storage_manager.is_initialized) {
        return STORAGE_ERROR_NOT_INITIALIZED;
    }
    
    if (type >= SCRIPT_TYPE_COUNT) {
        return STORAGE_ERROR_INVALID_PARAMS;
    }
    
    uint32_t fram_addr = ScriptStorage_GetFramAddress(type);
    const char* type_name = ScriptStorage_GetTypeName(type);
    
    BScript_Log("[ScriptStorage] Erasing %s script from FRAM", type_name);
    
    // Write zeros to magic code to invalidate the script
    uint8_t zeros[HEADER_SIZE] = {0};
    Std_ReturnType status = FRAM_SPI_WriteMem(g_storage_manager.fram_handle, fram_addr, zeros, HEADER_SIZE);
    if (status != E_OK) {
        BScript_Log("[ScriptStorage] Error: Failed to erase script header (status: %d)", status);
        return STORAGE_ERROR_FRAM_WRITE;
    }
    
    BScript_Log("[ScriptStorage] %s script erased successfully", type_name);
    return STORAGE_SUCCESS;
}

/**
 * @brief Auto-load all scripts from FRAM with delay
 * @param script_manager Pointer to script manager
 * @return Number of scripts successfully loaded
 */
uint8_t ScriptStorage_AutoLoadScripts(void* script_manager)
{
    if (!g_storage_manager.is_initialized || !g_storage_manager.auto_load_enabled) {
        BScript_Log("[ScriptStorage] Auto-load disabled or not initialized");
        return 0;
    }
    
    BScript_Log("[ScriptStorage] Starting auto-load sequence...");
    BScript_Log("[ScriptStorage] Waiting %u seconds for user input...", g_storage_manager.auto_load_delay_sec);
    
    // Wait for specified delay (checking every second for user activity)
    for (uint32_t i = 0; i < g_storage_manager.auto_load_delay_sec; i++) {
        BScript_Delayms(1000);  // 1 second delay
        if (UserActivityDetected()) {
            BScript_Log("[ScriptStorage] User activity detected, canceling auto-load");
            return 0;
        }
    }


    BScript_Log("[ScriptStorage] Auto-load delay completed, loading scripts from FRAM...");
    
    uint8_t loaded_count = 0;
    uint8_t buffer[MAX_SCRIPT_DATA_SIZE];
    
    // Try to load each script type
    for (ScriptType_t type = SCRIPT_TYPE_INIT; type < SCRIPT_TYPE_COUNT; type++) {
        const char* type_name = ScriptStorage_GetTypeName(type);
        
        if (ScriptStorage_ScriptExists(type)) {
            uint32_t actual_size;
            ScriptStorageResult_t result = ScriptStorage_LoadScript(type, buffer, sizeof(buffer), &actual_size);
            
            if (result == STORAGE_SUCCESS) {
                // Load script into script manager
                if (ScriptManager_LoadScript(type, buffer, actual_size)) {
                    loaded_count++;
                    BScript_Log("[ScriptStorage] Auto-loaded %s script successfully", type_name);
                } else {
                    BScript_Log("[ScriptStorage] Failed to load %s script into manager", type_name);
                }
            } else {
                BScript_Log("[ScriptStorage] Failed to load %s script from FRAM (result: %d)", type_name, result);
            }
        } else {
            BScript_Log("[ScriptStorage] No valid %s script found in FRAM", type_name);
        }
    }
    
    if (loaded_count == SCRIPT_TYPE_COUNT) {
        BScript_Log("[ScriptStorage] All scripts loaded successfully from FRAM, starting execution...");
        // All scripts loaded, start execution
        ScriptManager_StartExecution();
    } else if (loaded_count > 0) {
        BScript_Log("[ScriptStorage] Partial scripts loaded (%u/%u), waiting for remaining scripts...", 
                   loaded_count, SCRIPT_TYPE_COUNT);
        // TODO: Request missing scripts from master
        // ScriptStorage_RequestMissingScripts();
    } else {
        BScript_Log("[ScriptStorage] No scripts loaded from FRAM, requesting all scripts from master...");
        // TODO: Request all scripts from master
        // ScriptStorage_RequestAllScripts();
    }
    
    return loaded_count;
}

/**
 * @brief Get storage status
 * @param status Pointer to status structure
 */
void ScriptStorage_GetStatus(ScriptStorageStatus_t* status)
{
    if (!status) return;
    
    memset(status, 0, sizeof(ScriptStorageStatus_t));
    status->is_initialized = g_storage_manager.is_initialized;
    status->auto_load_enabled = g_storage_manager.auto_load_enabled;
    status->auto_load_delay_sec = g_storage_manager.auto_load_delay_sec;
    
    // Check which scripts exist
    for (ScriptType_t type = SCRIPT_TYPE_INIT; type < SCRIPT_TYPE_COUNT; type++) {
        status->script_exists[type] = ScriptStorage_ScriptExists(type);
    }
    
    status->fram_base_address = FRAM_SCRIPT_BASE_ADDR;
    status->script_slot_size = SCRIPT_SLOT_SIZE;
    status->max_script_size = MAX_SCRIPT_DATA_SIZE;
}

/**
 * @brief Print storage status
 */
void ScriptStorage_PrintStatus(void)
{
    ScriptStorageStatus_t status;
    ScriptStorage_GetStatus(&status);
    
    BScript_Log("[ScriptStorage] === STORAGE STATUS ===");
    BScript_Log("[ScriptStorage] Initialized: %s", status.is_initialized ? "YES" : "NO");
    BScript_Log("[ScriptStorage] Auto-load enabled: %s", status.auto_load_enabled ? "YES" : "NO");
    BScript_Log("[ScriptStorage] Auto-load delay: %u seconds", status.auto_load_delay_sec);
    BScript_Log("[ScriptStorage] FRAM base address: 0x%06X", status.fram_base_address);
    BScript_Log("[ScriptStorage] Script slot size: %u bytes", status.script_slot_size);
    BScript_Log("[ScriptStorage] Max script size: %u bytes", status.max_script_size);
    
    BScript_Log("[ScriptStorage] Script availability:");
    for (ScriptType_t type = SCRIPT_TYPE_INIT; type < SCRIPT_TYPE_COUNT; type++) {
        const char* type_name = ScriptStorage_GetTypeName(type);
        uint32_t addr = ScriptStorage_GetFramAddress(type);
        BScript_Log("  - %s: %s (0x%06X)", type_name, 
                   status.script_exists[type] ? "EXISTS" : "NOT FOUND", addr);
    }
    
    BScript_Log("[ScriptStorage] === END STATUS ===");
}

/**
 * @brief Enable/disable auto-load feature
 * @param enabled true to enable, false to disable
 */
void ScriptStorage_SetAutoLoadEnabled(_Bool enabled)
{
    g_storage_manager.auto_load_enabled = enabled;
    BScript_Log("[ScriptStorage] Auto-load %s", enabled ? "ENABLED" : "DISABLED");
}

/**
 * @brief Set auto-load delay
 * @param delay_sec Delay in seconds
 */
void ScriptStorage_SetAutoLoadDelay(uint32_t delay_sec)
{
    g_storage_manager.auto_load_delay_sec = delay_sec;
    BScript_Log("[ScriptStorage] Auto-load delay set to %u seconds", delay_sec);
}
