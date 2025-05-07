/**
 * @file gpio.h
 * @brief Interface pour la gestion des broches GPIO
 * @details Ce module fournit les fonctions nécessaires pour configurer et
 * contrôler les broches GPIO de la Raspberry Pi
 */

 #ifndef GPIO_H
 #define GPIO_H
 
 #include "common.h"
 
 /**
  * @brief Initialise le GPIO pour le servo-moteur
  * @return 0 en cas de succès, -1 en cas d'échec
  */
 int setupGPIO(void);
 
 /**
  * @brief Exporte une broche GPIO pour la rendre utilisable
  * @param pin Numéro de la broche GPIO à exporter
  */
 void exportGPIO(int pin);
 
 /**
  * @brief Définit la direction d'une broche GPIO (entrée ou sortie)
  * @param pin Numéro de la broche GPIO
  * @param direction Direction ("in" pour entrée, "out" pour sortie)
  */
 void setGPIODirection(int pin, const char* direction);
 
 /**
  * @brief Écrit une valeur sur une broche GPIO
  * @param pin Numéro de la broche GPIO
  * @param value Valeur à écrire (0 ou 1)
  */
 void writeGPIO(int pin, int value);
 
 #endif /* GPIO_H */