/**
 * @file common.h
 * @brief Définitions et déclarations communes pour le projet
 * @details Contient les constantes et variables globales partagées entre les modules
 */

 #ifndef COMMON_H
 #define COMMON_H
 
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <stddef.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <signal.h>
 #include <pthread.h>
 #include <termios.h>
 #include <sys/time.h>
 
 // Configuration des broches
 #define SERVO_PIN 18        // GPIO 18 (broche physique 12)
 #define UART_DEVICE "/dev/serial0"  // Port série UART
 
 // Configuration pour le servo-moteur (équivalent aux valeurs de la STM32)
 #define SERVO_MIN_POSITION    1000  // Position 0° (1ms)
 #define SERVO_MID_POSITION    1500  // Position 90° (1.5ms)
 #define SERVO_MAX_POSITION    2000  // Position 180° (2ms)
 #define SERVO_MIN_DISTANCE    5.0   // Distance minimum en cm
 #define SERVO_MAX_DISTANCE    25.0  // Distance maximum en cm
 #define SERVO_MIN_VALUE       1     // Valeur utilisateur minimum
 #define SERVO_MAX_VALUE       12    // Valeur utilisateur maximum
 #define PWM_PERIOD            20000 // Période PWM en microsecondes (50Hz)
 
 // Variables globales partagées
 extern int currentMode;          // 0: Arrêt, 1: Mode capteur ultrason, 2: Mode manuel
 extern float currentDistance;    // Distance actuelle mesurée par le capteur
 extern int manualValue;          // Valeur manuelle par défaut (milieu)
 extern int serialFd;             // Descripteur de fichier pour la communication série
 extern int stopThreads;          // Indicateur pour arrêter les threads
 extern pthread_mutex_t mutex;    // Mutex pour protéger les variables partagées
 extern int pwmRunning;           // Indicateur pour le thread PWM
 extern int currentPosition;      // Position actuelle du servo en microsecondes
 extern char lastDistanceMessage[64]; // Message de la dernière distance reçue
 
 #endif /* COMMON_H */