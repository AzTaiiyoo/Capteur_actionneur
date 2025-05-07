/**
 * @file servo.c
 * @brief Implémentation des fonctions de contrôle du servo-moteur
 */

 #include "servo.h"
 #include "gpio.h"
 
 /**
  * @brief Convertit une distance en position du servo
  * @param distance Distance mesurée en cm (entre SERVO_MIN_DISTANCE et SERVO_MAX_DISTANCE)
  * @return Valeur de comparaison PWM correspondante pour le servo
  * @details La distance est automatiquement limitée aux bornes valides
  */
 int distanceToPosition(float distance) {
     // Limiter la distance aux bornes valides
     if (distance < SERVO_MIN_DISTANCE)
         distance = SERVO_MIN_DISTANCE;
     if (distance > SERVO_MAX_DISTANCE)
         distance = SERVO_MAX_DISTANCE;
     
     // Conversion linéaire: min_distance->min_position, max_distance->max_position
     int position = SERVO_MIN_POSITION + (int)((distance - SERVO_MIN_DISTANCE) * 
                   (SERVO_MAX_POSITION - SERVO_MIN_POSITION) / 
                   (SERVO_MAX_DISTANCE - SERVO_MIN_DISTANCE));
     
     return position;
 }
 
 /**
  * @brief Convertit une valeur utilisateur en position du servo
  * @param value Valeur utilisateur (entre SERVO_MIN_VALUE et SERVO_MAX_VALUE)
  * @return Valeur de comparaison PWM correspondante pour le servo
  * @details La valeur est automatiquement limitée aux bornes valides
  */
 int valueToPosition(int value) {
     // Limiter la valeur aux bornes valides
     if (value < SERVO_MIN_VALUE)
         value = SERVO_MIN_VALUE;
     if (value > SERVO_MAX_VALUE)
         value = SERVO_MAX_VALUE;
     
     // Conversion linéaire: min_value->min_position, max_value->max_position
     int position = SERVO_MIN_POSITION + ((value - SERVO_MIN_VALUE) * 
                   (SERVO_MAX_POSITION - SERVO_MIN_POSITION) / 
                   (SERVO_MAX_VALUE - SERVO_MIN_VALUE));
     
     return position;
 }
 
 /**
  * @brief Thread qui génère le signal PWM pour le servo
  * @param arg Argument du thread (non utilisé)
  * @return NULL
  */
 void* pwmThread(void* arg) {
     struct timeval tv;
     long long startTime, currentTime, elapsedTime;
     
     while (pwmRunning) {
         // Obtenir la position actuelle du servo
         pthread_mutex_lock(&mutex);
         int position = currentPosition;
         pthread_mutex_unlock(&mutex);
         
         // Obtenir le temps de départ
         gettimeofday(&tv, NULL);
         startTime = tv.tv_sec * 1000000LL + tv.tv_usec;
         
         // Mettre la broche à HIGH pour la durée de l'impulsion
         writeGPIO(SERVO_PIN, 1);
         
         // Attendre la durée de l'impulsion
         usleep(position);
         
         // Mettre la broche à LOW pour le reste de la période
         writeGPIO(SERVO_PIN, 0);
         
         // Calcul du temps à attendre pour compléter la période (20ms)
         gettimeofday(&tv, NULL);
         currentTime = tv.tv_sec * 1000000LL + tv.tv_usec;
         elapsedTime = currentTime - startTime;
         
         if (elapsedTime < PWM_PERIOD) {
             usleep(PWM_PERIOD - elapsedTime);
         }
     }
     
     return NULL;
 }
 
 /**
  * @brief Thread pour contrôler le servo-moteur
  * @param arg Argument du thread (non utilisé)
  * @return NULL
  */
 void* servoControlThread(void* arg) {
     int position = SERVO_MID_POSITION;
     
     while (!stopThreads) {
         pthread_mutex_lock(&mutex);
         int mode = currentMode;
         float distance = currentDistance;
         int value = manualValue;
         pthread_mutex_unlock(&mutex);
         
         if (mode == 1) {
             // Mode 1: Contrôle par capteur ultrason
             if (distance >= SERVO_MIN_DISTANCE && distance <= SERVO_MAX_DISTANCE) {
                 position = distanceToPosition(distance);
             } else {
                 // Si distance hors plage, mettre en position milieu
                 position = SERVO_MID_POSITION;
             }
         } else if (mode == 2) {
             // Mode 2: Le servo de la Raspberry suit toujours le capteur
             if (distance >= SERVO_MIN_DISTANCE && distance <= SERVO_MAX_DISTANCE) {
                 position = distanceToPosition(distance);
             } else {
                 // Si distance hors plage, mettre en position milieu
                 position = SERVO_MID_POSITION;
             }
         } else {
             // Mode 0: Arrêt, servo en position milieu
             position = SERVO_MID_POSITION;
         }
         
         // Mettre à jour la position du servo
         pthread_mutex_lock(&mutex);
         currentPosition = position;
         pthread_mutex_unlock(&mutex);
         
         usleep(50000); // 50ms
     }
     
     return NULL;
 }