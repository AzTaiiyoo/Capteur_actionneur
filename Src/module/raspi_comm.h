#ifndef RASPI_COMM_H
#define RASPI_COMM_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

/**
 * @brief Initialise la communication avec la Raspberry Pi
 */
void RasPi_Init(void);

/**
 * @brief Envoie un message à la Raspberry Pi
 * @param message Pointeur vers la chaîne de caractères à envoyer
 * @details Le message est encapsulé entre les caractères '<' et '>'
 */
void RasPi_SendMessage(char* message);

/**
 * @brief Gère la réception des commandes de la Raspberry Pi
 * @details Cette fonction est non-bloquante et doit être appelée régulièrement
 */
void RasPi_HandleCommunication(void);

#endif /* RASPI_COMM_H */