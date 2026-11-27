/*
 * Copyright (c) 2023 Alif Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef HCI_UART_H_
#define HCI_UART_H_

#include <stdint.h>
#include <stdbool.h>

#define HCI_UART_BAUD_RATE 1000000 /* 1Mbit/s */

int32_t hci_uart_init(void);
void hci_uart_read(uint8_t *bufptr, uint32_t size, void (*callback)(void *, uint8_t), void *dummy);
void hci_uart_write(uint8_t *bufptr, uint32_t size, void (*callback)(void *, uint8_t), void *dummy);
void hci_uart_flow_on(void);
bool hci_uart_flow_off(void);

#endif /* HCI_UART_H_ */
