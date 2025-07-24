#include "../parser/bscript_parser.h"

typedef enum {
    SCRIPT_STATE_IDLE,
    SCRIPT_STATE_RUNNING,
    SCRIPT_STATE_PAUSED,
    SCRIPT_STATE_STOPPED,
    SCRIPT_STATE_FINISHED,
    SCRIPT_STATE_ERROR
} ScriptState;

typedef enum {
    STEP_EXEC_SUCCESS,
    STEP_EXEC_WAIT,
    STEP_EXEC_ERROR
} StepExecResult;

void BScript_Start(Script* script);
void BScript_Stop(void);
void BScript_Pause(void);
void BScript_Resume(void);
void BScript_Play(void);
ScriptState BScript_GetStateRunning(void);
