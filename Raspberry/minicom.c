#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#define UART_DEVICE "/dev/serial0"

int main() {
    int fd = open(UART_DEVICE, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror("Erreur d'ouverture du port série");
        return 1;
    }
    
    struct termios config;
    if (tcgetattr(fd, &config) != 0) {
        perror("Erreur de lecture des attributs");
        close(fd);
        return 1;
    }
    
    // Configuration standard 115200 8N1
    cfsetispeed(&config, B115200);
    cfsetospeed(&config, B115200);
    config.c_cflag &= ~PARENB;
    config.c_cflag &= ~CSTOPB;
    config.c_cflag &= ~CSIZE;
    config.c_cflag |= CS8;
    config.c_cflag |= CREAD | CLOCAL;
    config.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    config.c_oflag &= ~OPOST;
    
    if (tcsetattr(fd, TCSANOW, &config) != 0) {
        perror("Erreur d'application des attributs");
        close(fd);
        return 1;
    }
    
    tcflush(fd, TCIOFLUSH);
    
    printf("Envoi de <mode1> puis affichage de toutes les réponses...\n");
    
    // Envoyer mode1
    const char *cmd = "<mode1>\r\n";
    write(fd, cmd, strlen(cmd));
    tcdrain(fd);
    
    // Lecture des réponses
    printf("Réponses:\n");
    
    // Mode non-bloquant
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    
    // Écouter pendant 5 secondes
    time_t end_time = time(NULL) + 5;
    
    char buffer[256] = {0};
    int bufferIndex = 0;
    int inMessage = 0;
    
    while (time(NULL) < end_time) {
        char c;
        if (read(fd, &c, 1) > 0) {
            // Afficher chaque caractère
            printf("%c", (c >= 32 && c <= 126) ? c : '.');
            
            // Construire des messages complets entre < et >
            if (c == '<') {
                inMessage = 1;
                bufferIndex = 0;
                buffer[0] = c;
                bufferIndex++;
            } else if (c == '>' && inMessage) {
                buffer[bufferIndex] = c;
                buffer[bufferIndex + 1] = '\0';
                printf("\nMessage complet: %s\n", buffer);
                inMessage = 0;
            } else if (inMessage && bufferIndex < sizeof(buffer) - 2) {
                buffer[bufferIndex] = c;
                bufferIndex++;
            }
            
            fflush(stdout);
        }
        usleep(5000);
    }
    
    printf("\nDiagnostic terminé\n");
    
    fcntl(fd, F_SETFL, flags);
    close(fd);
    
    return 0;
}