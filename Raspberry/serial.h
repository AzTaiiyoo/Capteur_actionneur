/**
 * @file serial.h
 * @brief Interface pour la communication série
 * @details Ce module fournit les fonctions nécessaires pour communiquer avec
 * la STM32 via le port série UART
 */

 #ifndef SERIAL_H
 #define SERIAL_H
 
 #include "common.h"
 
 /**
  * @brief Ouvre la connexion série avec la STM32
  * @return Descripteur de fichier pour la connexion série, ou -1 en cas d'erreur
  */
 int openSerialConnection(void);
 
 /**
  * @brief Envoie une commande formatée à la STM32
  * @param fd Descripteur de fichier pour la connexion série
  * @param command Commande à envoyer
  */
 void sendCommand(int fd, const char* command);
 
 /**
  * @brief Thread pour recevoir les données de la STM32
  * @param arg Argument du thread (non utilisé)
  * @return NULL
  */
 void* receiveDataThread(void* arg);
 
 #endif /* SERIAL_H */