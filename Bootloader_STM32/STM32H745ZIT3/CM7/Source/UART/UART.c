/*
 * USART1.c
 *
 *  Created on: Nov 16, 2024
 *      Author: thanh
 */

#include "UART.h"

#include <stm32h7xx.h>
#include <stm32h7xx_ll_usart.h>


// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
usart_meta_t USART1_meta;
usart_meta_t *p_USART1_meta = &USART1_meta;

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
// USART FUNCTIONS

void USART1_IRQ(void) {
    uint8_t data;
    if (LL_USART_IsActiveFlag_TXE(USART1)) {
        if (!rbuffer_empty(&p_USART1_meta->rb_tx)) {
            data = rbuffer_remove(&p_USART1_meta->rb_tx);
            LL_USART_TransmitData8(USART1, (uint8_t)data);
        } else {
            LL_USART_DisableIT_TXE(USART1);
        }
    }
    if ((LL_USART_IsActiveFlag_RXNE(USART1) != RESET) && (LL_USART_IsEnabledIT_RXNE(USART1) != RESET)) {
        unsigned char data = LL_USART_ReceiveData8(USART1);

        if ((LL_USART_IsActiveFlag_ORE(USART1) != RESET) ||
            (LL_USART_IsActiveFlag_FE(USART1) != RESET) ||
            (LL_USART_IsActiveFlag_NE(USART1) != RESET)) {
            LL_USART_ClearFlag_ORE(USART1);
            LL_USART_ClearFlag_FE(USART1);
            LL_USART_ClearFlag_NE(USART1);
        } else {
            if (!rbuffer_full(&p_USART1_meta->rb_rx)) {
                rbuffer_insert(data, &p_USART1_meta->rb_rx);
            }
        }
        return;
    }
}

void USART1_init(void) {
    rbuffer_init(&p_USART1_meta->rb_tx); // Init Tx buffer
    rbuffer_init(&p_USART1_meta->rb_rx); // Init Rx buffer
    LL_USART_EnableIT_RXNE(USART1);
    uint8_t status[3] = {1, 'O', 'K'};
    USART1_send_array((const char*)status, 3);
}

void USART1_send_char(char c) {
    while (rbuffer_full(&p_USART1_meta->rb_tx))
        ;
    rbuffer_insert(c, &p_USART1_meta->rb_tx);
    LL_USART_EnableIT_TXE(USART1);
}

void USART1_send_string(const char *str) {
    while (*str) {
        USART1_send_char(*str++);
    }
}

void USART1_send_array(const char *str, uint8_t len) {
    uint8_t udx;
    for (udx = 0; udx < len; udx++) {
        USART1_send_char(*str++);
    }
}

uint8_t USART1_rx_count(void) {
    return rbuffer_count(&p_USART1_meta->rb_rx);
}

uint16_t USART1_read_char(void) {
    if (!rbuffer_empty(&p_USART1_meta->rb_rx)) {
        return (uint16_t)rbuffer_remove(&p_USART1_meta->rb_rx);
    } else {
        return 0xFFFF; // Trả về giá trị lỗi khi buffer rỗng
    }
}

void USART1_close(void) {
    while (!rbuffer_empty(&p_USART1_meta->rb_tx))
        ; // Chờ truyền hết dữ liệu trong buffer
    LL_USART_Disable(USART1); // Tắt USART1
}

void USART1_SetBaudRate(uint32_t Baud) {
    while (!rbuffer_empty(&p_USART1_meta->rb_tx))
        ; // Chờ truyền hết dữ liệu
    LL_USART_Disable(USART1);
    LL_USART_InitTypeDef USART_InitStruct = {0};
    USART_InitStruct.BaudRate = Baud;
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(USART1, &USART_InitStruct);
    LL_USART_ConfigAsyncMode(USART1);
    LL_USART_Enable(USART1);
}

volatile ringbuffer_t* uart_get_USART1_rx_buffer_address(void) {
    return &p_USART1_meta->rb_rx; // Trả về địa chỉ buffer Rx
}
