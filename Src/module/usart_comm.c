
/**
 * @file usart_comm.c
 * @brief Implémentation des fonctions de communication USART
 */

/* INCLUDES */
/* -------- */
#include "stm32f4xx_hal.h"
#include <stdlib.h>
#include <string.h>
#include "usart.h"
#include <stdbool.h>
#include "usart_comm.h"

/* MACROS */
/* ------ */
/**
 * @def USART_TIMEOUT
 * @brief Timeout pour la communication USART en millisecondes
 */
#define USART_TIMEOUT 1000 // Timeout for USART communication in milliseconds

/**
 * @def USART_BUFFER_SIZE
 * @brief Taille des buffers pour la communication USART
 */
#define USART_BUFFER_SIZE 256 // Buffer size for USART communication

/* VARIABLES*/
/* --------- */
/** @brief Handle UART pour USART2 (généré par CubeMX) */
extern UART_HandleTypeDef huart2;

/** @brief Buffer pour la réception de données */
uint8_t rxBuffer[USART_BUFFER_SIZE];

/** @brief Buffer pour l'envoi de données */
uint8_t txBuffer[USART_BUFFER_SIZE];

/* FUNCTION DEFINITIONS */
/* -------------------- */

/**
 * @brief Envoie un message formaté via USART
 * @param message Pointeur vers la chaîne de caractères à envoyer
 * @details Le message est encapsulé entre les caractères '<' et '>'
 */
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

/**
 * @brief Reçoit un message formaté via USART de manière non-bloquante
 * @param buffer Pointeur vers le buffer où stocker le message reçu
 * @param maxSize Taille maximale du buffer
 * @return Nombre de caractères reçus, ou -1 si aucun message complet n'a été reçu
 * @details Cette fonction est non-bloquante. Elle retourne -1 si aucun message
 * complet (délimité par '<' et '>') n'a été reçu. Les caractères reçus sont
 * également envoyés en écho sur le terminal.
 */
int receiveMessage(char* buffer, size_t maxSize) {
    static uint8_t receiveBuf[32];  // Buffer statique pour conserver l'état entre les appels
    static int index = 0;           // Index courant dans le buffer
    static bool startReceived = false; // Indique si on a reçu le caractère de début
    
    uint8_t c;
    
    // Vérifier si des données sont disponibles (non-bloquant)
    if (HAL_UART_Receive(&huart2, &c, 1, 0) == HAL_OK) {
        // Un caractère a été reçu
        char echoChar[2] = {c, 0};
        HAL_UART_Transmit(&huart2, (uint8_t*)echoChar, 1, 10);
        
        // Vérifier si c'est le caractère de début
        if (c == '<' && !startReceived) {
            startReceived = true;
            index = 0;
            return -1;  // Pas encore de commande complète
        }
        
        // Vérifier si c'est le caractère de fin
        if (c == '>' && startReceived) {
            receiveBuf[index] = '\0';  // Terminer la chaîne
            strncpy(buffer, (char*)receiveBuf, maxSize);
            HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, 10);
            
            // Réinitialiser l'état
            startReceived = false;
            index = 0;
            
            return strlen(buffer);  // Commande complète
        }
        
        // Si on est en train de recevoir une commande, stocker le caractère
        if (startReceived) {
            if (index < maxSize - 1) {
                receiveBuf[index++] = c;
            } else {
                // Buffer overflow
                HAL_UART_Transmit(&huart2, (uint8_t*)"\r\nBuffer overflow\r\n", 18, 100);
                startReceived = false;
                index = 0;
                return -1;
            }
        }
    }
    
    // Aucune commande complète n'a été reçue
    return -1;
}

/**
 * @brief Fonction de démonstration pour tester la communication UART
 * @details Cette fonction envoie un message via UART et attend une réponse.
 * Si une réponse est reçue, elle est retransmise sur le terminal.
 */
void USART_Demo(void) {
    char receiveBuffer[32]; // Buffer pour stocker les messages reçus
    const char* demoMessage = "Bonjour, bienvenu dans ce projet !";
    Send_Message((char*)demoMessage);

    // Boucle pour attendre une réponse
    while (1) {
        int receivedLength = Receive_Message(receiveBuffer, sizeof(receiveBuffer));
        
        if (receivedLength > 0) {
            Send_Message("Message reçu :");
            Send_Message(receiveBuffer);
        }

        HAL_Delay(100);
    }
}