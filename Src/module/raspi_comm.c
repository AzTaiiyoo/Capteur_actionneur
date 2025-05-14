#include "module/raspi_comm.h"
#include "module/hcsr04.h"
#include "usart.h"
#include "usart_comm.h"
#include <string.h>
#include <stdio.h>

// Le même format de commande que celui utilisé avec l'ordinateur
extern void processCommand(char* command);
extern int currentState; // Pour accéder à l'état courant du système

// Buffer de réception pour les commandes de la Raspberry Pi
#define RPI_BUFFER_SIZE 128
static uint8_t rxBuffer[RPI_BUFFER_SIZE];
static uint8_t cmdBuffer[RPI_BUFFER_SIZE]; // Buffer de traitement des commandes
static uint8_t txBuffer[RPI_BUFFER_SIZE];
static uint32_t lastReceiveTime = 0;
static int bufferIndex = 0;

/**
 * @brief Initialise la communication avec la Raspberry Pi
 */
void RasPi_Init(void) {
    // Réinitialiser les buffers
    memset(rxBuffer, 0, RPI_BUFFER_SIZE);
    memset(cmdBuffer, 0, RPI_BUFFER_SIZE);
    bufferIndex = 0;
    lastReceiveTime = HAL_GetTick();
    
    // Message d'initialisation pour debug
    sendMessage("RasPi communication initialized with improved buffer management");
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
        txBuffer[0] = '<';
        
        // Copie sécurisée du message
        memcpy(txBuffer + 1, message, messageLen);
        
        // Fin du message
        txBuffer[messageLen + 1] = '>';
        txBuffer[messageLen + 2] = '\0'; // Terminaison de la chaîne
        
        // Envoi du message complet
        HAL_UART_Transmit(&huart1, txBuffer, messageLen + 2, 100);
    }
}

/**
 * @brief Traite une commande reçue complète
 * @param cmd Commande à traiter (sans les délimiteurs)
 */
void processRasPiCommand(char* cmd) {
    char debugMsg[64];
    sprintf(debugMsg, "Processing command: '%s'", cmd);
    sendMessage(debugMsg);
    
    // État avant traitement
    sprintf(debugMsg, "Before processCommand, state: %d", currentState);
    sendMessage(debugMsg);
    
    if (strcmp(cmd, "getdistance") == 0) {
        // Obtenir le capteur et sa dernière distance mesurée
        HC_SR04* sensor = HC_SR04_get_instance();
        char response[32];
        sprintf(response, "LastDistance: %.2f cm", sensor->distance);
        RasPi_SendMessage(response);
        sendMessage("Distance command processed");
    } else {
        // Traiter les autres commandes
        processCommand(cmd);
        
        // État après traitement
        HAL_Delay(10);
        sprintf(debugMsg, "After processCommand, state: %d", currentState);
        sendMessage(debugMsg);
        
        // Confirmation à la Raspberry Pi
        char response[64];
        sprintf(response, "Command '%s' processed, new state: %d", cmd, currentState);
        RasPi_SendMessage(response);
    }
}

/**
 * @brief Gère la réception des commandes de la Raspberry Pi
 * @details Cette fonction est non-bloquante et doit être appelée régulièrement
 */
void RasPi_HandleCommunication(void) {
    uint8_t c;
    uint32_t currentTime = HAL_GetTick();
    
    // Nouvelle approche : recevoir le plus possible de caractères à chaque appel
    while (HAL_UART_Receive(&huart1, &c, 1, 0) == HAL_OK) {
        // Mettre à jour le temps de dernière réception
        lastReceiveTime = currentTime;
        
        // Ajouter au buffer si l'espace est disponible
        if (bufferIndex < RPI_BUFFER_SIZE - 1) {
            rxBuffer[bufferIndex++] = c;
            
            // Echo pour debug (optionnel - peut être désactivé)
            HAL_UART_Transmit(&huart1, &c, 1, 10);
        }
    }
    
    // Traiter le buffer si des données sont présentes et qu'un timeout s'est produit
    // (pour s'assurer que la commande complète a été reçue)
    if (bufferIndex > 0 && currentTime - lastReceiveTime > 50) { // 50ms timeout
        // Assurez-vous que le buffer est terminé par NULL
        rxBuffer[bufferIndex] = '\0';
        
        // Debug - afficher le buffer brut
        char debugBuf[64];
        sprintf(debugBuf, "Raw buffer: '%s' (length: %d)", rxBuffer, bufferIndex);
        sendMessage(debugBuf);
        
        // Rechercher des commandes complètes dans le buffer (format <commande>)
        char *start = NULL;
        char *end = NULL;
        char *ptr = (char*)rxBuffer;
        
        while ((start = strchr(ptr, '<')) != NULL) {
            end = strchr(start + 1, '>');
            if (end != NULL) {
                // Extraire la commande sans les délimiteurs
                size_t cmdLen = end - start - 1;
                if (cmdLen > 0 && cmdLen < RPI_BUFFER_SIZE - 1) {
                    memcpy(cmdBuffer, start + 1, cmdLen);
                    cmdBuffer[cmdLen] = '\0';
                    
                    // Traiter la commande
                    processRasPiCommand((char*)cmdBuffer);
                }
                
                // Passer à la position après cette commande
                ptr = end + 1;
            } else {
                // Pas de '>' trouvé, sortir de la boucle
                break;
            }
        }
        
        // Réinitialiser le buffer après traitement
        memset(rxBuffer, 0, RPI_BUFFER_SIZE);
        bufferIndex = 0;
    }
}