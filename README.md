# Rapport de Projet : Du Capteur à l’Actionneur

## Sommaire

- [Rapport de Projet : Du Capteur à l’Actionneur](#rapport-de-projet--du-capteur-à-lactionneur)
  - [Sommaire](#sommaire)
  - [1. Introduction](#1-introduction)
    - [Objectif du projet](#objectif-du-projet)
    - [Modes de fonctionnement](#modes-de-fonctionnement)
    - [Comportements supplémentaires](#comportements-supplémentaires)
    - [Schéma fonctionnel](#schéma-fonctionnel)
  - [2. Brochage des différents éléments](#2-brochage-des-différents-éléments)
  - [3. Modules développés](#3-modules-développés)
    - [3.1. Module HC-SR04](#31-module-hc-sr04)
    - [3.2. Module Servo-moteur](#32-module-servo-moteur)
    - [3.3. Module UART](#33-module-uart)
    - [3.4. Application principale](#34-application-principale)
  - [5. Résultats obtenus](#5-résultats-obtenus)
  - [6. Difficultés rencontrées et solutions](#6-difficultés-rencontrées-et-solutions)
  - [7. Conclusion](#7-conclusion)

## 1. Introduction

### Objectif du projet

Ce projet consiste à développer une application sur microcontrôleur STM32F407 avec les objectifs suivants :

- Mesurer une distance à l'aide d'un capteur HC-SR04.
- Contrôler un servo-moteur en fonction de la distance mesurée.
- Proposer deux modes de fonctionnement sélectionnables par un bouton-poussoir ou une commande UART.

### Modes de fonctionnement

L'application comporte deux modes :

1. **Mode 1 :**
   - LED bleue allumée.
   - La position du servo-moteur est proportionnelle à la distance mesurée (entre 5 et 25 cm).
2. **Mode 2 :**
   - LED verte allumée.
   - La position du servo-moteur dépend d'une consigne reçue via la liaison série.

### Comportements supplémentaires

- La distance mesurée est envoyée toutes les secondes sur la liaison série.
- Le passage entre les modes se fait :
  - Par appui sur le bouton-poussoir.
  - Par réception d'une commande dédiée via la liaison série.

### Schéma fonctionnel

Voici le schéma fonctionnel du système :

- **Entrées :**
  - Capteur HC-SR04 (TRIG, ECHO).
  - UART (TX, RX).
  - Bouton-poussoir.
- **Sorties :**
  - Servo-moteur (PWM).
  - LED (Bleue, Verte, Orange, Rouge).

## 2. Brochage des différents éléments

| Composant       | Broche STM32   | Description                      |
| --------------- | -------------- | -------------------------------- |
| HC-SR04 Trigger | PA8            | Signal de déclenchement (sortie) |
| HC-SR04 Echo    | PA9            | Signal de retour (entrée)        |
| Servo-moteur    | TIM1 Channel 1 | Contrôle PWM                     |
| LED Orange      | PD13           | Indique une distance valide      |
| LED Rouge       | PD14           | Indique une distance hors plage  |
| LED Bleue       | PD15           | Indique le mode 1                |
| LED Verte       | PD12           | Indique le mode 2                |
| Bouton-poussoir | PA0            | Permet de changer de mode        |
| UART TX         | PA2            | Transmission UART                |
| UART RX         | PA3            | Réception UART                   |

## 3. Modules développés

### 3.1. Module HC-SR04

**Objectif** :
Mesurer la distance entre le capteur et un obstacle en utilisant le signal Trigger et Echo.

**Configuration des périphériques** :

- **GPIO** :
  - PA8 configuré en sortie pour le Trigger.
  - PA9 configuré en entrée avec pull-down pour le Echo.
- **Timer** :
  - TIM1 utilisé pour mesurer le temps de vol du signal Echo.

**Fonctionnalités** :

- Envoi d'une impulsion Trigger de 10 µs.
- Mesure du temps de vol du signal Echo à l'aide de TIM1.
- Calcul de la distance en centimètres :  
  \[
  \text{Distance (cm)} = \frac{\text{Durée (µs)} \times 0.0343}{2}
  \]

**Validation** :

- Utilisation d'un oscilloscope pour vérifier l'impulsion Trigger et le signal Echo.
- Test avec des obstacles à différentes distances (5 cm, 15 cm, 25 cm).

### 3.2. Module Servo-moteur

**Objectif** :
Contrôler la position du servo-moteur en fonction de la distance mesurée ou d'une consigne reçue via UART.

**Configuration des périphériques** :

- **Timer** :
  - TIM1 configuré en mode PWM sur Channel 1.
  - Période du timer fixée à 20 ms (50 Hz).
  - Rapport cyclique ajusté entre 5 % et 10 % pour contrôler l'angle du servo.

**Fonctionnalités** :

- Fonction `Servo_set_position(float distance)` :
  - Calcule le rapport cyclique en fonction de la distance (5 cm → 5 %, 25 cm → 10 %).
  - Utilise `__HAL_TIM_SET_COMPARE` pour ajuster le rapport cyclique.

**Validation** :

- Utilisation d'un oscilloscope pour vérifier le signal PWM.
- Test avec le servo-moteur pour s'assurer que la position correspond à la distance.

### 3.3. Module UART

**Objectif** :
Envoyer et recevoir des données via la liaison série.

**Configuration des périphériques** :

- **UART** :
  - Baudrate : 115200.
  - TX : PA2, RX : PA3.

**Fonctionnalités** :

- Envoi de la distance mesurée toutes les secondes.
- Réception d'une consigne pour le mode 2.

**Validation** :

- Utilisation d'un terminal série (ex. PuTTY) pour envoyer et recevoir des données.
- Vérification des données envoyées avec un analyseur logique.

### 3.4. Application principale

**Objectif** :
Coordonner les différents modules et implémenter les deux modes de fonctionnement.

**Fonctionnalités** :

- **Mode 1** :
  - LED bleue allumée.
  - La position du servo-moteur est proportionnelle à la distance mesurée.
- **Mode 2** :
  - LED verte allumée.
  - La position du servo-moteur dépend d'une consigne reçue via UART.
- **Changement de mode** :
  - Appui sur le bouton-poussoir (PA0) ou réception d'une commande UART.

**Validation** :

- Test des deux modes avec des obstacles et des consignes UART.
- Vérification des changements de mode via le bouton-poussoir.

## 5. Résultats obtenus

| Test                              | Résultat attendu                       | Résultat obtenu |
| --------------------------------- | -------------------------------------- | --------------- |
| Mesure de distance (5 cm à 25 cm) | Distance correcte affichée en UART     | OK              |
| Contrôle du servo-moteur (Mode 1) | Position proportionnelle à la distance | OK              |
| Contrôle du servo-moteur (Mode 2) | Position dépendant de la consigne UART | OK              |
| Changement de mode                | LED bleue/verte s'allume correctement  | OK              |

## 6. Difficultés rencontrées et solutions

| Problème                                   | Solution                                                                     |
| ------------------------------------------ | ---------------------------------------------------------------------------- |
| Fonction `HAL_DelayMicroseconds` manquante | Implémentation d'une fonction utilisant TIM6 pour générer des délais précis. |
| Signal Echo instable                       | Ajout d'un pull-down sur la broche PA9.                                      |
| Servo-moteur ne répond pas correctement    | Ajustement des limites du rapport cyclique (5 % à 10 %).                     |
| UART non fonctionnel                       | Vérification des connexions et des paramètres de configuration.              |

## 7. Conclusion

Ce projet a permis de développer une application complète sur STM32F407, intégrant un capteur HC-SR04, un servo-moteur, et une liaison UART. Les objectifs du cahier des charges ont été atteints, et les modules développés sont fonctionnels et modulaires. Ce projet a également permis de renforcer les compétences en configuration des périphériques STM32 et en débogage matériel.
