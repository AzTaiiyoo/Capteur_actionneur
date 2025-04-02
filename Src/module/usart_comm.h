#ifndef USART_COMM_H
#define USART_COMM_H

#include <stddef.h>

/* Function prototypes */
void sendMessage(char* message);
int receiveMessage(char* buffer, size_t maxSize);

#endif /* USART_COMM_H */