#ifndef UART_SERIE_H
#define UART_SERIE_H

#include <../stm32f4xx_hal_uart.h>

/* FUNCTION PROTOTYPES */
/* ------------------- */

// Function to send message via UART4 to computer
void send_message(char* message);

// Function to receive message via UART4 from computer
void receive_message(void);

// Function to verify if communication protocol is valid
void test_communication(void);

#endif // UART_SERIE_