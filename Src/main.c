/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "module/hcsr04.h"
#include "module/servo.h"
#include "module/usart_comm.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/**
 * @typedef SystemState
 * @brief Énumération des états possibles du système
 */
typedef enum {
  STATE_IDLE,    /**< État initial/neutre */
  STATE_MODE1,   /**< Mode 1: LED bleue + servo positionné selon distance HCSR04 */
  STATE_MODE2    /**< Mode 2: LED verte + servo positionné selon consigne série */
} SystemState;

/**
 * @typedef Command
 * @brief Énumération des commandes reconnues par le système
 */
typedef enum {
  CMD_NONE,     /**< Aucune commande ou commande non reconnue */
  CMD_MODE1,    /**< Commande pour activer le mode 1 */
  CMD_MODE2,    /**< Commande pour activer le mode 2 */
  CMD_QUIT,     /**< Commande pour revenir à l'état IDLE */
  CMD_VALUE     /**< Valeur numérique pour positionner le servo en mode 2 */
} Command;

/**
 * @var currentState
 * @brief État actuel du système
 */
SystemState currentState = STATE_IDLE;

/**
 * @var servoPosition
 * @brief Position actuelle demandée pour le servo (de 1 à 12)
 */
uint8_t servoPosition = 6;  // Position par défaut (milieu)

/**
 * @var mode
 * @brief Mode de fonctionnement actuel (0 = IDLE, 1 = MODE1, 2 = MODE2)
 */
static int mode; 
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/**
 * @brief Traite une commande reçue via la communication série
 * @param command Chaîne de caractères contenant la commande à traiter
 */
void processCommand(char* command);

/**
 * @brief Analyse une chaîne de caractères pour identifier la commande
 * @param command Chaîne de caractères à analyser
 * @return Type de commande identifiée
 */
Command parseCommand(char* command);

/**
 * @brief Met à jour l'état du système en fonction du mode actuel
 */
void updateSystem(void);

/**
 * @brief Gère la réception des commandes via la liaison série
 */
void handleSerialCommunication(void);
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  // Initialisation du capteur HC-SR04
  HC_SR04* sensor = HC_SR04_get_instance();  // Obtenir l'instance du capteur HC-SR04
  if (!sensor) {
    Error_Handler();  // Si l'initialisation échoue, appeler Error_Handler
  }
  
  // Lancement de la PWM sur le canal 1 de TIM1
  // HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1); // Start PWM on TIM1 Channel 1
  // __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 3); // Set initial duty cycle to 0%

  // Démarrer la PWM pour le servo-moteur
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 1500); // Position initiale au centre
  
  HC_SR04_init(); // Initialisation du capteur HC-SR04

  // Initialiser le servo
  Servo_Init();
  Servo_SetToMiddle(); // Mettre en position neutre au démarrage

  sendMessage("System ready. Available commands: mode1, mode2, quit");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // Obtenir l'instance du capteur
    HC_SR04* sensor = HC_SR04_get_instance();
    
    // Mettre à jour la distance et contrôler les LEDs
    HC_SR04_update(sensor, mode);
    
    // Gérer la communication série pour les commandes
    handleSerialCommunication();
    
    // Mettre à jour l'état du système en fonction du mode
    updateSystem();
    
    // Petite pause pour éviter de surcharger le CPU
    // HAL_Delay(10);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  // Ne devrait jamais arriver
  // Error_Handler();
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/**
 * @brief Gère la réception des commandes via la liaison série
 * @details Cette fonction attend de façon non-bloquante une commande sur la 
 * liaison série. Lorsqu'une commande complète est reçue, elle est passée
 * à la fonction processCommand pour traitement.
 */
void handleSerialCommunication(void) {
  char cmdBuffer[32];
  
  // Essayer de recevoir une commande (non-bloquant maintenant)
  int result = receiveMessage(cmdBuffer, sizeof(cmdBuffer));
  
  if (result >= 0) {
      // Commande complète reçue, la traiter
      processCommand(cmdBuffer);
  }
  // Si result < 0, aucune commande complète n'a été reçue, on continue l'exécution
}

/**
 * @brief Analyse la commande reçue
 * @param command Chaîne de caractères contenant la commande
 * @return Type de commande identifiée
 * @details Reconnaît les commandes "mode1", "mode2", "quit" et les valeurs numériques
 * entre 1 et 12 pour le positionnement du servo en mode 2
 */
Command parseCommand(char* command)
{
  if (strcmp(command, "mode1") == 0) {
    return CMD_MODE1;
  }
  else if (strcmp(command, "mode2") == 0) {
    return CMD_MODE2;
  }
  else if (strcmp(command, "quit") == 0) {
    return CMD_QUIT;
  }
  else {
    // Vérifier si c'est une valeur numérique (pour le mode 2)
    int value;
    if (sscanf(command, "%d", &value) == 1) {
      if (value >= 1 && value <= 12) {
        servoPosition = value;
        return CMD_VALUE;
      }
    }
  }
  
  return CMD_NONE;
}

/**
 * @brief Traite la commande reçue et met à jour l'état du système
 * @param command Chaîne de caractères contenant la commande
 * @details Selon la commande identifiée, change l'état du système et
 * envoie un message de confirmation ou d'erreur
 */
void processCommand(char* command)
{
  Command cmd = parseCommand(command);
  
  switch (cmd) {
    case CMD_MODE1:
      if (currentState != STATE_MODE1) {
        currentState = STATE_MODE1;
        mode = 1;
        sendMessage("Mode 1 activated: LED blue + servo follows ultrasonic sensor");
      } else {
        sendMessage("Already in Mode 1");
      }
      break;
      
    case CMD_MODE2:
      if (currentState != STATE_MODE2) {
        currentState = STATE_MODE2;
        mode = 2;
        sendMessage("Mode 2 activated: LED green + servo follows serial input (1-12)");
      } else {
        sendMessage("Already in Mode 2");
      }
      break;
      
    case CMD_QUIT:
      if (currentState != STATE_IDLE) {
        currentState = STATE_IDLE;
        sendMessage("Returned to idle state");
      } else {
        sendMessage("Already in idle state");
      }
      break;
      
    case CMD_VALUE:
      if (currentState == STATE_MODE2) {
        char response[50];
        sprintf(response, "Servo position set to %d", servoPosition);
        sendMessage(response);
      } else {
        sendMessage("Error: Can only set servo position in Mode 2");
      }
      break;
      
    case CMD_NONE:
      sendMessage("Unknown command. Available: mode1, mode2, quit, or value (1-12) in Mode 2");
      break;
  }
}

/**
 * @brief Met à jour l'état du système en fonction du mode actuel
 * @details Exécutée périodiquement dans la boucle principale, cette fonction
 * met à jour les LEDs et la position du servo selon l'état actuel du système
 */
void updateSystem(void)
{
  static uint32_t lastUpdate = 0;
  uint32_t currentTime = HAL_GetTick();
  
  if (currentTime - lastUpdate >= 100) {
    lastUpdate = currentTime;
    
    HC_SR04* sensor = HC_SR04_get_instance();
    
    switch (currentState) {
      case STATE_IDLE:
        HAL_GPIO_WritePin(GPIOD, LED_BLEU_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, LED_VERT_Pin, GPIO_PIN_RESET);
        Servo_SetToMiddle();
        break;
        
      case STATE_MODE1:
        HAL_GPIO_WritePin(GPIOD, LED_BLEU_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, LED_VERT_Pin, GPIO_PIN_RESET);
        
        if (sensor->distance >= SERVO_MIN_DISTANCE && sensor->distance <= SERVO_MAX_DISTANCE) {
          uint32_t position = Servo_DistanceToPosition(sensor->distance);
          Servo_SetPosition(position);
        } else {
          Servo_SetToMiddle();
        }
        break;
        
      case STATE_MODE2:
        HAL_GPIO_WritePin(GPIOD, LED_VERT_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, LED_BLEU_Pin, GPIO_PIN_RESET);
        
        uint32_t position = Servo_ValueToPosition(servoPosition);
        Servo_SetPosition(position);
        break;
    }
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
