#include "simple_shield.h"
#include "string.h"
#include "stdlib.h"

#include "SPI_FRAM/fram_spi.h"

#ifdef DEBUG_SHIELD
#include "stdio.h"
#endif

static const char LINE_BREAK = '\n';
static const char CARRIAGE_RETURN = '\r';
static const char BACKSPACE = 0x7F;
static const int MAX_LOGIN_ATTEMPTS = 3;
static const uint32_t USER_TIMEOUT = 300000; // 5 minutes = 300000 ms
//static const uint32_t USER_TIMEOUT = 5000;


static char user_password[MAX_PASSWORD_LEN + 1] = "";
static int FRAM_PASSWORD_FAIL = 0;

#define FRAM_USER_PWD_LEN_ADDR  0x0000
#define FRAM_USER_PWD_ADDR      0x0001

static uint32_t fnv1a32(const char *s) {
    uint32_t h = 0x811C9DC5u;
    while (*s) {
        h ^= (uint8_t)(*s++);
        h *= 0x01000193u;
    }
    return h;
}
static int hash_equal(uint32_t a, uint32_t b) {
    uint32_t diff = a ^ b;
    diff |= diff >> 16;
    diff |= diff >> 8;
    return (diff & 0xFFu) == 0;
}

int verify_password(const char *input_password) {
    uint32_t input_hash = fnv1a32(input_password);
    uint32_t stored_hash = itwasmeulrich;
    return hash_equal(input_hash, stored_hash);
}

static void load_user_password(ShieldInstance_t *instance) {
    FRAM_SPI_HandleTypeDef *hfram = FRAM_SPI_GetHandle();
    uint8_t pwd_len = 0;

    if (FRAM_SPI_ReadMem(hfram, FRAM_USER_PWD_LEN_ADDR, &pwd_len, 1) != E_OK || pwd_len > MAX_PASSWORD_LEN) {
        Shield_WriteString(instance, "Could not load user-pwd, please use admin-pwd instead!\r\n");
    	FRAM_PASSWORD_FAIL = 1;
        return;
    }

    if (pwd_len > 0) {
        if (FRAM_SPI_ReadMem(hfram, FRAM_USER_PWD_ADDR, (uint8_t *)user_password, pwd_len) == E_OK) {
            user_password[pwd_len] = '\0';
        } else {
            Shield_WriteString(instance, "Could not load user-pwd, please use admin-pwd instead!\r\n");
        	FRAM_PASSWORD_FAIL = 1;
        }
    } else {
        Shield_WriteString(instance, "Could not load user-pwd, please use admin-pwd instead!\r\n");
    	FRAM_PASSWORD_FAIL = 1;
    }
}

static int verify_user_password(const char *input_password){
	if(FRAM_PASSWORD_FAIL == 1){
	    uint32_t input_hash = fnv1a32(input_password);
	    uint32_t stored_hash = itwasmeulrich;
	    return hash_equal(input_hash, stored_hash);
	}else{
		if(strcmp(input_password, user_password) == 0){
			return 1;
		}else{
			return 0;
		}
	}
	return 0;
}

void Shield_UpdateTimer(ShieldInstance_t *instance){
	if(instance->state == AUTH_USER){
		instance->last_activity_time++;
		if(instance->last_activity_time >= USER_TIMEOUT){
			Shield_Reset(instance);
		    Shield_WriteString(instance, "\r\nSession timed out due to inactivity.\r\n");
		}
	}
}
void Shield_ResetTimer(ShieldInstance_t *instance){
	if(instance->state == AUTH_USER){
		instance->last_activity_time = 0;
	}
}

void Shield_Init(ShieldInstance_t *instance, void (*write_char_func)(char c)) {
    instance->write_char = write_char_func;
    instance->initreset = 0;
    instance->pos = 0;
    instance->state = UNAUTHORIZED;
    instance->login_attempts = 0;
    instance->password_pos = 0;
    instance->last_activity_time = 0; // User should set this with actual time if needed
    memset(instance->buffer, 0, SHIELD_BUFFER_SIZE);
    memset(instance->password_buffer, 0, SHIELD_BUFFER_SIZE);
    Shield_WriteString(instance, "\33[2J");         // Clear screen
    Shield_WriteString(instance, "\033[0;0H");     // Move to 0:0
    Shield_WriteString(instance, "OBC: [CM4 -> OBC]");
    Shield_WriteString(instance, "\033[0;1H");
    Shield_WriteString(instance, SHIELD_NEWLINE);
//    Shield_WriteString(instance, SHIELD_INITATION);
}

void Shield_Reset(ShieldInstance_t *instance) {
    instance->initreset = 1;
    instance->pos = 0;
    instance->state = UNAUTHORIZED;
    instance->login_attempts = 0;
    instance->password_pos = 0;
    instance->last_activity_time = 0;
    memset(instance->buffer, 0, SHIELD_BUFFER_SIZE);
    memset(instance->password_buffer, 0, SHIELD_BUFFER_SIZE);
    Shield_WriteString(instance, "\33[2J");         // Clear screen
    Shield_WriteString(instance, "\033[0;0H");     // Move to 0:0
    Shield_WriteString(instance, "OBC: [CM4 -> OBC]");
    Shield_WriteString(instance, "\033[0;1H");
    Shield_WriteString(instance, SHIELD_NEWLINE);
    Shield_WriteString(instance, SHIELD_INITATION);
}

void Shield_WriteString(ShieldInstance_t *instance, const char *str) {
    while (*str) {
        instance->write_char(*str++);
    }
}

static void Shield_Process(ShieldInstance_t *instance) {
    if (instance->state == UNAUTHORIZED) {
        if (strcmp(instance->buffer, "admin") == 0) {
            instance->state = LOGIN_ADMIN;
            instance->login_attempts = 0;
            Shield_WriteString(instance, "Password: ");
        } else if (strcmp(instance->buffer, "user") == 0) {
        	load_user_password(instance);
            instance->state = LOGIN_USER;
            instance->login_attempts = 0;
            Shield_WriteString(instance, "Password: ");
        } else {
            Shield_WriteString(instance, "Unsupported this user name. [admin/user]\r\n");
            Shield_WriteString(instance, SHIELD_NEWLINE);
            Shield_WriteString(instance, "Login as [admin/user]: ");
        }
    } else if (instance->state == AUTH_ADMIN || instance->state == AUTH_USER) {
        Shield_WriteString(instance, "HelloWorld\r\n");
    }
}

void Shield_ReceiveChar(ShieldInstance_t *instance, char ch) {
#ifdef DEBUG_SHIELD
    char hex[6];
    snprintf(hex, sizeof(hex), "0x%02X ", ch);
    Shield_WriteString(instance, hex);
#endif
    if (instance->state == LOGIN_ADMIN || instance->state == LOGIN_USER) {
        if (ch == LINE_BREAK || ch == CARRIAGE_RETURN) {
            instance->password_buffer[instance->password_pos] = '\0';
            if (instance->state == LOGIN_ADMIN && verify_password(instance->password_buffer) == 1) {
                instance->state = AUTH_ADMIN;
                Shield_WriteString(instance, SHIELD_NEWLINE);
                Shield_WriteString(instance, "‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾\r\n");
                Shield_WriteString(instance, "Logged in as admin: SLT-OBC v1.2.0");
                Shield_WriteString(instance, SHIELD_NEWLINE);
                Shield_WriteString(instance, "__________________________________\r\n");
                Shield_WriteString(instance, "   ______ _______ ____  ___  _____\r\n");
                Shield_WriteString(instance, "  / __/ //_  __(_) __ \\/ _ )/ ___/\r\n");
                Shield_WriteString(instance, " _\\ \\/ /__/ / _ / /_/ / _  / /__  \r\n");
                Shield_WriteString(instance, "/___/____/_/ (_)\\____/____/\\___/  \r\n");
                Shield_WriteString(instance, "__________________________________\r\n");
                Shield_WriteString(instance, SHIELD_NEWLINE);
            } else if (instance->state == LOGIN_USER && verify_user_password(instance->password_buffer) == 1) {
                instance->state = AUTH_USER;
                Shield_WriteString(instance, SHIELD_NEWLINE);
                Shield_WriteString(instance, "‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾\r\n");
                Shield_WriteString(instance, "Logged in as user: SLT-OBC v1.2.0");
                Shield_WriteString(instance, SHIELD_NEWLINE);
                Shield_WriteString(instance, "__________________________________\r\n");
                Shield_WriteString(instance, "   ______ _______ ____  ___  _____\r\n");
                Shield_WriteString(instance, "  / __/ //_  __(_) __ \\/ _ )/ ___/\r\n");
                Shield_WriteString(instance, " _\\ \\/ /__/ / _ / /_/ / _  / /__  \r\n");
                Shield_WriteString(instance, "/___/____/_/ (_)\\____/____/\\___/  \r\n");
                Shield_WriteString(instance, "__________________________________\r\n");
                Shield_WriteString(instance, SHIELD_NEWLINE);
            } else {
                instance->login_attempts++;
                Shield_WriteString(instance, SHIELD_NEWLINE);
                if (instance->login_attempts >= MAX_LOGIN_ATTEMPTS) {
                    instance->state = UNAUTHORIZED;
                    Shield_WriteString(instance, "Too many failed attempts. Please try again.");
                    Shield_WriteString(instance, SHIELD_NEWLINE);
                    Shield_WriteString(instance, SHIELD_NEWLINE);
                    Shield_WriteString(instance, SHIELD_INITATION);
                } else {
                    Shield_WriteString(instance, "Incorrect password. Please try again.");
                    Shield_WriteString(instance, SHIELD_NEWLINE);
                    Shield_WriteString(instance, "Password: ");
                }
            }
            instance->password_pos = 0;
        } else if (ch == BACKSPACE) {
            if (instance->password_pos > 0) {
                instance->password_pos--;
                instance->write_char('\b');
                instance->write_char(' ');
                instance->write_char('\b');
            }
        } else if (instance->password_pos < SHIELD_BUFFER_SIZE - 1) {
            instance->password_buffer[instance->password_pos++] = ch;
            instance->write_char('*'); // Mask password with *
        }
    } else if (instance->state == AUTH_ADMIN || instance->state == AUTH_USER) {
        Shield_WriteString(instance, "Hello123\r\n"); // Placeholder for command processing
    } else {
        if (ch == LINE_BREAK || ch == CARRIAGE_RETURN) {
            instance->buffer[instance->pos] = '\0';
            if (instance->pos > 0) {
                Shield_WriteString(instance, SHIELD_NEWLINE);
                Shield_Process(instance);
                instance->pos = 0;
            } else {
                Shield_WriteString(instance, SHIELD_NEWLINE);
                Shield_WriteString(instance, SHIELD_INITATION);
            }
        } else if (ch == BACKSPACE) {
            if (instance->pos > 0) {
                instance->pos--;
                instance->buffer[instance->pos] = '\0';
                instance->write_char('\b');
                instance->write_char(' ');
                instance->write_char('\b');
            }
        } else if (instance->pos < SHIELD_BUFFER_SIZE - 1) {
            instance->buffer[instance->pos++] = ch;
            instance->write_char(ch);
        }
    }
}

ShieldAuthState_t Shield_GetState(ShieldInstance_t *instance) {
    return instance->state;
}

