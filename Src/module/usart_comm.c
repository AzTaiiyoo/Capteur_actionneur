/* INCLUDES */
/* -------- */
#include "stm32f4xx_hal.h"
#include <stdlib.h>
#include <string.h>
#include "usart.h"  // Au lieu de uart_serie.h
#include <stdbool.h>
#include "usart_comm.h"

/* MACROS */
/* ------ */
#define USART_TIMEOUT 1000 // Timeout for USART communication in milliseconds
#define USART_BUFFER_SIZE 256 // Buffer size for USART communication

/* VARIABLES*/
/* --------- */
extern UART_HandleTypeDef huart2; // USART2 handle (généré par CubeMX)

uint8_t rxBuffer[USART_BUFFER_SIZE]; // Buffer for receiving data
uint8_t txBuffer[USART_BUFFER_SIZE]; // Buffer for sending data

/* FUNCTION DEFINITIONS */
/* -------------------- */
void sendMessage(char* message) {
    size_t messageLen = strlen(message);
    
    // Vérification que le message + délimiteurs tient dans le buffer
    if (messageLen < USART_BUFFER_SIZE - 3) { // -3 pour '<', '>' et '\0'
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
        HAL_UART_Transmit(&huart2, txBuffer, messageLen + 2, USART_TIMEOUT);
    }
    else {
        // Message d'erreur
        const char* errorMsg = "Error: Message too long";
        size_t errorLen = strlen(errorMsg);
        
        txBuffer[0] = 0x3C; // '<'
        memcpy(txBuffer + 1, errorMsg, errorLen);
        txBuffer[errorLen + 1] = 0x3E; // '>'
        txBuffer[errorLen + 2] = '\0'; // Terminaison
        
        HAL_UART_Transmit(&huart2, txBuffer, errorLen + 2, USART_TIMEOUT);
    }
}

int receiveMessage(char* buffer, size_t maxSize) {
    uint8_t c;
    int index = 0;
    bool startReceived = false;
    char echoChar[2] = {0}; // Pour l'écho d'un caractère
    
    // Vider le buffer de destination
    memset(buffer, 0, maxSize);
    
    // Attendre indéfiniment
    while (1) {
        // Lire un caractère avec un timeout beaucoup plus long (3 secondes)
        if (HAL_UART_Receive(&huart2, &c, 1, 3000) == HAL_OK) {
            // Créer un écho du caractère reçu
            echoChar[0] = c;
            HAL_UART_Transmit(&huart2, (uint8_t*)echoChar, 1, 10);
            
            // Vérifier si c'est le caractère de début
            if (c == '<' && !startReceived) {
                startReceived = true;
                index = 0; // Réinitialiser l'index
                continue;
            }
            
            // Vérifier si c'est le caractère de fin
            if (c == '>' && startReceived) {
                buffer[index] = '\0'; // Terminer la chaîne
                HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, 10);
                return index; // Commande complète
            }
            
            // Si on est en train de recevoir une commande, stocker le caractère
            if (startReceived) {
                if (index < maxSize - 1) {
                    buffer[index++] = c;
                } else {
                    // Buffer overflow
                    HAL_UART_Transmit(&huart2, (uint8_t*)"\r\nBuffer overflow\r\n", 18, 100);
                    startReceived = false;
                    index = 0;
                }
            }
        } else if (startReceived) {
            // Timeout seulement si on a commencé une commande
            HAL_UART_Transmit(&huart2, (uint8_t*)"\r\nTimeout pendant la saisie\r\n", 29, 100);
            startReceived = false;
            index = 0;
        }
        // Sinon, on continue d'attendre un début de commande
    }
}