/*
 * common_state.h
 *
 *  Created on: Mar 25, 2025
 *      Author: CAO HIEU
 */

#ifndef M0_APP_STRUCTS_COMMON_STATE_H_
#define M0_APP_STRUCTS_COMMON_STATE_H_

#include "utils.h"

typedef enum {
    INIT_STATE_NOT_STARTED = 0,
	INIT_STATE_STEP_PREPARING,
    INIT_STATE_STEP_ZERO,
    INIT_STATE_STEP_ONE,
    INIT_STATE_STEP_TWO,
    INIT_STATE_FINAL,
    INIT_STATE_COMPLETED,
    INIT_STATE_FAILED
} InitState_t;

typedef enum {
    PROGRAM_STATE_BOOTING = 0,
    PROGRAM_STATE_RUNNING,
    PROGRAM_STATE_ERROR,
    PROGRAM_STATE_HALTED
} ProgramState_t;

typedef struct {
    InitState_t init_state;
    ProgramState_t program_state;
    Std_ReturnType last_error;
} SystemStatus_t;

static SystemStatus_t system_status = {
    .init_state = INIT_STATE_NOT_STARTED,
    .program_state = PROGRAM_STATE_BOOTING,
    .last_error = E_OK
};

#endif /* M0_APP_STRUCTS_COMMON_STATE_H_ */
