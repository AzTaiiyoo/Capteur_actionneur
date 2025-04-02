#ifndef HC_SR04_H
#define HC_SR04_H

#include "stm32f4xx_hal.h"

// Définition des broches
#define HC_SR04_TRIGGER_PIN    GPIO_PIN_8   // PA8
#define HC_SR04_ECHO_PIN       GPIO_PIN_9   // PA9
#define HC_SR04_GPIO_PORT      GPIOA

// Structure pour gérer l'état du capteur HC-SR04
typedef struct {
    GPIO_TypeDef* trigger_port;
    uint16_t trigger_pin;
    GPIO_TypeDef* echo_port;
    uint16_t echo_pin;
    TIM_HandleTypeDef* timer;
    float distance; // Champ pour stocker la distance mesurée
} HC_SR04;

// Prototypes de fonctions
HC_SR04* HC_SR04_get_instance(void); // Récupérer l'instance du capteur
void HC_SR04_init(void);             // Initialiser le capteur
float HC_SR04_get_distance(HC_SR04* sensor); // Calculer la distance
void HC_SR04_update(HC_SR04* sensor); // Mettre à jour la distance

#endif /* HC_SR04_H */