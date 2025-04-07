
/**
 * @file hcsr04.c
 * @brief Implémentation des fonctions pour le capteur ultrasonique HC-SR04
 */

 #include "hcsr04.h"
 #include "gpio.h"
 #include "tim.h"
 #include <stdio.h>
 #include <stdint.h>
 #include "tim.h"
 #include "usart_comm.h"
 
 /**
  * @brief Instance statique du capteur HC-SR04 (singleton)
  */
 static HC_SR04 sensor_instance = {0};
 
 /**
  * @brief Récupère l'instance du capteur HC-SR04 (Singleton)
  * @return Pointeur vers l'instance unique du capteur HC-SR04
  */
 HC_SR04* HC_SR04_get_instance(void) {
     return &sensor_instance;
 }
 
 /**
  * @brief Génère un délai précis en microsecondes
  * @param us Nombre de microsecondes de délai
  * @details Utilise le timer TIM1 pour générer un délai précis
  */
 void Delay_Microseconds(uint32_t us)
 {   
     __HAL_TIM_SET_COUNTER(&htim1, 0);  // Réinitialiser le compteur
     while(__HAL_TIM_GET_COUNTER(&htim1) < us); // Attendre le nombre de µs demandé
 }
 
 /**
  * @brief Initialise le capteur HC-SR04
  * @details Configure les broches GPIO et le timer nécessaires pour le capteur
  */
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
 
 /**
  * @brief Mesure la distance avec le capteur HC-SR04
  * @param sensor Pointeur vers l'instance du capteur HC-SR04
  * @return Distance mesurée en centimètres, ou -1.0f en cas d'erreur
  * @note La plage valide de mesure est de 5 à 25 cm.
  */
 float HC_SR04_get_distance(HC_SR04* sensor) {
     uint32_t start_time, stop_time;
 
     // Envoi d'une impulsion Trigger de 10 µs
     HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
     Delay_Microseconds(10); // Délai précis en microsecondes
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
 
 /**
  * @brief Met à jour périodiquement la distance mesurée par le capteur
  * @param sensor Pointeur vers l'instance du capteur HC-SR04
  * @param mode Mode d'opération (1 pour activer les mises à jour périodiques)
  * @details Cette fonction effectue une mesure toutes les 250ms lorsque mode=1, 
  * met à jour les LEDs d'état et envoie la distance via UART
  */
 void HC_SR04_update(HC_SR04* sensor, int mode) {
     if (mode == 1) {
         static uint32_t last_update = 0;
         uint32_t current_time = HAL_GetTick();
         
         if (current_time - last_update >= 250) {
             // Force la mise à jour de last_update
             last_update = current_time;
             
             // Mettre à jour la distance mesurée
             sensor->distance = HC_SR04_get_distance(sensor);
             
             // Envoyer la distance via UART
             char message[32];
             if (sensor->distance >= 5 && sensor->distance <= 25) {
                 sprintf(message, "Distance: %.2f cm", sensor->distance);
                 sendMessage(message);
                 
                 // Indiquer l'état avec les LEDs
                 HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);   // LED ORANGE ON
                 HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET); // LED ROUGE OFF
             } else {
                 // Ajoutez un compteur pour voir si cette section est exécutée plusieurs fois
                 static uint32_t error_count = 0;
                 sprintf(message, "Distance hors plage (%lu)", ++error_count);
                 sendMessage(message);
                 
                 // LEDs pour indiquer erreur
                 HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET); // LED ORANGE OFF
                 HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);   // LED ROUGE ON
             }
         }
     }
 }

  /**
 * @brief Fonction de démonstration simple pour tester le capteur HC-SR04
 * @details Cette fonction mesure la distance et utilise les LEDs pour indiquer
 *          si la distance est valide ou hors plage.
 */
void HC_SR04_Demo(void) {
    HC_SR04* sensor = HC_SR04_get_instance(); // Obtenir l'instance du capteur
    HC_SR04_init(); // Initialiser le capteur

    while (1) {
        // Mesurer la distance
        float distance = HC_SR04_get_distance(sensor);

        // Vérifier si la distance est valide
        if (distance >= 5.0f && distance <= 25.0f) {
            // Distance valide : LED Orange ON, LED Rouge OFF
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);   
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET); 
        } else {
            // Distance hors plage ou erreur : LED Rouge ON, LED Orange OFF
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET); 
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET); 
        }
        HAL_Delay(250);
    }
}