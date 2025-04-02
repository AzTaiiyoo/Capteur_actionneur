#include "hcsr04.h"
#include "gpio.h"
#include "tim.h"
#include <stdio.h>
#include <stdint.h>
#include "tim.h"

// Définition du capteur HC-SR04
static HC_SR04 sensor_instance = {0};

// Récupération de l'instance du capteur (Singleton)
HC_SR04* HC_SR04_get_instance(void) {
    return &sensor_instance;
}

// Initialisation du capteur HC-SR04
void HC_SR04_init(void) {
    // Configuration Trigger (PA8) en sortie
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Configuration Echo (PA9) en entrée avec pull-down
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN; // Ajout d'un pull-down pour stabiliser le signal
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Initialisation du timer pour mesurer le temps
    HAL_TIM_Base_Start(&htim1);
}

// Mesure de la distance avec Timer pour meilleure précision
float HC_SR04_get_distance(HC_SR04* sensor) {
    uint32_t start_time, stop_time;

    // Envoi d'une impulsion Trigger de 10 µs
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
    HAL_DelayMicroseconds(10); // Délai précis en microsecondes
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);

    // Attendre l'activation de Echo (avec timeout)
    uint32_t timeout = HAL_GetTick() + 10; // Timeout de 10 ms
    while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9) == GPIO_PIN_RESET) {
        if (HAL_GetTick() > timeout) {
            return -1.0f; // Timeout, retourner une erreur
        }
    }

    // Capturer le temps de départ
    start_time = __HAL_TIM_GET_COUNTER(&htim1);

    // Attendre la fin de l'impulsion Echo (avec timeout)
    timeout = HAL_GetTick() + 10; // Timeout de 10 ms
    while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9) == GPIO_PIN_SET) {
        if (HAL_GetTick() > timeout) {
            return -1.0f; // Timeout, retourner une erreur
        }
    }

    // Capturer le temps de fin
    stop_time = __HAL_TIM_GET_COUNTER(&htim1);

    // Calcul du temps de vol (µs) et distance en cm
    uint32_t duration = stop_time - start_time;
    float distance = (duration * 0.0343f) / 2.0f;

    // Si hors de la plage 5-25cm, retourner -1 (erreur)
    if (distance < 5 || distance > 25) {
        return -1.0f;
    }

    return distance;
}

// Mise à jour de la distance et contrôle des LEDs
void HC_SR04_update(HC_SR04* sensor) {
    static uint32_t last_update = 0;
    if (HAL_GetTick() - last_update >= 250) {
        // Mettre à jour la distance mesurée
        sensor->distance = HC_SR04_get_distance(sensor);
        last_update = HAL_GetTick();

        // Indiquer l'état de la distance avec les LEDs
        if (sensor->distance >= 5 && sensor->distance <= 25) {
            // Distance valide : Allumer la LED Orange, éteindre la LED Rouge
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);   // LED ORANGE ON
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET); // LED ROUGE OFF
        } else {
            // Distance hors plage : Allumer la LED Rouge, éteindre la LED Orange
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET); // LED ORANGE OFF
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);   // LED ROUGE ON
        }
    }
}