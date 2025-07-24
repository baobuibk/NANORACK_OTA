#ifndef BSCRIPT_PARSER_H
#define BSCRIPT_PARSER_H

#include "stdint.h"

#define MAGIC_HEADER            0xC0DEDEAD
#define MAGIC_STEP              0xDEADBEEF
#define MAX_STEPS               200
#define MAX_PARAM_SIZE          71

#define PARAM_TYPE_UINT8   0x01
#define PARAM_TYPE_UINT16  0x02
#define PARAM_TYPE_UINT32  0x03
#define PARAM_TYPE_FLOAT   0x04
#define PARAM_TYPE_STRING  0x05

typedef enum {
    PARSE_IDX_OK = 0,
    PARSE_IDX_INVALID_INDEX = -1,
    PARSE_IDX_INSUFFICIENT_DATA = -2,
    PARSE_IDX_LENGTH_MISMATCH = -3,
    PARSE_IDX_UNKNOWN_TYPE = -4
} ParseByIndexResult;

typedef enum {
    PARSE_OK = 0,
    PARSE_ERROR = -1,
    PARSE_ERROR_MAGIC = -2,
    PARSE_ERROR_SIZE = -3,
    PARSE_ERROR_STEP_MAGIC = -4,
    PARSE_ERROR_STEP_LENGTH = -5,
    PARSE_ERROR_STEP_OVERFLOW = -6
} ScriptParseResult;

typedef struct {
    uint32_t smagic;        // 0xDEADBEEF
    uint16_t step_id;
    uint8_t  action_id;
    uint8_t  param_len;
    uint8_t  parameters[MAX_PARAM_SIZE - 7]; 
} Step;

typedef struct {
    uint16_t version;                             // From header
    uint16_t total_steps;
    Step     steps[MAX_STEPS];
} Script;

ParseByIndexResult BScript_ParseParamByIndex(const uint8_t* buffer, uint8_t total_len, uint8_t index, void* out_value);
ScriptParseResult BScript_ParseScript(const uint8_t* buffer, uint32_t buffer_size, Script* script);

#endif // BSCRIPT_PARSER_H
