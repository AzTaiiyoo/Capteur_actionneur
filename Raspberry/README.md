# Compilation des programmes de test servo et UART

## Fichiers sources

- `servo_test.c` : Test PWM pour servo-moteur avec balayage automatique
- `test_uart.c` : Test simple de communication UART
- `uart_test.c` : Diagnostic avancé de la communication UART

## Commandes de compilation

### servo_test.c

```bash
gcc -Wall -Wextra -O2 -o servo_test servo_test.c -lpthread
```

### test_uart.c

```bash
gcc -Wall -Wextra -O2 -o test_uart test_uart.c
```

### uart_test.c

```bash
gcc -Wall -Wextra -O2 -o uart_test uart_test.c
```

## Utilisation de minicom

Pour configurer et utiliser minicom avec le port série:

```bash
# Installation si nécessaire
sudo apt-get install minicom

# Configuration du port série
sudo minicom -s
# Sélectionnez "Serial port setup"
# Configurez le port à /dev/serial0
# Vitesse (baud rate): 115200
# Bits de données: 8
# Parité: None
# Bits de stop: 1
# Contrôle de flux: None
# Sauvegardez comme configuration par défaut

# Lancer minicom
sudo minicom
```

## Exécution des programmes compilés

```bash
sudo ./servo_test
sudo ./test_uart
sudo ./uart_test
```
