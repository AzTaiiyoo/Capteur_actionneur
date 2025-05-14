/**
 * @file serial.c
 * @brief Implémentation des fonctions de communication série
 */

 #include "serial.h"

 /**
  * @brief Ouvre la connexion série avec la STM32
  * @return Descripteur de fichier pour la connexion série, ou -1 en cas d'erreur
  */
 int openSerialConnection(void) {
    int fd = open(UART_DEVICE, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        fprintf(stderr, "Impossible d'ouvrir le port série %s\n", UART_DEVICE);
        return -1;
    }
    
    // Configuration du port série
    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    
    if (tcgetattr(fd, &tty) != 0) {
        fprintf(stderr, "Erreur lors de la lecture des attributs du port série\n");
        close(fd);
        return -1;
    }
    
    // Configurer la vitesse à 115200 bauds
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);
    
    // 8N1 (8 bits, pas de parité, 1 bit d'arrêt)
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    
    // Pas de contrôle de flux
    tty.c_cflag &= ~CRTSCTS;
    
    // Activer la réception et désactiver le contrôle du modem
    tty.c_cflag |= CREAD | CLOCAL;
    
    // Mode non canonique, pas d'écho
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    
    // Désactiver le traitement de sortie
    tty.c_oflag &= ~OPOST;
    
    // Configuration des timeouts - À SUPPRIMER OU MODIFIER
    // Ne pas configurer VMIN et VTIME pour utiliser les valeurs par défaut
    // comme dans le code qui fonctionne
    
    // Appliquer les modifications
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        fprintf(stderr, "Erreur lors de l'application des attributs du port série\n");
        close(fd);
        return -1;
    }
    
    // Vider les buffers (ajout recommandé)
    tcflush(fd, TCIOFLUSH);
    
    // IMPORTANT: NE PAS configurer en mode non-bloquant après
    // Supprimer cette ligne qui peut causer des problèmes:
    // fcntl(fd, F_SETFL, O_NONBLOCK);
    
    printf("Connexion série établie sur %s\n", UART_DEVICE);
    return fd;
}
 
 /**
  * @brief Envoie une commande formatée à la STM32
  * @param fd Descripteur de fichier pour la connexion série
  * @param command Commande à envoyer
  */
void sendCommand(int fd, const char* command) {
    if (fd < 0) return;
    
    // Formater la commande
    char formattedCommand[64];
    snprintf(formattedCommand, sizeof(formattedCommand), "<%s>", command);
    
    // Assurez-vous que chaque caractère est envoyé correctement
    for (int i = 0; i < strlen(formattedCommand); i++) {
        write(fd, &formattedCommand[i], 1);
        // Petit délai entre chaque caractère
        usleep(1000); // 1ms
    }
    
    printf("Commande envoyée: %s\n", command);
}
 
 /**
  * @brief Thread pour recevoir les données de la STM32
  * @param arg Argument du thread (non utilisé)
  * @return NULL
  */
 void* receiveDataThread(void* arg) {
     char buffer[256] = {0};
     int bufferIndex = 0;
     int startReceived = 0;
     char c;
     
     while (!stopThreads) {
         if (serialFd < 0) {
             usleep(100000); // 100ms
             continue;
         }
         
         // Lire les données disponibles (un caractère à la fois)
         ssize_t bytesRead = read(serialFd, &c, 1);
         if (bytesRead > 0) {
             // Traiter le caractère reçu
             if (c == '<') {
                 // Début d'un message
                 bufferIndex = 0;
                 buffer[0] = '\0';
                 startReceived = 1;
             } else if (c == '>' && startReceived) {
                 // Fin d'un message
                 buffer[bufferIndex] = '\0'; // Terminaison de la chaîne
                 
                 // Traiter le message reçu
                 if (strstr(buffer, "Distance:") != NULL && strstr(buffer, "cm") != NULL) {
                     // Mettre à jour la distance actuelle
                     pthread_mutex_lock(&mutex);
                     
                     // Extraire la valeur de distance
                     char* distanceStart = strstr(buffer, "Distance:") + 9;
                     char* distanceEnd = strstr(distanceStart, "cm");
                     
                     if (distanceEnd != NULL) {
                         *distanceEnd = '\0'; // Terminer la chaîne à la fin de la valeur
                         
                         // Convertir la chaîne en nombre à virgule flottante
                         float distance = atof(distanceStart);
                         currentDistance = distance;
                         
                         // Mettre à jour le message de la dernière distance
                         sprintf(lastDistanceMessage, "%.2f cm", distance);
                         
                         // Afficher la distance reçue (pour débogage)
                         printf("Distance reçue: %.2f cm\n", distance);
                     }
                     
                     pthread_mutex_unlock(&mutex);
                 } else if (strstr(buffer, "LastDistance:") != NULL && strstr(buffer, "cm") != NULL) {
                     // Message spécifique de débogage suite à la commande getdistance
                     printf("\033[1;36m"); // Cyan en gras pour mettre en évidence
                     printf("DÉBOGAGE - %s\n", buffer);
                     printf("\033[0m");    // Revenir à la couleur normale
                     
                     // Mettre à jour le message de la dernière distance
                     pthread_mutex_lock(&mutex);
                     strcpy(lastDistanceMessage, buffer + 13); // Skip "LastDistance:"
                     pthread_mutex_unlock(&mutex);
                 } else if (strstr(buffer, "Mode") != NULL) {
                     // Afficher les messages de mode
                     printf("STM32: %s\n", buffer);
                 } else if (strstr(buffer, "System ready") != NULL) {
                     // Message de démarrage
                     printf("STM32: %s\n", buffer);
                 } else {
                     // Autres messages de la STM32
                     printf("Message reçu: %s\n", buffer);
                 }
                 
                 startReceived = 0;
             } else if (startReceived && bufferIndex < sizeof(buffer) - 1) {
                 // Ajouter le caractère au buffer
                 buffer[bufferIndex++] = c;
             }
         }
         
         usleep(10000); // 10ms
     }
     
     return NULL;
 }