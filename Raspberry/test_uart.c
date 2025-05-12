// test_uart.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

int main() {
    // Ouvrir le port série
    int fd = open("/dev/serial0", O_RDWR | O_NOCTTY);
    if (fd < 0) {
        fprintf(stderr, "Erreur ouverture port série: %s\n", strerror(errno));
        return 1;
    }
    
    // Configuration
    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    
    // Lire les paramètres actuels
    if (tcgetattr(fd, &tty) != 0) {
        fprintf(stderr, "Erreur tcgetattr: %s\n", strerror(errno));
        close(fd);
        return 1;
    }
    
    // Configurer à 115200 8N1
    tty.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    tty.c_iflag = 0;
    tty.c_oflag = 0;
    tty.c_lflag = 0;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 10; // 1 seconde timeout
    
    // Appliquer la configuration
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        fprintf(stderr, "Erreur tcsetattr: %s\n", strerror(errno));
        close(fd);
        return 1;
    }
    
    // Vider les buffers
    tcflush(fd, TCIOFLUSH);
    
    // Envoyer un message de test
    const char *msg = "<TEST>";
    printf("Envoi de: %s\n", msg);
    write(fd, msg, strlen(msg));
    tcdrain(fd);
    
    // Attendre une réponse
    printf("Attente de réponse...\n");
    usleep(500000); // 500ms
    
    // Lire la réponse
    char buf[256];
    memset(buf, 0, sizeof(buf));
    
    ssize_t bytesRead = read(fd, buf, sizeof(buf) - 1);
    if (bytesRead > 0) {
        printf("Reçu (%zd octets): ", bytesRead);
        
        // Afficher les caractères reçus
        for (ssize_t i = 0; i < bytesRead; i++) {
            if (buf[i] >= 32 && buf[i] <= 126) {
                printf("%c", buf[i]);
            } else {
                printf("[%02X]", (unsigned char)buf[i]);
            }
        }
        printf("\n");
    } else if (bytesRead < 0) {
        fprintf(stderr, "Erreur de lecture: %s\n", strerror(errno));
    } else {
        printf("Aucune donnée reçue\n");
    }
    
    // Fermer le port
    close(fd);
    return 0;
}