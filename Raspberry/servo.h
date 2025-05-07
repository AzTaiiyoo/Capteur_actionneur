/**
 * @file servo.h
 * @brief Interface pour le contrôle du servo-moteur
 * @details Ce module fournit les fonctions nécessaires pour contrôler un 
 * servo-moteur en utilisant un signal PWM généré par le GPIO
 */

 #ifndef SERVO_H
 #define SERVO_H
 
 #include "common.h"
 
 /**
  * @brief Convertit une distance en position du servo
  * @param distance Distance mesurée en cm (entre SERVO_MIN_DISTANCE et SERVO_MAX_DISTANCE)
  * @return Valeur de comparaison PWM correspondante pour le servo
  * @details La distance est automatiquement limitée aux bornes valides
  */
 int distanceToPosition(float distance);
 
 /**
  * @brief Convertit une valeur utilisateur en position du servo
  * @param value Valeur utilisateur (entre SERVO_MIN_VALUE et SERVO_MAX_VALUE)
  * @return Valeur de comparaison PWM correspondante pour le servo
  * @details La valeur est automatiquement limitée aux bornes valides
  */
 int valueToPosition(int value);
 
 /**
  * @brief Thread qui génère le signal PWM pour le servo
  * @param arg Argument du thread (non utilisé)
  * @return NULL
  */
 void* pwmThread(void* arg);
 
 /**
  * @brief Thread pour contrôler le servo-moteur
  * @param arg Argument du thread (non utilisé)
  * @return NULL
  */
 void* servoControlThread(void* arg);
 
 #endif /* SERVO_H */