/**
 * @file hcsr04.h
 * @brief Interface pour le capteur ultrasonique HC-SR04
 * @details Ce module fournit les fonctions nécessaires pour interfacer avec 
 * un capteur de distance à ultrasons HC-SR04. Il permet de mesurer des 
 * distances entre 5 et 25 cm.
 * @author STM32 Team
 * @date 2025
 */

 #ifndef HC_SR04_H
 #define HC_SR04_H
 
 #include "stm32f4xx_hal.h"
 
 /**
  * @def HC_SR04_TRIGGER_PIN
  * @brief Broche utilisée pour le signal Trigger du capteur HC-SR04 (PA8)
  */
 #define HC_SR04_TRIGGER_PIN    GPIO_PIN_8   // PA8
 
 /**
  * @def HC_SR04_ECHO_PIN
  * @brief Broche utilisée pour le signal Echo du capteur HC-SR04 (PA9)
  */
 #define HC_SR04_ECHO_PIN       GPIO_PIN_9   // PA9
 
 /**
  * @def HC_SR04_GPIO_PORT
  * @brief Port GPIO pour les signaux du capteur HC-SR04
  */
 #define HC_SR04_GPIO_PORT      GPIOA
 
 /**
  * @struct HC_SR04
  * @brief Structure de données pour le capteur HC-SR04
  * @details Cette structure contient les configurations et état du capteur HC-SR04
  */
 typedef struct {
     GPIO_TypeDef* trigger_port;   /**< Port GPIO pour le signal Trigger */
     uint16_t trigger_pin;         /**< Broche pour le signal Trigger */
     GPIO_TypeDef* echo_port;      /**< Port GPIO pour le signal Echo */
     uint16_t echo_pin;            /**< Broche pour le signal Echo */
     TIM_HandleTypeDef* timer;     /**< Timer utilisé pour les mesures */
     float distance;               /**< Distance mesurée en cm */
 } HC_SR04;
 
 /**
  * @brief Récupère l'instance du capteur HC-SR04 (Singleton)
  * @return Pointeur vers l'instance unique du capteur HC-SR04
  */
 HC_SR04* HC_SR04_get_instance(void);
 
 /**
  * @brief Initialise le capteur HC-SR04
  * @details Configure les broches GPIO et le timer nécessaires pour le capteur
  */
 void HC_SR04_init(void);
 
 /**
  * @brief Mesure la distance avec le capteur HC-SR04
  * @param sensor Pointeur vers l'instance du capteur HC-SR04
  * @return Distance mesurée en centimètres, ou -1.0f en cas d'erreur
  * @note La plage valide de mesure est de 5 à 25 cm.
  */
 float HC_SR04_get_distance(HC_SR04* sensor);
 
 /**
  * @brief Met à jour périodiquement la distance mesurée par le capteur
  * @param sensor Pointeur vers l'instance du capteur HC-SR04
  * @param mode Mode d'opération (1 pour activer les mises à jour périodiques)
  * @details Cette fonction effectue une mesure toutes les 250ms lorsque mode=1, 
  * met à jour les LEDs d'état et envoie la distance via UART
  */
 void HC_SR04_update(HC_SR04* sensor, int mode);

/**
* @brief Fonction de démonstration pour le capteur HC-SR04
* @details Affiche la distance mesurée sur le terminal série
*/
void HC_SR04_Demo(void);
 
 #endif /* HC_SR04_H */
 