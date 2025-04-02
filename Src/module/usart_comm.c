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
    uint32_t startTime = HAL_GetTick();
    
    // Vider le buffer de destination
    memset(buffer, 0, maxSize);
    
    // Attendre jusqu'au timeout
    while ((HAL_GetTick() - startTime) < USART_TIMEOUT) {
        // Lire un caractère
        if (HAL_UART_Receive(&huart2, &c, 1, 10) == HAL_OK) {
            // Stocker aussi dans le buffer global pour référence
            if (index < USART_BUFFER_SIZE - 1) {
                rxBuffer[index] = c;
            }
            
            // Vérifier si c'est le caractère de début
            if (c == '<' && !startReceived) {
                startReceived = true;
                index = 0; // Réinitialiser l'index pour le buffer de destination
                continue; // Passer au caractère suivant
            }
            
            // Vérifier si c'est le caractère de fin
            if (c == '>' && startReceived) {
                buffer[index] = '\0'; // Terminer la chaîne
                rxBuffer[index + 1] = '\0'; // Aussi terminer le buffer global
                return index; // Retourner la longueur du message
            }
            
            // Si on a déjà reçu le caractère de début, stocker les caractères
            if (startReceived) {
                if (index < maxSize - 1) { // Laisser de la place pour '\0'
                    buffer[index++] = c;
                } else {
                    // Buffer overflow
                    return -1;
                }
            }
        }
    }
    
    // Timeout atteint sans recevoir un message complet
    return -1;
}