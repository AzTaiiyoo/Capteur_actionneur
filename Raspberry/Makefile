# Makefile pour le projet de contrôle de servo-moteur sur Raspberry Pi
# Mai 2025

# Compilateur et options
CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
LDFLAGS = -lpthread

# Fichiers source pour le programme principal
MAIN_SRCS = main.c gpio.c servo.c serial.c
MAIN_OBJS = $(MAIN_SRCS:.c=.o)
MAIN_TARGET = servo_controller

# Fichiers source pour les programmes de test
TEST_TARGETS = pwm_test test_uart minicom

# Tous les fichiers objets
ALL_OBJS = $(MAIN_OBJS) pwm_test.o test_uart.o minicom.o

# Cible par défaut
all: $(MAIN_TARGET) $(TEST_TARGETS)

# Règle pour le programme principal
$(MAIN_TARGET): $(MAIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Règle pour les programmes de test
pwm_test: pwm_test.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_uart: test_uart.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

minicom: minicom.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Règle générique pour compiler les fichiers source en objets
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Génération des dépendances
depend: $(MAIN_SRCS) $(TEST_TARGETS:=.c)
	$(CC) -MM $^ > .depend

# Nettoyage
clean:
	rm -f $(MAIN_TARGET) $(TEST_TARGETS) $(ALL_OBJS) *~ .depend

# Installation des exécutables dans /usr/local/bin (nécessite des privilèges root)
install: $(MAIN_TARGET) $(TEST_TARGETS)
	install -m 755 $(MAIN_TARGET) /usr/local/bin/
	install -m 755 $(TEST_TARGETS) /usr/local/bin/

# Cible pour empêcher make de confondre un fichier nommé "clean" avec la cible clean
.PHONY: all clean install depend

# Inclusion des fichiers de dépendance s'ils existent
-include .depend