
/**
 * @file servo.c
 * @brief Implémentation des fonctions de contrôle du servo-moteur
 */

 #include "servo.h"
 #include "tim.h"
 
 /**
  * @brief Initialise le servo-moteur
  * @details Démarre la génération du signal PWM sur TIM3 Channel 1
  */
 void Servo_Init(void) {
     // Démarrer la PWM sur TIM3 Channel 1
     HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
 }
 
 /**
  * @brief Positionne le servo à une position spécifique
  * @param position Valeur de comparaison PWM (entre SERVO_MIN_POSITION et SERVO_MAX_POSITION)
  * @details La valeur est automatiquement limitée aux bornes valides
  */
 void Servo_SetPosition(uint32_t position) {
     // Limiter la position aux bornes valides
     if (position < SERVO_MIN_POSITION) position = SERVO_MIN_POSITION;
     if (position > SERVO_MAX_POSITION) position = SERVO_MAX_POSITION;
     
     // Définir la valeur de comparaison pour la PWM
     __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, position);
 }
 
 /**
  * @brief Positionne le servo au milieu de sa plage de mouvement (90°)
  */
 void Servo_SetToMiddle(void) {
     Servo_SetPosition(SERVO_MID_POSITION);
 }
 
 /**
  * @brief Convertit une distance en position du servo
  * @param distance Distance mesurée en cm (entre SERVO_MIN_DISTANCE et SERVO_MAX_DISTANCE)
  * @return Valeur de comparaison PWM correspondante pour le servo
  * @details La distance est automatiquement limitée aux bornes valides
  */
 uint32_t Servo_DistanceToPosition(float distance) {
     // Limiter la distance aux bornes valides
     if (distance < SERVO_MIN_DISTANCE) distance = SERVO_MIN_DISTANCE;
     if (distance > SERVO_MAX_DISTANCE) distance = SERVO_MAX_DISTANCE;
     
     // Conversion linéaire: min_distance->min_position, max_distance->max_position
     return SERVO_MIN_POSITION + (uint32_t)((distance - SERVO_MIN_DISTANCE) * 
            (SERVO_MAX_POSITION - SERVO_MIN_POSITION) / 
            (SERVO_MAX_DISTANCE - SERVO_MIN_DISTANCE));
 }
 
 /**
  * @brief Convertit une valeur utilisateur en position du servo
  * @param value Valeur utilisateur (entre SERVO_MIN_VALUE et SERVO_MAX_VALUE)
  * @return Valeur de comparaison PWM correspondante pour le servo
  * @details La valeur est automatiquement limitée aux bornes valides
  */
 uint32_t Servo_ValueToPosition(uint8_t value) {
     // Limiter la valeur aux bornes valides
     if (value < SERVO_MIN_VALUE) value = SERVO_MIN_VALUE;
     if (value > SERVO_MAX_VALUE) value = SERVO_MAX_VALUE;
     
     // Conversion linéaire: min_value->min_position, max_value->max_position
     return SERVO_MIN_POSITION + (uint32_t)((value - SERVO_MIN_VALUE) * 
            (SERVO_MAX_POSITION - SERVO_MIN_POSITION) / 
            (SERVO_MAX_VALUE - SERVO_MIN_VALUE));
 }

  /**
 * @brief Fonction de démonstration pour tester le contrôle du servo-moteur
 * @details Cette fonction déplace le servo à différentes positions en fonction
 *          de distances simulées ou de valeurs utilisateur.
 */
void Servo_Demo(void) {
    Servo_Init();

    // Déplacer le servo à la position minimale
    Servo_SetPosition(SERVO_MIN_POSITION);
    HAL_Delay(1000); // Attendre 1 seconde

    // Déplacer le servo à la position centrale
    Servo_SetToMiddle();
    HAL_Delay(1000); // Attendre 1 seconde

    // Déplacer le servo à la position maximale
    Servo_SetPosition(SERVO_MAX_POSITION);
    HAL_Delay(1000); // Attendre 1 seconde

    // Revenir à la position centrale
    Servo_SetToMiddle();
}