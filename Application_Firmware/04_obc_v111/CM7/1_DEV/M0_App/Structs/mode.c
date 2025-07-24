/*
 * mode.c
 *
 *  Created on: Mar 3, 2025
 *      Author: CAO HIEU
 */

#include "mode.h"
#include "CLI_Terminal/CLI_Src/embedded_cli.h"
#include <stdio.h>

static ForwardMode_t currentMode = FORWARD_MODE_NORMAL;
static uint8_t backslashCount = 0;

void ForwardMode_Set(ForwardMode_t mode) {
    currentMode = mode;
    backslashCount = 0;
}

ForwardMode_t ForwardMode_Get(void) {
    return currentMode;
}

_Bool ForwardMode_ProcessReceivedByte(uint8_t byte) {
    if ((char)byte == '\\') {
        backslashCount++;
    } else {
        backslashCount = 0;
    }
    if (backslashCount >= 10) {
        currentMode = FORWARD_MODE_NORMAL;
        backslashCount = 0;
        return true;
    }
    return false;
}
