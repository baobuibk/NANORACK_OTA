/*
 * UART7.c
 *
 *  Created on: Nov 16, 2024
 *      Author: thanh
 */

#include "UART.h"
#include <stm32f7xx.h>
#include <stm32f7xx_ll_usart.h>


// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
usart_meta_t UART7_meta;
usart_meta_t *p_UART7_meta = &UART7_meta;

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
// USART FUNCTIONS

void UART7_IRQHandler(void)
{
    uint8_t data;
    if (LL_USART_IsActiveFlag_TXE(UART7)) {
        if (!rbuffer_empty(&p_UART7_meta->rb_tx)) {
            data = rbuffer_remove(&p_UART7_meta->rb_tx);
            LL_USART_TransmitData8(UART7, (uint8_t)data);
        } else {
            LL_USART_DisableIT_TXE(UART7);
        }
    }
    if ((LL_USART_IsActiveFlag_RXNE(UART7) != RESET) && (LL_USART_IsEnabledIT_RXNE(UART7) != RESET)) {
        unsigned char data = LL_USART_ReceiveData8(UART7);
        if ((LL_USART_IsActiveFlag_ORE(UART7) != RESET) ||
            (LL_USART_IsActiveFlag_FE(UART7) != RESET) ||
            (LL_USART_IsActiveFlag_NE(UART7) != RESET)) {
            LL_USART_ClearFlag_ORE(UART7);
            LL_USART_ClearFlag_FE(UART7);
            LL_USART_ClearFlag_NE(UART7);
        } else {
            if (!rbuffer_full(&p_UART7_meta->rb_rx)) {
                rbuffer_insert(data, &p_UART7_meta->rb_rx);
            }
        }
        return;
    }
}

void UART7_init(void) {
    rbuffer_init(&p_UART7_meta->rb_tx); // Init Tx buffer
    rbuffer_init(&p_UART7_meta->rb_rx); // Init Rx buffer
    LL_USART_EnableIT_RXNE(UART7);
}

void UART7_send_char(char c) {
    while (rbuffer_full(&p_UART7_meta->rb_tx))
        ;
    rbuffer_insert(c, &p_UART7_meta->rb_tx);
    LL_USART_EnableIT_TXE(UART7);
}

void UART7_send_string(const char *str) {
    while (*str) {
        UART7_send_char(*str++);
    }
}

void UART7_send_array(const char *str, uint8_t len) {
    uint8_t udx;
    for (udx = 0; udx < len; udx++) {
        UART7_send_char(*str++);
    }
}

uint8_t UART7_rx_count(void) {
    return rbuffer_count(&p_UART7_meta->rb_rx);
}

uint16_t UART7_read_char(void) {
    if (!rbuffer_empty(&p_UART7_meta->rb_rx)) {
        return (uint16_t)rbuffer_remove(&p_UART7_meta->rb_rx);
    } else {
        return 0xFFFF; // Trả về giá trị lỗi khi buffer rỗng
    }
}

void UART7_close(void) {
    while (!rbuffer_empty(&p_UART7_meta->rb_tx))
        ; // Chờ truyền hết dữ liệu trong buffer
    LL_USART_Disable(UART7); // Tắt UART7
}

void UART7_SetBaudRate(uint32_t Baud) {
    while (!rbuffer_empty(&p_UART7_meta->rb_tx))
        ; // Chờ truyền hết dữ liệu
    LL_USART_Disable(UART7);
    LL_USART_InitTypeDef USART_InitStruct = {0};
    USART_InitStruct.BaudRate = Baud;
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(UART7, &USART_InitStruct);
    LL_USART_ConfigAsyncMode(UART7);
    LL_USART_Enable(UART7);
}

volatile ringbuffer_t* uart_get_UART7_rx_buffer_address(void) {
    return &p_UART7_meta->rb_rx; // Trả về địa chỉ buffer Rx
}
