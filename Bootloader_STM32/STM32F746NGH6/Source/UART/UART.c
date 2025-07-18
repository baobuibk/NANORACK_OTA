/*
 * USART6.c
 *
 *  Created on: Nov 16, 2024
 *      Author: thanh
 */

#include "UART.h"

#include <stm32f7xx.h>
#include <stm32f7xx_ll_usart.h>


// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
usart_meta_t USART6_meta;
usart_meta_t *p_USART6_meta = &USART6_meta;

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
// USART FUNCTIONS

void USART6_IRQ(void) {
    uint8_t data;
    if (LL_USART_IsActiveFlag_TXE(USART6)) {
        if (!rbuffer_empty(&p_USART6_meta->rb_tx)) {
            data = rbuffer_remove(&p_USART6_meta->rb_tx);
            LL_USART_TransmitData8(USART6, (uint8_t)data);
        } else {
            LL_USART_DisableIT_TXE(USART6);
        }
    }
    if ((LL_USART_IsActiveFlag_RXNE(USART6) != RESET) && (LL_USART_IsEnabledIT_RXNE(USART6) != RESET)) {
        unsigned char data = LL_USART_ReceiveData8(USART6);

        if ((LL_USART_IsActiveFlag_ORE(USART6) != RESET) ||
            (LL_USART_IsActiveFlag_FE(USART6) != RESET) ||
            (LL_USART_IsActiveFlag_NE(USART6) != RESET)) {
            LL_USART_ClearFlag_ORE(USART6);
            LL_USART_ClearFlag_FE(USART6);
            LL_USART_ClearFlag_NE(USART6);
        } else {
            if (!rbuffer_full(&p_USART6_meta->rb_rx)) {
                rbuffer_insert(data, &p_USART6_meta->rb_rx);
            }
        }
        return;
    }
}

void USART6_init(void) {
    rbuffer_init(&p_USART6_meta->rb_tx); // Init Tx buffer
    rbuffer_init(&p_USART6_meta->rb_rx); // Init Rx buffer
    LL_USART_EnableIT_RXNE(USART6);
}

void USART6_send_char(char c) {
    while (rbuffer_full(&p_USART6_meta->rb_tx))
        ;
    rbuffer_insert(c, &p_USART6_meta->rb_tx);
    LL_USART_EnableIT_TXE(USART6);
}

void USART6_send_string(const char *str) {
    while (*str) {
        USART6_send_char(*str++);
    }
}

void USART6_send_array(const char *str, uint8_t len) {
    uint8_t udx;
    for (udx = 0; udx < len; udx++) {
        USART6_send_char(*str++);
    }
}

uint8_t USART6_rx_count(void) {
    return rbuffer_count(&p_USART6_meta->rb_rx);
}

uint16_t USART6_read_char(void) {
    if (!rbuffer_empty(&p_USART6_meta->rb_rx)) {
        return (uint16_t)rbuffer_remove(&p_USART6_meta->rb_rx);
    } else {
        return 0xFFFF; // Trả về giá trị lỗi khi buffer rỗng
    }
}

void USART6_close(void) {
    while (!rbuffer_empty(&p_USART6_meta->rb_tx))
        ; // Chờ truyền hết dữ liệu trong buffer
    LL_USART_Disable(USART6); // Tắt USART6
}

void USART6_SetBaudRate(uint32_t Baud) {
    while (!rbuffer_empty(&p_USART6_meta->rb_tx))
        ; // Chờ truyền hết dữ liệu
    LL_USART_Disable(USART6);
    LL_USART_InitTypeDef USART_InitStruct = {0};
    USART_InitStruct.BaudRate = Baud;
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(USART6, &USART_InitStruct);
    LL_USART_ConfigAsyncMode(USART6);
    LL_USART_Enable(USART6);
}

volatile ringbuffer_t* uart_get_USART6_rx_buffer_address(void) {
    return &p_USART6_meta->rb_rx; // Trả về địa chỉ buffer Rx
}
