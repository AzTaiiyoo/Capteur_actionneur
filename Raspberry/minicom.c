/**
 * @file uart_test.c
 * @brief Programme de test de communication UART avec la STM32
 * @details Ce programme ouvre une connexion série avec la STM32, 
 * envoie une commande <mode1> et affiche toutes les réponses reçues
 * @author Développeur du Projet
 * @date Mai 2025
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>

/**
 * @brief Chemin du périphérique UART sur la Raspberry Pi
 * @details Ce chemin correspond au port série matériel de la Raspberry Pi
 */
#define UART_DEVICE "/dev/serial0"

/**
 * @brief Fonction principale du programme
 * @return int Code de sortie (0 = succès, 1 = erreur)
 * @details Ouvre une connexion série avec la STM32, envoie une commande 
 * et affiche les réponses pendant une période de 5 secondes
 */
int main() {
    /**
     * @brief Ouverture du port série
     * @details Ouverture en mode lecture/écriture sans contrôle de terminal
     */
    int fd = open(UART_DEVICE, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror("Erreur d'ouverture du port série");
        return 1;
    }
    
    /**
     * @brief Structure de configuration du port série
     */
    struct termios config;
    if (tcgetattr(fd, &config) != 0) {
        perror("Erreur de lecture des attributs");
        close(fd);
        return 1;
    }
    
    /**
     * @brief Configuration du port série
     * @details Configuration standard 115200 bauds, 8 bits de données, 
     * pas de parité, 1 bit de stop (8N1)
     */
    cfsetispeed(&config, B115200);    // Vitesse en entrée: 115200 bauds
    cfsetospeed(&config, B115200);    // Vitesse en sortie: 115200 bauds
    config.c_cflag &= ~PARENB;        // Pas de parité
    config.c_cflag &= ~CSTOPB;        // 1 bit de stop
    config.c_cflag &= ~CSIZE;         // Masque de taille de caractère
    config.c_cflag |= CS8;            // 8 bits de données
    config.c_cflag |= CREAD | CLOCAL; // Activer la réception, ignorer les signaux modem
    config.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Mode raw (non canonique)
    config.c_oflag &= ~OPOST;         // Mode raw en sortie
    
    /**
     * @brief Application des paramètres de configuration
     */
    if (tcsetattr(fd, TCSANOW, &config) != 0) {
        perror("Erreur d'application des attributs");
        close(fd);
        return 1;
    }
    
    /**
     * @brief Vider les tampons d'entrée et de sortie
     */
    tcflush(fd, TCIOFLUSH);
    
    printf("Envoi de <mode1> puis affichage de toutes les réponses...\n");
    
    /**
     * @brief Envoi de la commande <mode1> au périphérique
     * @details La commande est terminée par CR+LF
     */
    const char *cmd = "<mode1>\r\n";
    write(fd, cmd, strlen(cmd));
    tcdrain(fd);  // Attendre que toutes les données soient envoyées
    
    printf("Réponses:\n");
    
    /**
     * @brief Configuration du port en mode non-bloquant pour la lecture
     * @details Permet de lire sans attendre qu'un octet soit disponible
     */
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    
    /**
     * @brief Définition de la durée d'écoute (5 secondes)
     */
    time_t end_time = time(NULL) + 5;
    
    /**
     * @brief Variables pour la gestion des messages
     * @details Un message complet est encadré par les caractères < et >
     */
    char buffer[256] = {0};   // Tampon pour stocker les messages complets
    int bufferIndex = 0;      // Index de position dans le tampon
    int inMessage = 0;        // Indicateur de construction de message en cours
    
    /**
     * @brief Boucle principale d'écoute des réponses
     * @details Écoute pendant 5 secondes et affiche les caractères reçus
     * Détecte également les messages complets encadrés par < et >
     */
    while (time(NULL) < end_time) {
        char c;
        if (read(fd, &c, 1) > 0) {
            // Afficher chaque caractère (ou un point pour les caractères non imprimables)
            printf("%c", (c >= 32 && c <= 126) ? c : '.');
            
            // Construire des messages complets entre < et >
            if (c == '<') {
                // Début d'un nouveau message
                inMessage = 1;
                bufferIndex = 0;
                buffer[0] = c;
                bufferIndex++;
            } else if (c == '>' && inMessage) {
                // Fin d'un message complet
                buffer[bufferIndex] = c;
                buffer[bufferIndex + 1] = '\0';
                printf("\nMessage complet: %s\n", buffer);
                inMessage = 0;
            } else if (inMessage && bufferIndex < sizeof(buffer) - 2) {
                // Ajout du caractère au message en cours
                buffer[bufferIndex] = c;
                bufferIndex++;
            }
            
            fflush(stdout);  // Forcer l'affichage immédiat
        }
        usleep(5000);  // Pause de 5ms pour réduire l'utilisation CPU
    }
    
    printf("\nDiagnostic terminé\n");
    
    /**
     * @brief Restauration du mode de fonctionnement initial et fermeture
     */
    fcntl(fd, F_SETFL, flags);  // Restaurer le mode d'origine
    close(fd);                  // Fermer le descripteur de fichier
    
    return 0;
}