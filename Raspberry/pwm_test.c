/**
 * @file servo_test.c
 * @brief Programme de test pour vérifier le fonctionnement PWM du servo-moteur sur Raspberry Pi
 * @details Ce programme génère un signal PWM précis pour contrôler un servo-moteur
 * et effectue un balayage automatique de la position minimale à la position maximale
 * @author Développeur du Projet
 * @date Mai 2025
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>

/**
 * @defgroup gpio_config Configuration GPIO
 * @{
 */
/** @brief Numéro de la broche GPIO utilisée pour le servo-moteur (GPIO 18, broche physique 12) */
#define SERVO_PIN  18
/** @} */

/**
 * @defgroup servo_config Configuration du servo-moteur
 * @{
 */
/** @brief Position minimale du servo-moteur (1ms = 0°) */
#define SERVO_MIN_POSITION  1000
/** @brief Position centrale du servo-moteur (1.5ms = 90°) */
#define SERVO_MID_POSITION  1500
/** @brief Position maximale du servo-moteur (2ms = 180°) */
#define SERVO_MAX_POSITION  2000
/** @brief Période du signal PWM en microsecondes (50Hz = 20ms) */
#define PWM_PERIOD         20000
/** @} */

/**
 * @defgroup global_vars Variables globales
 * @{
 */
/** @brief Drapeau indiquant si le programme est en cours d'exécution (1 = exécution, 0 = arrêt) */
volatile int running = 1;
/** @brief Position actuelle du servo-moteur en microsecondes */
volatile int currentPosition = SERVO_MID_POSITION;
/** @brief Mutex pour protéger l'accès à la variable currentPosition */
pthread_mutex_t positionMutex = PTHREAD_MUTEX_INITIALIZER;
/** @} */

// Prototypes de fonctions
void setupGPIO(void);
void writeGPIO(int value);
void cleanup(int signum);
void* pwmThread(void* arg);
void* sweepThread(void* arg);

/**
 * @brief Fonction principale
 * @return int Code de sortie (0 = succès, 1 = erreur)
 * @details Initialise le système, crée les threads de génération PWM et de balayage,
 * puis attend la fin de l'exécution
 */
int main(void) {
    printf("=== Programme de test PWM pour servo-moteur sur Raspberry Pi ===\n");
    printf("Appuyez sur Ctrl+C pour quitter\n\n");
    
    // Gestion du signal d'interruption (Ctrl+C)
    signal(SIGINT, cleanup);
    
    // Initialisation du GPIO
    setupGPIO();
    
    // Création des threads
    pthread_t pwmThreadId, sweepThreadId;
    
    /**
     * @brief Création du thread de génération PWM
     * @see pwmThread
     */
    if (pthread_create(&pwmThreadId, NULL, pwmThread, NULL) != 0) {
        fprintf(stderr, "Erreur lors de la création du thread PWM\n");
        return 1;
    }
    
    /**
     * @brief Création du thread de balayage du servo
     * @see sweepThread
     */
    if (pthread_create(&sweepThreadId, NULL, sweepThread, NULL) != 0) {
        fprintf(stderr, "Erreur lors de la création du thread de balayage\n");
        running = 0;
        pthread_join(pwmThreadId, NULL);
        return 1;
    }
    
    // Attendre la fin des threads
    pthread_join(sweepThreadId, NULL);
    pthread_join(pwmThreadId, NULL);
    
    return 0;
}

/**
 * @brief Initialise le GPIO pour le servo-moteur
 * @details Configure la broche GPIO en tant que sortie en utilisant l'interface sysfs
 * @see SERVO_PIN
 */
void setupGPIO(void) {
    // Exporter le GPIO
    int fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0) {
        perror("Erreur lors de l'ouverture du fichier d'export GPIO");
        exit(1);
    }
    
    char buffer[3];
    snprintf(buffer, sizeof(buffer), "%d", SERVO_PIN);
    write(fd, buffer, strlen(buffer));
    close(fd);
    
    // Donner au système de fichiers le temps de créer le fichier
    usleep(100000); // 100ms
    
    // Configurer la direction du GPIO (sortie)
    char path[50];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", SERVO_PIN);
    fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror("Erreur lors de l'ouverture du fichier de direction GPIO");
        exit(1);
    }
    
    write(fd, "out", 3);
    close(fd);
    
    printf("GPIO initialisé avec succès\n");
}

/**
 * @brief Écrit une valeur sur le GPIO du servo
 * @param value Valeur à écrire (0 = état bas, 1 = état haut)
 * @details Utilise l'interface sysfs pour modifier l'état de la broche GPIO
 * @see SERVO_PIN
 */
void writeGPIO(int value) {
    char path[50];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", SERVO_PIN);
    
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror("Erreur lors de l'ouverture du fichier de valeur GPIO");
        return;
    }
    
    char buffer[2];
    snprintf(buffer, sizeof(buffer), "%d", value);
    
    write(fd, buffer, 1);
    close(fd);
}

/**
 * @brief Thread qui génère le signal PWM pour le servo
 * @param arg Argument du thread (non utilisé)
 * @return void* Valeur de retour du thread (NULL)
 * @details Génère un signal PWM précis en contrôlant la durée des impulsions pour
 * positionner le servo-moteur à l'angle souhaité
 * @see currentPosition
 * @see positionMutex
 * @see PWM_PERIOD
 */
void* pwmThread(void* arg) {
    struct timeval tv;
    long long startTime, currentTime, elapsedTime;
    int position;
    
    printf("Thread PWM démarré...\n");
    
    while (running) {
        // Obtenir la position actuelle du servo
        pthread_mutex_lock(&positionMutex);
        position = currentPosition;
        pthread_mutex_unlock(&positionMutex);
        
        // Obtenir le temps de départ
        gettimeofday(&tv, NULL);
        startTime = tv.tv_sec * 1000000LL + tv.tv_usec;
        
        // Mettre la broche à HIGH pour la durée de l'impulsion
        writeGPIO(1);
        
        // Attendre la durée de l'impulsion
        usleep(position);
        
        // Mettre la broche à LOW pour le reste de la période
        writeGPIO(0);
        
        // Calcul du temps à attendre pour compléter la période (20ms)
        gettimeofday(&tv, NULL);
        currentTime = tv.tv_sec * 1000000LL + tv.tv_usec;
        elapsedTime = currentTime - startTime;
        
        if (elapsedTime < PWM_PERIOD) {
            usleep(PWM_PERIOD - elapsedTime);
        }
    }
    
    printf("Thread PWM terminé\n");
    return NULL;
}

/**
 * @brief Thread qui fait balayer le servo de gauche à droite
 * @param arg Argument du thread (non utilisé)
 * @return void* Valeur de retour du thread (NULL)
 * @details Fait osciller le servo-moteur entre sa position minimale et maximale
 * en modifiant progressivement la valeur de currentPosition
 * @see SERVO_MIN_POSITION
 * @see SERVO_MAX_POSITION
 * @see currentPosition
 * @see positionMutex
 */
void* sweepThread(void* arg) {
    int position = SERVO_MIN_POSITION;
    int direction = 1;  // 1 = droite, -1 = gauche
    
    printf("Thread de balayage démarré...\n");
    printf("Le servo va balayer de gauche à droite continuellement\n");
    
    while (running) {
        // Mettre à jour la position
        pthread_mutex_lock(&positionMutex);
        currentPosition = position;
        pthread_mutex_unlock(&positionMutex);
        
        // Afficher la position actuelle (environ toutes les 100ms)
        if (position % 100 == 0) {
            printf("Position servo: %d µs\n", position);
        }
        
        // Incrémenter/décrémenter la position
        position += direction * 10;  // Incréments de 10µs
        
        // Inverser la direction aux extrémités
        if (position >= SERVO_MAX_POSITION) {
            position = SERVO_MAX_POSITION;
            direction = -1;
            printf("Limite droite atteinte, retour à gauche\n");
        } else if (position <= SERVO_MIN_POSITION) {
            position = SERVO_MIN_POSITION;
            direction = 1;
            printf("Limite gauche atteinte, déplacement à droite\n");
        }
        
        // Petit délai pour un balayage visible
        usleep(20000);  // 20ms
    }
    
    printf("Thread de balayage terminé\n");
    return NULL;
}

/**
 * @brief Nettoyage des ressources avant de quitter
 * @param signum Numéro du signal (pour la gestion de SIGINT)
 * @details Arrête les threads, met la broche GPIO à l'état bas et libère la broche
 * @see running
 * @see SERVO_PIN
 */
void cleanup(int signum) {
    printf("\nNettoyage et arrêt du programme...\n");
    
    // Arrêter les threads
    running = 0;
    usleep(500000);  // 500ms pour laisser le temps aux threads de s'arrêter
    
    // Mettre le GPIO à l'état bas
    writeGPIO(0);
    
    // Libérer le GPIO
    int fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd >= 0) {
        char buffer[3];
        snprintf(buffer, sizeof(buffer), "%d", SERVO_PIN);
        write(fd, buffer, strlen(buffer));
        close(fd);
    }
    
    printf("Nettoyage terminé\n");
    
    if (signum != 0) {
        exit(signum);
    }
}