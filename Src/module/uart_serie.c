/* INCLUDES */
/* -------- */
#include <../stm32f4xx_hal_uart.h>
#include <stdlib.h>
#include "uart_serie.h"

/* MACROS */
/* ------ */
#define UART4_TIMEOUT 1000 // Timeout for UART4 communication in milliseconds
#define UART4_BUFFER_SIZE 256 // Buffer size for UART4 communication

/* VARIABLES*/
/* --------- */

UART_HandleTypeDef huart4; // UART4 handle

uint8_t rxBuffer[UART4_BUFFER_SIZE]; // Buffer for receiving data
uint8_t txBuffer[UART4_BUFFER_SIZE]; // Buffer for sending data


/* FUNCTION DEFINITIONS */
/* -------------------- */
void sendMessage(char* message) {
    size_t messageLen = strlen(message);
    
    // Vérification que le message + délimiteurs tient dans le buffer
    if (messageLen < UART4_BUFFER_SIZE - 3) { // -3 pour '<', '>' et '\0'

        // Nettoyage du buffer d'envoi
        memset(txBuffer, 0, sizeof(txBuffer));

        // Début du message
        txBuffer[0] = 0x3C; // '<'
        
        // Copie sécurisée du message
        memcpy(txBuffer + 1, message, messageLen);
        
        // Fin du message
        txBuffer[messageLen + 1] = 0x3E; // '>'
        txBuffer[messageLen + 2] = '\0'; // Terminaison de la chaîne
        
        // Envoi du message (sans le '\0' final)
        HAL_UART_Transmit(&huart4, txBuffer, messageLen + 2, UART4_TIMEOUT);
    }
    else {
        // Message d'erreur
        const char* errorMsg = "Error: Message too long";
        size_t errorLen = strlen(errorMsg);
        
        txBuffer[0] = 0x3C; // '<'
        memcpy(txBuffer + 1, errorMsg, errorLen);
        txBuffer[errorLen + 1] = 0x3E; // '>'
        txBuffer[errorLen + 2] = '\0'; // Terminaison
        
        HAL_UART_Transmit(&huart4, txBuffer, errorLen + 2, UART4_TIMEOUT);
    }
}

void receiveMessage(char * buffer, size_t max_size){
    
}