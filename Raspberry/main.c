/**
 * @file main.c
 * @brief Programme principal pour le contrôle du servo-moteur via la Raspberry Pi
 * @details Ce programme gère la communication avec la STM32 et le contrôle du servo-moteur
 * en fonction du mode sélectionné
 * @author Développeur du Projet
 * @date Mai 2025
 * @version 1.0
 */

#include "common.h"
#include "gpio.h"
#include "servo.h"
#include "serial.h"

/**
 * @brief Mode de fonctionnement actuel du système
 * @details 0 = Arrêt, 1 = Les deux servos suivent le capteur, 2 = Mode mixte
 */
int currentMode = 0;

/**
 * @brief Distance actuelle mesurée par le capteur ultrason (en cm)
 */
float currentDistance = 0.0;

/**
 * @brief Valeur manuelle pour le contrôle du servo-moteur (1-12)
 * @details Valeur par défaut : position centrale (6)
 */
int manualValue = 6;

/**
 * @brief Descripteur de fichier pour la connexion série
 */
int serialFd = -1;

/**
 * @brief Drapeau pour signaler l'arrêt des threads
 * @details 0 = Continuer l'exécution, 1 = Arrêter les threads
 */
int stopThreads = 0;

/**
 * @brief Mutex pour la synchronisation des accès aux variables partagées
 */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Indique si la génération du signal PWM est active
 * @details 0 = Inactif, 1 = Actif
 */
int pwmRunning = 1;

/**
 * @brief Position actuelle du servo-moteur
 * @details Valeur par défaut : position centrale
 */
int currentPosition = SERVO_MID_POSITION;

/**
 * @brief Dernier message de distance reçu
 */
char lastDistanceMessage[64] = "Aucune donnée";

// Prototypes de fonctions
void handleUserInput(void);
void cleanup(int signum);
void printHelp(void);

/**
 * @brief Fonction principale
 * @return int Code de sortie (0 = succès, 1 = erreur)
 * @details Initialise le système, crée les threads nécessaires et gère l'interface utilisateur
 */
int main(void) {
    // Gestion du signal d'interruption (Ctrl+C)
    signal(SIGINT, cleanup);
    
    printf("=== Programme de contrôle du servo-moteur sur Raspberry Pi ===\n");
    printf("Communication avec STM32 F407VGT via UART\n\n");
    
    // Initialisation du GPIO
    if (setupGPIO() != 0) {
        fprintf(stderr, "Erreur lors de l'initialisation du GPIO\n");
        return 1;
    }
    
    // Ouvrir la connexion série
    serialFd = openSerialConnection();
    if (serialFd < 0) {
        fprintf(stderr, "Erreur lors de l'ouverture du port série\n");
        return 1;
    }
    
    // Création des threads
    pthread_t pwmThreadId, serialThreadId, servoThreadId;
    
    /**
     * @brief Thread pour générer le signal PWM
     * @see pwmThread dans servo.c
     */
    if (pthread_create(&pwmThreadId, NULL, pwmThread, NULL) != 0) {
        fprintf(stderr, "Erreur lors de la création du thread PWM\n");
        return 1;
    }
    
    /**
     * @brief Thread pour recevoir les données de la STM32
     * @see receiveDataThread dans serial.c
     */
    if (pthread_create(&serialThreadId, NULL, receiveDataThread, NULL) != 0) {
        fprintf(stderr, "Erreur lors de la création du thread de réception série\n");
        return 1;
    }
    
    /**
     * @brief Thread pour contrôler le servo-moteur
     * @see servoControlThread dans servo.c
     */
    if (pthread_create(&servoThreadId, NULL, servoControlThread, NULL) != 0) {
        fprintf(stderr, "Erreur lors de la création du thread de contrôle servo\n");
        return 1;
    }
    
    // Attendre un peu pour s'assurer que tout est initialisé
    usleep(500000); // 500ms
    
    // Afficher l'aide
    printHelp();
    
    // Envoyer un message initial à la STM32
    sendCommand(serialFd, "quit"); // Commencer en mode IDLE
    
    // Interface utilisateur en mode texte
    handleUserInput();
    
    // Attendre la fin des threads
    stopThreads = 1;
    pwmRunning = 0;
    pthread_join(pwmThreadId, NULL);
    pthread_join(serialThreadId, NULL);
    pthread_join(servoThreadId, NULL);
    
    // Nettoyage final
    cleanup(0);
    
    return 0;
}

/**
 * @brief Gère l'interface utilisateur en mode texte
 * @details Cette fonction lit les commandes de l'utilisateur et les envoie à la STM32
 * Elle implémente différentes commandes pour contrôler les modes et obtenir des informations
 */
void handleUserInput(void) {
    char command[32];
    while (!stopThreads) {
        printf("\nEntrez une commande: ");
        fflush(stdout);
        
        if (fgets(command, sizeof(command), stdin) == NULL) {
            // En cas d'erreur de lecture ou EOF
            break;
        }
        
        // Supprimer le retour à la ligne
        size_t len = strlen(command);
        if (len > 0 && command[len-1] == '\n') {
            command[len-1] = '\0';
        }
        
        // Traiter la commande
        if (strcmp(command, "mode1") == 0) {
            pthread_mutex_lock(&mutex);
            currentMode = 1;
            pthread_mutex_unlock(&mutex);
            
            sendCommand(serialFd, "mode1");
            printf("Mode 1 activé: Les deux servos suivent le capteur ultrason\n");
        } else if (strcmp(command, "mode2") == 0) {
            pthread_mutex_lock(&mutex);
            currentMode = 2;
            pthread_mutex_unlock(&mutex);
            
            sendCommand(serialFd, "mode2");
            printf("Mode 2 activé: Le servo STM32 suit les commandes manuelles, le servo Raspberry suit le capteur\n");
        } else if (strcmp(command, "getdistance") == 0) {
            // Envoyer une requête pour obtenir la dernière distance
            sendCommand(serialFd, "getdistance");
            printf("Demande de la dernière distance mesurée...\n");
            
            // Afficher la dernière distance reçue
            pthread_mutex_lock(&mutex);
            printf("Dernière distance reçue: %s\n", lastDistanceMessage);
            pthread_mutex_unlock(&mutex);
        } else if (strcmp(command, "help") == 0 || strcmp(command, "?") == 0) {
            printHelp();
        } else if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0) {
            sendCommand(serialFd, "quit");
            
            pthread_mutex_lock(&mutex);
            currentMode = 0;
            pthread_mutex_unlock(&mutex);
            
            printf("Arrêt du programme...\n");
            stopThreads = 1;
            break;
        } else if (atoi(command) >= 1 && atoi(command) <= 12) {
            pthread_mutex_lock(&mutex);
            int mode = currentMode;
            pthread_mutex_unlock(&mutex);
            
            if (mode == 2) {
                int value = atoi(command);
                
                pthread_mutex_lock(&mutex);
                manualValue = value;
                pthread_mutex_unlock(&mutex);
                
                sendCommand(serialFd, command);
                printf("Position du servo STM32 définie à %d\n", value);
            } else {
                printf("Les commandes de valeur (1-12) ne fonctionnent qu'en mode 2\n");
            }
        } else if (strcmp(command, "demo") == 0) {
            sendCommand(serialFd, "demo");
            printf("Mode de démonstration activé\n");
        } else {
            printf("Commande non reconnue. Tapez 'help' pour afficher l'aide.\n");
        }
    }
}

/**
 * @brief Affiche l'aide pour les commandes disponibles
 * @details Liste toutes les commandes prises en charge par le programme avec leur description
 */
void printHelp(void) {
    printf("\n=== Commandes disponibles ===\n");
    printf("  mode1       - Mode 1: Les deux servos suivent le capteur ultrason\n");
    printf("  mode2       - Mode 2: Le servo STM32 suit les commandes manuelles,\n");
    printf("                le servo Raspberry suit le capteur\n");
    printf("  1-12        - En mode 2, définit la position du servo STM32\n");
    printf("  getdistance - Affiche la dernière distance mesurée\n");
    printf("  demo        - Lance le mode de démonstration sur la STM32\n");
    printf("  help, ?     - Affiche cette aide\n");
    printf("  quit, exit  - Quitte le programme\n");
    printf("========================\n");
}

/**
 * @brief Nettoyage et libération des ressources
 * @param signum Numéro du signal (pour la gestion de SIGINT)
 * @details Arrête les threads, met la broche GPIO à l'état bas et ferme la connexion série
 */
void cleanup(int signum) {
    printf("\nNettoyage et arrêt du programme...\n");
    
    // Arrêter les threads
    stopThreads = 1;
    pwmRunning = 0;
    usleep(500000); // 500ms pour laisser le temps aux threads de s'arrêter
    
    // Mettre la broche GPIO à l'état bas
    writeGPIO(SERVO_PIN, 0);
    
    // Fermer la connexion série
    if (serialFd >= 0) {
        close(serialFd);
    }
    
    printf("Nettoyage terminé\n");
    
    if (signum != 0) {
        exit(signum);
    }
}