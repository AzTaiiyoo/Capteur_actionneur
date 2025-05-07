/**
 * @file gpio.c
 * @brief Implémentation des fonctions de gestion des GPIO
 * @details Fonctions pour configurer et contrôler les broches GPIO de la Raspberry Pi
 */

 #include "gpio.h"

 /**
  * @brief Initialise le GPIO pour le servo-moteur
  * @return 0 en cas de succès, -1 en cas d'échec
  */
 int setupGPIO(void) {
     // Exporter la broche GPIO et la configurer en sortie
     exportGPIO(SERVO_PIN);
     setGPIODirection(SERVO_PIN, "out");
     
     printf("GPIO initialisé avec succès\n");
     return 0;
 }
 
 /**
  * @brief Exporte une broche GPIO pour la rendre utilisable
  * @param pin Numéro de la broche GPIO à exporter
  */
 void exportGPIO(int pin) {
     int fd = open("/sys/class/gpio/export", O_WRONLY);
     if (fd < 0) {
         fprintf(stderr, "Erreur lors de l'ouverture du fichier d'export GPIO\n");
         return;
     }
     
     char buffer[3];
     snprintf(buffer, sizeof(buffer), "%d", pin);
     
     // Ignorer l'erreur si la broche est déjà exportée
     write(fd, buffer, strlen(buffer));
     close(fd);
     
     // Attendre que le système de fichiers soit prêt
     usleep(100000); // 100ms
 }
 
 /**
  * @brief Définit la direction d'une broche GPIO (entrée ou sortie)
  * @param pin Numéro de la broche GPIO
  * @param direction Direction ("in" pour entrée, "out" pour sortie)
  */
 void setGPIODirection(int pin, const char* direction) {
     char path[50];
     snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pin);
     
     int fd = open(path, O_WRONLY);
     if (fd < 0) {
         fprintf(stderr, "Erreur lors de l'ouverture du fichier de direction GPIO\n");
         return;
     }
     
     write(fd, direction, strlen(direction));
     close(fd);
 }
 
 /**
  * @brief Écrit une valeur sur une broche GPIO
  * @param pin Numéro de la broche GPIO
  * @param value Valeur à écrire (0 ou 1)
  */
 void writeGPIO(int pin, int value) {
     char path[50];
     snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);
     
     int fd = open(path, O_WRONLY);
     if (fd < 0) {
         fprintf(stderr, "Erreur lors de l'ouverture du fichier de valeur GPIO\n");
         return;
     }
     
     char buffer[2];
     snprintf(buffer, sizeof(buffer), "%d", value);
     
     write(fd, buffer, 1);
     close(fd);
 }