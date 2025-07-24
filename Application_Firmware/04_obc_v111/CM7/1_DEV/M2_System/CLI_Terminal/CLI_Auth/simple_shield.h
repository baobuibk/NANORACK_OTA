#ifndef SIMPLE_SHIELD_H
#define SIMPLE_SHIELD_H

#include "stdint.h"

#define ADMIN_USERNAME          "admin"
#define USER_USERNAME           "user"
#define itwasmeulrich          	0xc5477ba3u	//ptn209b3@ https://md5calc.com/hash/fnv1a32/ptn209b3%40
#define USER_PASSWORD           "user123@"

#define MAX_PASSWORD_LEN        16
#define LOGIN_MAX_ATTEMPTS      3
#define AUTO_LOGOUT_TIMEOUT_MS  (5ul * 60ul * 1000ul)   /* 5 minutes */


#define SHIELD_NEWLINE "\r\n"
#define SHIELD_INITATION "Login as [admin/user]: "
#define SHIELD_BUFFER_SIZE 32

typedef enum {
    UNAUTHORIZED = 0,
    LOGIN_ADMIN,
    LOGIN_USER,
    AUTH_ADMIN,
    AUTH_USER
} ShieldAuthState_t;

typedef struct {
    char buffer[SHIELD_BUFFER_SIZE];           // Input buffer for commands
    int initreset;                             // On init or reset
    int pos;                                   // Position in input buffer
    ShieldAuthState_t state;                   // Current authentication state
    int login_attempts;                        // Number of failed login attempts
    char password_buffer[SHIELD_BUFFER_SIZE];  // Buffer for password input
    int password_pos;                          // Position in password buffer
    uint32_t last_activity_time;               // Last activity time for timeout
    void (*write_char)(char c);                // Function pointer for writing characters
} ShieldInstance_t;

void Shield_Init(ShieldInstance_t *instance, void (*write_char_func)(char c));
void Shield_ReceiveChar(ShieldInstance_t *instance, char ch);
void Shield_WriteString(ShieldInstance_t *instance, const char *str);
void Shield_Reset(ShieldInstance_t *instance);
ShieldAuthState_t Shield_GetState(ShieldInstance_t *instance);
void Shield_UpdateTimer(ShieldInstance_t *instance);
void Shield_ResetTimer(ShieldInstance_t *instance);

#endif /* SIMPLE_SHIELD_H */
