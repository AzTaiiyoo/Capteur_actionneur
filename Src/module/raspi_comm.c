#include "module/raspi_comm.h"
#include "module/hcsr04.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

// Le même format de commande que celui utilisé avec l'ordinateur
extern void processCommand(char* command);

// Buffer de réception pour les commandes de la Raspberry Pi
#define RPI_BUFFER_SIZE 32
static uint8_t rxBuffer[RPI_BUFFER_SIZE];
static uint8_t txBuffer[RPI_BUFFER_SIZE];
static int bufferIndex = 0;
static int startReceived = 0;

/**
 * @brief Initialise la communication avec la Raspberry Pi
 */
void RasPi_Init(void) {
    // Réinitialiser le buffer de réception
    memset(rxBuffer, 0, RPI_BUFFER_SIZE);
    bufferIndex = 0;
    startReceived = 0;
}

/**
 * @brief Envoie un message à la Raspberry Pi
 * @param message Pointeur vers la chaîne de caractères à envoyer
 * @details Le message est encapsulé entre les caractères '<' et '>'
 */
void RasPi_SendMessage(char* message) {
    size_t messageLen = strlen(message);
    
    // Vérification que le message + délimiteurs tient dans le buffer
    if (messageLen < RPI_BUFFER_SIZE - 3) { // -3 pour '<', '>' et '\0'
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
        HAL_UART_Transmit(&huart1, txBuffer, messageLen + 2, 100);
    }
}

/**
 * @brief Gère la réception des commandes de la Raspberry Pi
 * @details Cette fonction est non-bloquante et doit être appelée régulièrement
 */
void RasPi_HandleCommunication(void) {
    uint8_t c;
    
    // Recevoir un caractère (non-bloquant)
    if (HAL_UART_Receive(&huart1, &c, 1, 0) == HAL_OK) {
        // Faire écho du caractère reçu
        HAL_UART_Transmit(&huart1, &c, 1, 10);
        
        if (c == '<') {
            // Début d'un message
            bufferIndex = 0;
            startReceived = 1;
        } else if (c == '>' && startReceived) {
            // Fin d'un message, traiter la commande
            rxBuffer[bufferIndex] = '\0';
            
            // Ajouter une commande spéciale pour obtenir la dernière distance
            if (strcmp((char*)rxBuffer, "getdistance") == 0) {
                // Obtenir le capteur et sa dernière distance mesurée
                HC_SR04* sensor = HC_SR04_get_instance();
                char response[32];
                sprintf(response, "LastDistance: %.2f cm", sensor->distance);
                RasPi_SendMessage(response);
            } else {
                // Traiter les autres commandes normalement
                processCommand((char*)rxBuffer);
            }
            
            // Réinitialiser le buffer
            bufferIndex = 0;
            startReceived = 0;
        } else if (startReceived && bufferIndex < RPI_BUFFER_SIZE - 1) {
            // Ajouter le caractère au buffer
            rxBuffer[bufferIndex++] = c;
        }
    }
}