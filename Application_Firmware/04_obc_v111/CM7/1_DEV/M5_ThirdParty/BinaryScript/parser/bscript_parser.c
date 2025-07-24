#include "bscript_parser.h"
#include "string.h"
#include "../logger/bscript_logger.h"

// param buffer:
// [
//   0x03,             // Num of field TLV (3)
//   0x01, 0x01, 0x11,             // field 0: uint8_t
//   0x02, 0x02, 0x34, 0x12,       // field 1: uint16_t
//   0x03, 0x04, 0x78, 0x56, 0x34, 0x12  // field 2: uint32_t
// ]

typedef struct {
    uint32_t hmagic;        // 0xC0DEDEAD
    uint16_t version;      
    uint16_t total_steps;
    uint16_t header_crc;    // CRC16_XMODEM of first 8 bytes
} ScriptFileHeader;

uint16_t crc16_xmodem(const uint8_t *data, uint32_t length) {
    uint16_t crc = 0x0000;
    for (uint32_t i = 0; i < length; ++i) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; ++j) {
            crc = (crc & 0x8000) ? ((crc << 1) ^ 0x1021) : (crc << 1);
        }
    }
    return crc;
}

ParseByIndexResult BScript_ParseParamByIndex(const uint8_t* buffer, uint8_t total_len, uint8_t index, void* out_value) {
    if (total_len < 1) return PARSE_IDX_INSUFFICIENT_DATA;

    uint8_t num_fields = buffer[0];
    if (index >= num_fields) return PARSE_IDX_INVALID_INDEX;

    uint8_t offset = 1;
    for (uint8_t i = 0; i < num_fields; ++i) {
        if (offset + 2 > total_len) return PARSE_IDX_INSUFFICIENT_DATA;

        uint8_t type = buffer[offset];
        uint8_t length = buffer[offset + 1];
        offset += 2;

        if (offset + length > total_len) return PARSE_IDX_INSUFFICIENT_DATA;

        if (i == index) {
            switch (type) {
                case PARAM_TYPE_UINT8:
                    if (length != 1) return PARSE_IDX_LENGTH_MISMATCH;
                    *(uint8_t*)out_value = buffer[offset];
                    return PARSE_IDX_OK;

                case PARAM_TYPE_UINT16:
                    if (length != 2) return PARSE_IDX_LENGTH_MISMATCH;
                    *(uint16_t*)out_value = (buffer[offset + 1] << 8) | buffer[offset];
                    return PARSE_IDX_OK;

                case PARAM_TYPE_UINT32:
                    if (length != 4) return PARSE_IDX_LENGTH_MISMATCH;
                    *(uint32_t*)out_value = ((uint32_t)buffer[offset + 3] << 24) |
                                            ((uint32_t)buffer[offset + 2] << 16) |
                                            ((uint32_t)buffer[offset + 1] << 8) |
                                            buffer[offset];
                    return PARSE_IDX_OK;

                case PARAM_TYPE_FLOAT:
                    if (length != 4) return PARSE_IDX_LENGTH_MISMATCH;
                    memcpy(out_value, &buffer[offset], 4);
                    return PARSE_IDX_OK;

                case PARAM_TYPE_STRING:
                    memcpy(out_value, &buffer[offset], length);
                    ((char*)out_value)[length] = '\0';
                    return PARSE_IDX_OK;

                default:
                    return PARSE_IDX_UNKNOWN_TYPE;
            }
        }

        offset += length;
    }

    return PARSE_IDX_INVALID_INDEX;
}

ScriptParseResult BScript_ParseScript(const uint8_t* buffer, uint32_t buffer_size, Script* script) {
    if (buffer_size < sizeof(ScriptFileHeader) + sizeof(uint16_t)) {
        BScript_Log("[Parser] Error: File too small for header + CRC");
        return PARSE_ERROR_SIZE;
    }

    const ScriptFileHeader* header = (const ScriptFileHeader*)buffer;

    if (header->hmagic != MAGIC_HEADER) {
        BScript_Log("[Parser] Error: Invalid header magic: 0x%08X", header->hmagic);
        return PARSE_ERROR_MAGIC;
    }

    if (header->total_steps > MAX_STEPS) {
        BScript_Log("[Parser] Error: Too many steps: %u\n", header->total_steps);
        return PARSE_ERROR;
    }

    // === CRC Check ===
    // 1. Check header_crc (on first 8 bytes)
    uint16_t computed_header_crc = crc16_xmodem(buffer, 8);
    if (computed_header_crc != header->header_crc) {
        BScript_Log("[Parser] Header CRC mismatch: expected 0x%04X, got 0x%04X",
               header->header_crc, computed_header_crc);
        return PARSE_ERROR;
    }

    // 2. Check total CRC (last 2 bytes of file)
    uint16_t file_total_crc = *(uint16_t*)(buffer + buffer_size - 2);
    uint16_t computed_total_crc = crc16_xmodem(buffer, buffer_size - 2);
    if (file_total_crc != computed_total_crc) {
        BScript_Log("[Parser] Total CRC mismatch: expected 0x%04X, got 0x%04X",
               file_total_crc, computed_total_crc);
        return PARSE_ERROR;
    }

    // === Parse steps ===
    script->total_steps = header->total_steps;

    uint32_t offset = sizeof(ScriptFileHeader);

    if(offset != 10){
        offset = 10;
        BScript_Log("[Parser] Warning: offset struct != 10, we would change to offset 10");
    }


    for (uint16_t i = 0; i < script->total_steps; i++) {
        if (offset + 8 > buffer_size - 2) {
            BScript_Log("[Parser] Error: Step %u header out of bounds", i + 1);
            return PARSE_ERROR_STEP_LENGTH;
        }

        Step* step = &script->steps[i];
        step->smagic = *(uint32_t*)(buffer + offset);
        step->step_id = *(uint16_t*)(buffer + offset + 4);
        step->action_id = *(uint8_t*)(buffer + offset + 6);
        step->param_len = *(uint8_t*)(buffer + offset + 7);

        offset += 8;

        if (step->smagic != MAGIC_STEP) {
            BScript_Log("[Parser] Error: Step %u invalid magic: 0x%08X", i + 1, step->smagic);
            return PARSE_ERROR_STEP_MAGIC;
        }

        if (step->param_len > sizeof(step->parameters)) {
            BScript_Log("[Parser] Error: Step %u parameter too long: %u", i + 1, step->param_len);
            return PARSE_ERROR_STEP_OVERFLOW;
        }

        if (offset + step->param_len > buffer_size - 2) {
            BScript_Log("[Parser] Error: Step %u parameter data out of bounds", i + 1);
            return PARSE_ERROR_STEP_OVERFLOW;
        }

        memcpy(step->parameters, buffer + offset, step->param_len);
        offset += step->param_len;

        BScript_Log("[Parser] Step %u: action_id=%u, param_len=%u", step->step_id, step->action_id, step->param_len);
    }

    return PARSE_OK;
}
