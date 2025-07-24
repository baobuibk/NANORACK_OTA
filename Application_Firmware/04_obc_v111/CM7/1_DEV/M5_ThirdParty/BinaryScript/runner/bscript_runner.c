////Executor
//#include <stdint.h>
//#include <stdbool.h>
//#include "bscript_runner.h"
//#include "action_id.h"
//#include "../logger/bscript_logger.h"
//
//typedef struct {
//    Script* script;
//    uint16_t current_step;
//    ScriptState state;
//    _Bool request_stop;
//    _Bool request_pause;
//    uint32_t delay_until;
//} ScriptRunnerState;
//
//static ScriptRunnerState runner;
//
//void BScript_Stop(void) {
//    runner.request_stop = true;
//}
//
//void BScript_Pause(void) {
//    runner.request_pause = true;
//}
//
//void BScript_Resume(void) {
//    runner.request_pause = false;
//    if (runner.state == SCRIPT_STATE_PAUSED)
//        runner.state = SCRIPT_STATE_RUNNING;
//}
//
//ScriptState BScript_GetStateRunning(void) {
//    return runner.state;
//}
//
//StepExecResult execute_step(Step* step) {
//    BScript_Log("[Runner] Executing step %u: action_id = %u", step->step_id, step->action_id);
//
//    switch (step->action_id) {
//
//        case 0x01: { // eg: set_laser_intensity
//            uint8_t laser;
//            uint8_t mode;
//            if (BScript_ParseParamByIndex(step->parameters, step->param_len, 0, &laser) != PARSE_IDX_OK) return STEP_EXEC_ERROR;
//            if (BScript_ParseParamByIndex(step->parameters, step->param_len, 1, &mode) != PARSE_IDX_OK) return STEP_EXEC_ERROR;
//
//            BScript_Log("[Runner] Set laser intensity to %u %u", laser, mode);
//            break;
//        }
//
//        case SCRIPT_DELAY: { // delay (ms)
//            uint32_t delay_ms;
//            if (BScript_ParseParamByIndex(step->parameters, step->param_len, 0, &delay_ms) != PARSE_IDX_OK)
//                return STEP_EXEC_ERROR;
//
//            BScript_Log("[Runner] Delay %u ms", delay_ms);
//            runner.delay_until = BScript_GetTick() + delay_ms;
//            return STEP_EXEC_WAIT;
//        }
//
//        case SCRIPT_JMP: { // JMP
//            uint16_t target_step;
//            if (BScript_ParseParamByIndex(step->parameters, step->param_len, 0, &target_step) != PARSE_IDX_OK)
//                return STEP_EXEC_ERROR;
//
//            if (target_step >= runner.script->total_steps) {
//                BScript_Log("[Runner] Invalid jump target: %u", target_step);
//                return STEP_EXEC_ERROR;
//            }
//
//            BScript_Log("[Runner] Jumping to step %u", target_step);
//            runner.current_step = target_step;
//            return STEP_EXEC_SUCCESS;
//        }
//
//        default:
//            BScript_Log("[Runner] Unknown action: %u", step->action_id);
//            return STEP_EXEC_ERROR;
//    }
//
//    return STEP_EXEC_SUCCESS;
//}
//
//
//void BScript_CallToPlay(void) {
//    switch (runner.state) {
//        case SCRIPT_STATE_IDLE:
//            break;
//
//        case SCRIPT_STATE_RUNNING:
//            if (runner.request_stop) {
//                runner.state = SCRIPT_STATE_STOPPED;
//                break;
//            }
//
//            if (runner.request_pause) {
//                runner.state = SCRIPT_STATE_PAUSED;
//                break;
//            }
//
//            if (runner.current_step >= runner.script->total_steps) {
//                runner.state = SCRIPT_STATE_FINISHED;
//                break;
//            }
//
//            Step* step = &runner.script->steps[runner.current_step];
//            StepExecResult result = execute_step(step);
//
//            if (result == STEP_EXEC_ERROR) {
//                runner.state = SCRIPT_STATE_ERROR;
//            } else if (result == STEP_EXEC_WAIT) {
//                runner.state = SCRIPT_STATE_PAUSED; // or WAITING
//            } else if (result == STEP_EXEC_SUCCESS && step->action_id != 0x07) {
//                runner.current_step++;
//            }
//
//            break;
//
//        case SCRIPT_STATE_PAUSED:
//            if (BScript_GetTick() >= runner.delay_until && runner.delay_until != 0) {
//                runner.delay_until = 0;
//                runner.state = SCRIPT_STATE_RUNNING;
//            }
//            break;
//
//        case SCRIPT_STATE_STOPPED:
//        case SCRIPT_STATE_FINISHED:
//        case SCRIPT_STATE_ERROR:
//            break;
//    }
//}
