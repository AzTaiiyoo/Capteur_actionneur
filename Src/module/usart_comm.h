/**
 * @file usart_comm.h
 * @brief Interface pour la communication USART
 * @details Ce module fournit les fonctions nécessaires pour envoyer et recevoir
 * des messages formatés via la liaison série USART2.
 * @author STM32 Team
 * @date 2025
 */

 #ifndef USART_COMM_H
 #define USART_COMM_H
 
 #include <stddef.h>
 
 /**
  * @brief Envoie un message formaté via USART
  * @param message Pointeur vers la chaîne de caractères à envoyer
  * @details Le message est encapsulé entre les caractères '<' et '>'
  */
 void sendMessage(char* message);
 
 /**
  * @brief Reçoit un message formaté via USART de manière non-bloquante
  * @param buffer Pointeur vers le buffer où stocker le message reçu
  * @param maxSize Taille maximale du buffer
  * @return Nombre de caractères reçus, ou -1 si aucun message complet n'a été reçu
  * @details Cette fonction est non-bloquante. Elle retourne -1 si aucun message
  * complet (délimité par '<' et '>') n'a été reçu.
  */
 int receiveMessage(char* buffer, size_t maxSize);
 
 #endif /* USART_COMM_H */
 