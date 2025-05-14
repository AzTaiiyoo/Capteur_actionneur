/**
 * @file test_uart.c
 * @brief Programme simple de test de communication UART sur Raspberry Pi
 * @details Ce programme ouvre une connexion série, envoie un message de test
 * et affiche la réponse reçue avec un formatage lisible
 * @author Développeur du Projet
 * @date Mai 2025
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

/**
 * @brief Fonction principale du programme
 * @return int Code de sortie (0 = succès, 1 = erreur)
 * @details Ouvre le port série, envoie un message de test et attend une réponse
 */
int main() {
    /**
     * @brief Ouverture du port série
     * @details Ouvre le port série /dev/serial0 avec les options suivantes:
     *  - O_RDWR: ouverture en lecture/écriture
     *  - O_NOCTTY: ce processus ne devient pas le "contrôleur" du port
     */
    int fd = open("/dev/serial0", O_RDWR | O_NOCTTY);
    if (fd < 0) {
        fprintf(stderr, "Erreur ouverture port série: %s\n", strerror(errno));
        return 1;
    }
    
    /**
     * @brief Structure pour la configuration du port série
     */
    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    
    /**
     * @brief Lecture des paramètres actuels du port série
     */
    if (tcgetattr(fd, &tty) != 0) {
        fprintf(stderr, "Erreur tcgetattr: %s\n", strerror(errno));
        close(fd);
        return 1;
    }
    
    /**
     * @brief Configuration du port série en 115200 8N1
     * @details Paramètres de configuration:
     *  - B115200: vitesse à 115200 bauds
     *  - CS8: 8 bits de données
     *  - CLOCAL: ignore les signaux de contrôle de modem
     *  - CREAD: active la réception des caractères
     *  - VMIN = 0, VTIME = 10: timeout de 1 seconde
     */
    tty.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    tty.c_iflag = 0;    // Désactive tous les traitements en entrée
    tty.c_oflag = 0;    // Désactive tous les traitements en sortie
    tty.c_lflag = 0;    // Mode non-canonique
    tty.c_cc[VMIN] = 0; // Pas de nombre minimum de caractères
    tty.c_cc[VTIME] = 10; // Timeout de 1 seconde (10 décisecs)
    
    /**
     * @brief Application de la configuration au port série
     */
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        fprintf(stderr, "Erreur tcsetattr: %s\n", strerror(errno));
        close(fd);
        return 1;
    }
    
    /**
     * @brief Vidage des tampons d'entrée et de sortie
     */
    tcflush(fd, TCIOFLUSH);
    
    /**
     * @brief Envoi d'un message de test sur le port série
     */
    const char *msg = "<TEST>";
    printf("Envoi de: %s\n", msg);
    write(fd, msg, strlen(msg));
    tcdrain(fd);  // Attendre que toutes les données soient transmises
    
    /**
     * @brief Attente d'une réponse
     */
    printf("Attente de réponse...\n");
    usleep(500000); // Pause de 500ms
    
    /**
     * @brief Lecture et traitement de la réponse
     * @details Lit les données reçues et les affiche de manière lisible:
     *  - Les caractères imprimables sont affichés normalement
     *  - Les caractères non-imprimables sont affichés en hexadécimal
     */
    char buf[256];
    memset(buf, 0, sizeof(buf));
    
    ssize_t bytesRead = read(fd, buf, sizeof(buf) - 1);
    if (bytesRead > 0) {
        printf("Reçu (%zd octets): ", bytesRead);
        
        // Afficher les caractères reçus
        for (ssize_t i = 0; i < bytesRead; i++) {
            if (buf[i] >= 32 && buf[i] <= 126) {
                // Caractère imprimable ASCII
                printf("%c", buf[i]);
            } else {
                // Caractère non-imprimable, affichage en hexadécimal
                printf("[%02X]", (unsigned char)buf[i]);
            }
        }
        printf("\n");
    } else if (bytesRead < 0) {
        fprintf(stderr, "Erreur de lecture: %s\n", strerror(errno));
    } else {
        printf("Aucune donnée reçue\n");
    }
    
    /**
     * @brief Fermeture du port série et terminaison du programme
     */
    close(fd);
    return 0;
}