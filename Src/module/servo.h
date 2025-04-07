/**
 * @file servo.h
 * @brief Interface pour le contrôle d'un servo-moteur
 * @details Ce module fournit les fonctions nécessaires pour contrôler un 
 * servo-moteur en utilisant un signal PWM généré par le timer TIM3
 * @author STM32 Team
 * @date 2025
 */

 #ifndef SERVO_H
 #define SERVO_H
 
 #include "stm32f4xx_hal.h"
 #include <stdint.h>
 
 /**
  * @def SERVO_MIN_POSITION
  * @brief Valeur PWM minimale pour le servo (correspond à 0°)
  * @details Durée d'impulsion de 1ms à une fréquence de 50Hz
  */
 #define SERVO_MIN_POSITION    1000  // Position 0° (1ms)
 
 /**
  * @def SERVO_MID_POSITION
  * @brief Valeur PWM pour la position centrale du servo (correspond à 90°)
  * @details Durée d'impulsion de 1.5ms à une fréquence de 50Hz
  */
 #define SERVO_MID_POSITION    1500  // Position 90° (1.5ms)
 
 /**
  * @def SERVO_MAX_POSITION
  * @brief Valeur PWM maximale pour le servo (correspond à 180°)
  * @details Durée d'impulsion de 2ms à une fréquence de 50Hz
  */
 #define SERVO_MAX_POSITION    2000  // Position 180° (2ms)
 
 /**
  * @def SERVO_MIN_DISTANCE
  * @brief Distance minimale (en cm) utilisée pour la conversion en position servo
  */
 #define SERVO_MIN_DISTANCE    5.0f   // Distance minimale en cm
 
 /**
  * @def SERVO_MAX_DISTANCE
  * @brief Distance maximale (en cm) utilisée pour la conversion en position servo
  */
 #define SERVO_MAX_DISTANCE    25.0f  // Distance maximale en cm
 
 /**
  * @def SERVO_MIN_VALUE
  * @brief Valeur utilisateur minimale pour le positionnement du servo en mode 2
  */
 #define SERVO_MIN_VALUE       1      // Valeur minimale en mode 2
 
 /**
  * @def SERVO_MAX_VALUE
  * @brief Valeur utilisateur maximale pour le positionnement du servo en mode 2
  */
 #define SERVO_MAX_VALUE       12     // Valeur maximale en mode 2
 
 /**
  * @brief Initialise le servo-moteur
  * @details Démarre la génération du signal PWM sur TIM3 Channel 1
  */
 void Servo_Init(void);
 
 /**
  * @brief Positionne le servo à une position spécifique
  * @param position Valeur de comparaison PWM (entre SERVO_MIN_POSITION et SERVO_MAX_POSITION)
  * @details La valeur est automatiquement limitée aux bornes valides
  */
 void Servo_SetPosition(uint32_t position);
 
 /**
  * @brief Positionne le servo au milieu de sa plage de mouvement (90°)
  */
 void Servo_SetToMiddle(void);
 
 /**
  * @brief Convertit une distance en position du servo
  * @param distance Distance mesurée en cm (entre SERVO_MIN_DISTANCE et SERVO_MAX_DISTANCE)
  * @return Valeur de comparaison PWM correspondante pour le servo
  * @details La distance est automatiquement limitée aux bornes valides
  */
 uint32_t Servo_DistanceToPosition(float distance);
 
 /**
  * @brief Convertit une valeur utilisateur en position du servo
  * @param value Valeur utilisateur (entre SERVO_MIN_VALUE et SERVO_MAX_VALUE)
  * @return Valeur de comparaison PWM correspondante pour le servo
  * @details La valeur est automatiquement limitée aux bornes valides
  */
 uint32_t Servo_ValueToPosition(uint8_t value);

/**
* @brief Fonction de démonstration pour le servo-moteur
* @details Cette fonction effectue une série de mouvements pour tester le servo
*/
void Servo_Demo(void);

 #endif /* SERVO_H */
 